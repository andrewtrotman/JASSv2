/*
	JASS_ANYTIME.CPP
	----------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdlib.h>

#include <string>

#include "timer.h"
#include "version.h"
#include "commandline.h"
#include "channel_file.h"
#include "channel_trec.h"
#include "JASS_anytime_api.h"
#include "JASS_anytime_query.h"

/*
	PARAMETERS
	----------
*/
static double rho = 100.0;											///< In the anytime paper rho is the percentage of the collection that should be used as a cap to the number of postings processed.
static double rho_min = 0;											///< In the anytime paper rho is the percentage of the collection that should be used as a cap to the number of postings processed, this is the minimum to process.
static double parameter_relative_rho = 100.0;						///< Percentage of the all the postings for this query that should be processed
static size_t maximum_number_of_postings_to_process = 0;			///< Computed from rho
static size_t minimum_number_of_postings_to_process = 0;			///< The minimum number of postings to process (to prevent "way too early" early termination
static std::string parameter_queryfilename;							///< Name of file containing the queries
static size_t parameter_threads = 1;								///< Number of concurrent queries
static size_t parameter_top_k = 10;									///< Number of results to return
static size_t accumulator_width = 0;								///< The width (2^accumulator_width) of the accumulator 2-D array (if they are being used).
static bool parameter_ascii_query_parser = false;					///< When true use the ASCII pre-casefolded query parser
static bool parameter_help = false;									///< Print the usage information
static bool parameter_index_v2 = false;								///< The index is a JASS version 2 index
static std::string parameter_rsv_scores_filename;					///< The name of the file containing ordered pairs <query_id> <rsv> for the minimum rsv to be found

static std::string parameters_errors;								///< Any errors as a result of command line parsing
static auto parameters = std::make_tuple							///< The  command line parameter block
	(
	JASS::commandline::parameter("-?",   "--help",         "                  Print this help.", parameter_help),
	JASS::commandline::parameter("-2",   "--v2_index",     "                  The index is a JASS v2 index", parameter_index_v2),
	JASS::commandline::parameter("-a",   "--asciiparser ", "                  Use simple query parser (ASCII seperated pre-casefolded tokens)", parameter_ascii_query_parser),
	JASS::commandline::parameter("-k",   "--top-k",        "<top-k>           Number of results to return to the user (top-k value) [default = -k10]", parameter_top_k),
	JASS::commandline::parameter("-q",   "--queryfile",    "<filename>        Name of file containing a list of queries (1 per line, each line prefixed with query-id)", parameter_queryfilename),
	JASS::commandline::parameter("-Q",   "--queryrsvfile", "<filename>        Name of file containing a list of the minimum rsv value for a document to be found (1 per line: <query_id> <rsv>)", parameter_rsv_scores_filename),
	JASS::commandline::parameter("-r",   "--rho",          "<integer_percent> Percent of the collection size to use as max number of postings to process [default = -r100] (overrides -R)", rho),
	JASS::commandline::parameter("-⌊r⌋", "--rho_min",      "<integer_percent> Percent of the collection size to use as minimum number of postings to process [default is 0] (overrides -R)", rho_min),
	JASS::commandline::parameter("-R",   "--RHO",          "<integer_max>     Max number of postings to process [default is all]", maximum_number_of_postings_to_process),
	JASS::commandline::parameter("-⌊R⌋", "--RHO_min",      "<integer_min>     Minimum number of postings to process [default is 0]", minimum_number_of_postings_to_process),
	JASS::commandline::parameter("-ℝ",   "--Relative_RHO", "<integer_percent> Percent of this queries postings to use as max number of postings to process [default = -ℝ100] (overrides -R and -r)", parameter_relative_rho),
	JASS::commandline::parameter("-t",   "--threads",      "<threadcount>     Number of threads to use (one query per thread) [default = -t1]", parameter_threads),
	JASS::commandline::parameter("-w",   "--width",        "<2^w>             The width of the 2D accumulator array (2^w is used)", accumulator_width)
	);

