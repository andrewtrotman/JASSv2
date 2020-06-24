/*
	JASS_ANYTIME.CPP
	----------------
	Written bu Andrew Trotman.
	Based on JASS v1, which was written by Andrew Trotman and Jimmy Lin
*/
#include <stdio.h>
#include <stdlib.h>

#include <limits>
#include <memory>
#include <fstream>
#include <algorithm>

#include "file.h"
#include "timer.h"
#include "threads.h"
#include "query_heap.h"
#include "run_export.h"
#include "commandline.h"
#include "query_bucket.h"
#include "channel_file.h"
#include "channel_trec.h"
#include "query_maxblock.h"
#include "compress_integer.h"
#include "JASS_anytime_stats.h"
#include "JASS_anytime_query.h"
#include "query_maxblock_heap.h"
#include "deserialised_jass_v1.h"
#include "compress_integer_all.h"
#include "compress_integer_qmx_jass_v1.h"
#include "JASS_anytime_thread_result.h"
#include "compress_integer_elias_gamma_simd.h"

constexpr size_t MAX_QUANTUM = 0x0FFF;
constexpr size_t MAX_TERMS_PER_QUERY = 1024;

constexpr size_t MAX_DOCUMENTS = JASS::query::MAX_DOCUMENTS;
constexpr size_t MAX_TOP_K = JASS::query::MAX_TOP_K;

/*
	PARAMETERS
	----------
*/
double rho = 100.0;												///< In the anytime paper rho is the prcentage of the collection that should be used as a cap to the number of postings processed.
size_t maximum_number_of_postings_to_process = 0;	///< Computed from
std::string parameter_queryfilename;					///< Name of file containing the queries
size_t parameter_threads = 1;								///< Number of concurrent queries
size_t parameter_top_k = 10;								///< Number of results to return
size_t accumulator_width = 7;								///< The width (2^accumulator_width) of the accumulator 2-D array (if they are being used).
bool parameter_help = false;

std::string parameters_errors;							///< Any errors as a result of command line parsing
auto parameters = std::make_tuple						///< The  command line parameter block
	(
	JASS::commandline::parameter("-?", "--help",      "Print this help.", parameter_help),
	JASS::commandline::parameter("-q", "--queryfile", "<filename>        Name of file containing a list of queries (1 per line, each line prefixed with query-id)", parameter_queryfilename),
	JASS::commandline::parameter("-t", "--threads",   "<threadcount>     Number of threads to use (one query per thread) [default = -t1]", parameter_threads),
	JASS::commandline::parameter("-k", "--top-k",     "<top-k>           Number of results to return to the user (top-k value) [default = -k10]", parameter_top_k),
	JASS::commandline::parameter("-r", "--rho",       "<integer_percent> Percent of the collection size to use as max number of postings to process [default = -r100] (overrides -RHO)", rho),
	JASS::commandline::parameter("-R", "--RHO",       "<integer_max>     Max number of postings to process [default is all] (overridden by -rho)", maximum_number_of_postings_to_process),
	JASS::commandline::parameter("-w", "--width",     "<2^w>             The width of the 2d accumulator array (2^w is used)", accumulator_width)
	);

/*
	ANYTIME()
	---------
*/
void anytime(JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, size_t postings_to_process, size_t top_k)
	{
	/*
		Allocate the Score-at-a-Time table
	*/
	uint64_t *segment_order = new uint64_t [MAX_TERMS_PER_QUERY * MAX_QUANTUM];

	/*
		Allocate a JASS query object
	*/
	std::string codex_name;
	int32_t d_ness;
	std::unique_ptr<JASS::compress_integer> jass_query = index.codex(codex_name, d_ness);
#ifdef NEVER
//	typedef JASS::query_maxblock<uint16_t, MAX_DOCUMENTS, MAX_TOP_K> QUERY_TYPE;
	typedef JASS::query_heap QUERY_TYPE;
//	typedef JASS::query_bucket<uint16_t, MAX_DOCUMENTS, MAX_TOP_K> QUERY_TYPE;
//	typedef JASS::query_maxblock_heap<uint16_t, MAX_DOCUMENTS, MAX_TOP_K> QUERY_TYPE;
#endif

	try
		{
		jass_query->init(index.primary_keys(), index.document_count(), top_k, accumulator_width);
		}
	catch (std::bad_array_new_length &ers)
		{
		exit(printf("Can't load index as the number of documents is too large - change MAX_DOCUMENTS in query.h\n"));
		}

	/*
		Start the timer
	*/
	auto total_search_time = JASS::timer::start();

	/*
		Now start searching
	*/
	size_t next_query = 0;
	std::string query = JASS_anytime_query::get_next_query(query_list, next_query);
	std::string query_id;

	while (query.size() != 0)
		{
		static const std::string seperators_between_id_and_query = " \t:";
		output.queries_executed++;

		/*
			Extract the query ID from the query
		*/
		auto end_of_id = query.find_first_of(seperators_between_id_and_query);
		if (end_of_id == std::string::npos)
			query_id = "";
		else
			{
			query_id = query.substr(0, end_of_id);
			auto start_of_query = query.substr(end_of_id, std::string::npos).find_first_not_of(seperators_between_id_and_query);
			if (start_of_query == std::string::npos)
				query = query.substr(end_of_id, std::string::npos);
			else
				query = query.substr(end_of_id + start_of_query, std::string::npos);
			}

		/*
			Process the query
		*/
		jass_query->parse(query);
		auto &terms = jass_query->terms();

		/*
			Parse the query and extract the list of impact segments
		*/
		uint64_t *current_segment = segment_order;
		uint16_t largest_possible_rsv = std::numeric_limits<decltype(largest_possible_rsv)>::min();
		uint16_t smallest_possible_rsv = std::numeric_limits<decltype(smallest_possible_rsv)>::max();
		for (const auto &term : terms)
			{
//std::cout << "TERM:" << term << "\n";

			/*
				Get the metadata for this term (and if this term isn't in the vocab them move on to the next term)
			*/
			JASS::deserialised_jass_v1::metadata metadata;
			if (!index.postings_details(metadata, term))
				continue;

			/*
				Add to the list of impact segments that need to be processed
			*/
			std::copy((uint64_t *)(metadata.offset), (uint64_t *)(metadata.offset) + metadata.impacts, current_segment);

			/*
				Normally the highest impact is the first impact, but binary_to_JASS gets it wrong and puts the highest impact last!
			*/
			size_t highest_term_impact = JASS::maths::maximum(((JASS::deserialised_jass_v1::segment_header *)(index.postings() + current_segment[0]))->impact, ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + current_segment[metadata.impacts - 1]))->impact);
			largest_possible_rsv += highest_term_impact;
			smallest_possible_rsv = JASS::maths::minimum(smallest_possible_rsv, ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + current_segment[0]))->impact, ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + current_segment[metadata.impacts - 1]))->impact);

			current_segment += metadata.impacts;
			}

		/*
			Sort the segments from highest impact to lowest impact
		*/
		std::sort
			(
			segment_order,
			current_segment,
			[postings = index.postings()](uint64_t first, uint64_t second)
				{
				JASS::deserialised_jass_v1::segment_header *lhs = (JASS::deserialised_jass_v1::segment_header *)(postings + first);
				JASS::deserialised_jass_v1::segment_header *rhs = (JASS::deserialised_jass_v1::segment_header *)(postings + second);

				/*
					sort from highest to lowest impact, but break ties by placing the lowest quantum-frequency first and the highest quantum-drequency last
				*/
				if (lhs->impact < rhs->impact)
					return false;
				else if (lhs->impact > rhs->impact)
					return true;
				else			// impact scores are the same, so tie break on the length of the segment
					return lhs->segment_frequency < rhs->segment_frequency;
				}
			);

		/*
			0 terminate the list of segments
		*/
		*current_segment = 0;

		/*
			Process the segments
		*/
		jass_query->rewind(smallest_possible_rsv, largest_possible_rsv);
//std::cout << "MAXRSV:" << largest_possible_rsv << "\n";

		size_t postings_processed = 0;
		for (uint64_t *current = segment_order; current < current_segment; current++)
			{
//std::cout << "Process Segment->(" << ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + *current))->impact << ":" << ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + *current))->segment_frequency << ")\n";
			const JASS::deserialised_jass_v1::segment_header &header = *reinterpret_cast<const JASS::deserialised_jass_v1::segment_header *>(index.postings() + *current);

			/*
				The anytime algorithms basically boils down to this... have we processed enough postings yet?  If so then stop
				The definition of "enough" is that processing the next segment will exceed postings_to_process so we wil be over
				the "time limit" so we must not do it.
			*/
			if (postings_processed + header.segment_frequency > postings_to_process)
				break;
			postings_processed += header.segment_frequency;

			/*
				Process the postings
			*/
			JASS::query::ACCUMULATOR_TYPE impact = header.impact;
			jass_query->decode_and_process(impact, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);
			}

		jass_query->sort();
		
		/*
			stop the timer
		*/
		output.search_time_in_ns += JASS::timer::stop(total_search_time).nanoseconds();

		/*
			Serialise the results list (don't time this)
		*/
		JASS::run_export(JASS::run_export::TREC, output.results_list, query_id.c_str(), *jass_query, "JASSv2", true);

		/*
			Re-start the timer
		*/
		total_search_time = JASS::timer::start();

		/*
			get the next query
		*/
		query = JASS_anytime_query::get_next_query(query_list, next_query);
		}

	/*
		clean up
	*/
	delete [] segment_order;
	}