/*
	MAKE_INPUT_CHANNEL()
	--------------------
*/
JASS::channel *make_input_channel(std::string filename)
	{
	std::string file;
	JASS::file::read_entire_file(filename, file);
	
	/*
		If the start of the file is a digit then we think we have a JASS topic file.
		If the start is not a digit then we're expecting to see a TREC topic file.
	*/
	if (::isdigit(file[0]))
		return new JASS::channel_file(filename);				// JASS topic file
	else
		{
		std::unique_ptr<JASS::channel> source(new JASS::channel_file(filename));
		return new JASS::channel_trec(source, "tq");		// TREC topic file
		}
	}

/*
	LOAD_QUERIES()
	--------------
*/
void load_queries(std::vector<JASS_anytime_query> &query_list, const std::string filename)
	{
	/*
		Read from the query file into a list of queries array.
	*/
	JASS::channel *input = make_input_channel(parameter_queryfilename);		// read from here
	std::string query;												// the channel read goes into memory managed by this object

	/*
		Read the query set and bung it into a vector
	*/
	input->gets(query);

	std::size_t found = query.find_last_not_of(" \t\f\v\n\r");
	if (found != std::string::npos)
	  query.erase(found + 1);
	else
	  query.clear();            // str is all whitespace

	while (query.size() != 0)
		{
		query_list.push_back(query);

		input->gets(query);

		std::size_t found = query.find_last_not_of(" \t\f\v\n\r");
		if (found != std::string::npos)
		  query.erase(found + 1);
		else
		  query.clear();            // str is all whitespace
		}
		
	delete input;
	}

/*
	USAGE()
	-------
*/
static uint8_t usage(const std::string &exename)
	{
	std::cout << JASS::commandline::usage(exename, parameters) << "\n";
	return 1;
	}