/*
	MAKE_INPUT_CHANNEL()
	--------------------
*/
std::unique_ptr<JASS::channel> make_input_channel(std::string filename)
	{
	std::string file;
	JASS::file::read_entire_file(filename, file);
	/*
		If the start of the file is a digit then we think we have a JASS topic file.
		If the start is not a digit then we're expecting to see a TREC topic file.
	*/
	if (::isdigit(file[0]))
		return std::unique_ptr<JASS::channel_file>(new JASS::channel_file(filename));				// JASS topic file
	else
		{
		std::unique_ptr<JASS::channel> source(new JASS::channel_file(filename));
		return std::unique_ptr<JASS::channel_trec>(new JASS::channel_trec(source, "tq"));		// TREC topic file
		}
	}

/*
	USAGE()
	-------
*/
uint8_t usage(const std::string &exename)
	{
	std::cout << JASS::commandline::usage(exename, parameters) << "\n";
	return 1;
	}

/*
	MAIN_EVENT()
	------------
*/
int main_event(int argc, const char *argv[])
	{
	auto total_run_time = JASS::timer::start();
	/*
		Parse the commane line parameters
	*/
	auto success = JASS::commandline::parse(argc, argv, parameters, parameters_errors);
	if (!success)
		{
		std::cout << parameters_errors;
		exit(1);
		}
	if (parameter_help)
		exit(usage(argv[0]));

	if (parameter_top_k > MAX_TOP_K)
		{
		std::cout << "top-k specified (" << parameter_top_k << ") is larger than maximum TOP-K (" << MAX_TOP_K << "), change MAX_TOP_K in " << __FILE__  << " and recompile.\n";
		exit(1);
		}

	/*
		Run-time statistics
	*/
	JASS_anytime_stats stats;
	stats.threads = parameter_threads;

	/*
		Read the index
	*/
	JASS::deserialised_jass_v1 index(true);
	index.read_index();

	if (index.document_count() > MAX_DOCUMENTS)
		{
		std::cout << "There are " << index.document_count() << " documents in this index which is larger than MAX_DOCUMENTS (" << MAX_DOCUMENTS << "), change MAX_DOCUMENTS in " << __FILE__ << " and recompile.\n";
		exit(1);
		}
	stats.number_of_documents = index.document_count();

	/*
		Set the Anytime stopping criteria
	*/
	size_t postings_to_process = (std::numeric_limits<size_t>::max)();

	if (maximum_number_of_postings_to_process != 0)
		postings_to_process = maximum_number_of_postings_to_process;

	if (rho != 100.0)
		postings_to_process = static_cast<size_t>(static_cast<double>(index.document_count()) * rho / 100.0);

std::cout << "Maximum number of postings to process:" << postings_to_process << "\n";

	/*
		Read from the query file into a list of queries array.
	*/
	std::unique_ptr<JASS::channel> input = make_input_channel(parameter_queryfilename);		// read from here
	std::string query;												// the channel read goes into memory managed by this object

	/*
		Read the query set and bung it into a vector
	*/
	std::vector<JASS_anytime_query> query_list;

	input->gets(query);

	std::size_t found = query.find_last_not_of(" \t\f\v\n\r");
	if (found != std::string::npos)
	  query.erase(found + 1);
	else
	  query.clear();            // str is all whitespace

	while (query.size() != 0)
		{
		query_list.push_back(query);
		stats.number_of_queries++;

		input->gets(query);

		std::size_t found = query.find_last_not_of(" \t\f\v\n\r");
		if (found != std::string::npos)
		  query.erase(found + 1);
		else
		  query.clear();            // str is all whitespace
		}

	/*
		Allocate a thread pool and the place to put the answers
	*/
	std::vector<JASS::thread> thread_pool;
	std::vector<JASS_anytime_thread_result> output;
	output.resize(parameter_threads);

	/*
		Extract the compression scheme from the index
	*/
	std::string codex_name;
	int32_t d_ness;
	index.codex(codex_name, d_ness);
	std::cout << "Index compressed with " << codex_name << "-D" << d_ness << "\n";

	/*
		Start the work
	*/
	auto total_search_time = JASS::timer::start();
	if (parameter_threads == 1)
		{
		anytime(output[0], index, query_list, postings_to_process, parameter_top_k);
		}
	else
		{
		/*
			Multiple threads, so start each worker
		*/
		for (size_t which = 0; which < parameter_threads ; which++)
			thread_pool.push_back(JASS::thread(anytime, std::ref(output[which]), std::ref(index), std::ref(query_list), postings_to_process, parameter_top_k));
		/*
			Wait until they're all done (blocking on the completion of each thread in turn)
		*/
		for (auto &thread : thread_pool)
			thread.join();
		}

	stats.wall_time_in_ns = JASS::timer::stop(total_search_time).nanoseconds();

	/*
		Compute the per-thread stats
	*/
	for (size_t which = 0; which < parameter_threads ; which++)
		stats.sum_of_CPU_time_in_ns += output[which].search_time_in_ns;

	/*
		Dump the answer
	*/
	std::ostringstream TREC_file;
	for (auto &result : output)
		if ((size_t)result.results_list.tellp() != 0)
			TREC_file << result.results_list.str();

	if ((size_t)TREC_file.tellp() != 0)
		JASS::file::write_entire_file("ranking.txt", TREC_file.str());

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