/*
	MAIN_EVENT()
	------------
*/
static int main_event(int argc, const char *argv[])
	{
	JASS_anytime_api engine;

	/*
		Start the instrumentation tools
	*/
	auto total_run_time = JASS::timer::start();
	JASS_anytime_stats stats;
	stats.threads = parameter_threads;

	/*
		Parse the command line parameters
	*/
	if (!JASS::commandline::parse(argc, argv, parameters, parameters_errors))
		{
		std::cout << parameters_errors;
		exit(1);
		}
	if (parameter_help)
		exit(usage(argv[0]));

	/*
		Set the top-k value
	*/
	if (engine.set_top_k(parameter_top_k) != JASS_ERROR_OK)
		{
		std::cout << "The top-k specified (" << parameter_top_k << ") is larger than maximum TOP-K (" << engine.get_max_top_k() << ")\n";
		return 0;
		}

	/*
		Read the pre-computed rsv score table, if there is one.
	*/
	if (!parameter_rsv_scores_filename.empty())
		if (engine.load_oracle_scores(parameter_rsv_scores_filename) != JASS_ERROR_OK)
			{
			std::cout << "Cannot load the Oracle top-k scores from file '" << parameter_rsv_scores_filename << "'\n";
			return 0;
			}

	/*
		Read the index into memory
	*/
	if (engine.load_index(parameter_index_v2 ? 2 : 1, "", true) != JASS_ERROR_OK)
		{
		std::cout << "Cannot load the index\n";
		return 0;
		}
	stats.number_of_documents = engine.get_document_count();

	/*
		Set the accumulator width
	*/
	if (accumulator_width != 0)
		if (engine.set_accumulator_width(accumulator_width) != JASS_ERROR_OK)
			{
			std::cout << "Cannot set the accumulator width to " << accumulator_width << '\n';
			return 0;
			}

	/*
		Set the parser (this will normally be the "regular" query parser, but sometimes the queries contain "weird stuff" and need to be tokenised with spaces as seperators.
	*/
	if (parameter_ascii_query_parser)
		engine.use_ascii_parser();
	else
		engine.use_query_parser();

	/*
		Set the Anytime stopping criteria
	*/
	if (maximum_number_of_postings_to_process != 0)
		if (engine.set_postings_to_process(maximum_number_of_postings_to_process) != JASS_ERROR_OK)
			{
			std::cout << "Failure to set the number of postings to process to " << maximum_number_of_postings_to_process << '\n';
			return 0;
			}
	if (minimum_number_of_postings_to_process != 0)
		if (engine.set_postings_to_process_minimum(minimum_number_of_postings_to_process) != JASS_ERROR_OK)
			{
			std::cout << "Failure to set the minimum number of postings to process to " << minimum_number_of_postings_to_process << '\n';
			return 0;
			}
	if (rho != 100.0)
		if (engine.set_postings_to_process_proportion(rho) != JASS_ERROR_OK)
			{
			std::cout << "Failure to set the proportion of postings to process\n";
			return 0;
			}
	if (rho_min != 0)
		if (engine.set_postings_to_process_proportion_minimum(rho_min) != JASS_ERROR_OK)
			{
			std::cout << "Failure to set the minimum proportion of postings to process\n";
			return 0;
			}
	if (parameter_relative_rho != 100)
		if (engine.set_postings_to_process_relative(parameter_relative_rho) != JASS_ERROR_OK)
			{
			std::cout << "Failure to set the relative postings stopping condition\n";
			return 0;
			}

	/*
		Report the number of postings we're going to process
	*/
	if (engine.get_postings_to_process() < engine.get_document_count())
		std::cout << "Maximum number of postings to process: " << engine.get_postings_to_process() << "\n";
	else
		std::cout << "Maximum number of postings to process: Search to completion\n";

	/*
		Report the compression scheme used in this index
	*/
	std::string codex_name;
	int32_t d_ness;
	engine.get_encoding_scheme(codex_name, d_ness);
	std::cout << "Index compressed with " << codex_name << "-D" << d_ness << "\n";

	/*
		Load the queries
	*/
	std::vector<JASS_anytime_query> query_list;
	load_queries(query_list, parameter_queryfilename);
	stats.number_of_queries = query_list.size();
	
	/*
		Search
	*/
	std::vector<JASS_anytime_thread_result> output;
	output.resize(1);

	auto total_search_time = JASS::timer::start();
#ifdef ONE_BY_ONE
	for (size_t which = 0; which < query_list.size(); which++)
		engine.search(&output[0], query_list[which].query);
#else
	engine.search(output, query_list, parameter_threads);
#endif
	stats.wall_time_in_ns = JASS::timer::stop(total_search_time).nanoseconds();

	/*
		Compute and dump the per-query stats.  Output the run in TREC format.
	*/
	std::ostringstream TREC_file;
	std::ostringstream stats_file;
	stats_file << "<JASSv2stats>\n";
	for (size_t which = 0; which < parameter_threads ; which++)
		for (const auto &[query_id, result] : output[which])
			{
			stats_file << "<id>" << result.query_id << "</id><query>" << result.query << "</query><postings>" << result.postings_processed << "</postings><time_ns>" << result.search_time_in_ns << "</time_ns>\n";
			stats.sum_of_CPU_time_in_ns += result.search_time_in_ns;
			TREC_file << result.results_list;
			}
	stats_file << "</JASSv2stats>\n";

	JASS::file::write_entire_file("ranking.txt", TREC_file.str());
	JASS::file::write_entire_file("JASSv2Stats.txt", stats_file.str());

	/*
		Finally, output how we did.
	*/
	stats.total_run_time_in_ns = JASS::timer::stop(total_run_time).nanoseconds();
	std::cout << stats;

	return 0;
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	std::cout << JASS::version::build() << '\n';
	if (argc == 1)
		exit(usage(argv[0]));
	try
		{
		return main_event(argc, argv);
		}
	catch (std::exception &ers)
		{
		std::cout << "Unexpected Exception:" << ers.what() << "\n";
		return 1;
		}
	}
