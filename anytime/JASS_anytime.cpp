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
#include "query.h"
#include "threads.h"
#include "decode_d0.h"
#include "run_export.h"
#include "commandline.h"
#include "channel_file.h"
#include "compress_integer.h"
#include "JASS_anytime_stats.h"
#include "JASS_anytime_query.h"
#include "deserialised_jass_v1.h"
#include "JASS_anytime_thread_result.h"

constexpr size_t MAX_QUANTUM = 0x0FFF;
constexpr size_t MAX_TERMS_PER_QUERY = 1024;

constexpr size_t MAX_DOCUMENTS = 50'000'000;
constexpr size_t MAX_TOP_K = 1'000;

/*
	PARAMETERS
	----------
*/
std::string parameter_queryfilename;				///< Name of file containing the queries
size_t parameter_threads = 1;							///< Number of concurrent queries
size_t parameter_top_k = 10;							///< Number of results to return
bool parameter_help = false;

std::string parameters_errors;						///< Any errors as a result of command line parsing
auto parameters = std::make_tuple					///< The  command line parameter block
	(
	JASS::commandline::parameter("-?", "--help", "Print this help.", parameter_help),
	JASS::commandline::parameter("-q", "--queryfile", "<filename>  Name of file containing a list of queries (1 per line, each line prefixed with query-id)", parameter_queryfilename),
	JASS::commandline::parameter("-t", "--threads",   "<threadcount> Number of threads to use (one query per thread) [default = -t1]", parameter_threads),
	JASS::commandline::parameter("-k", "--top-k",     "<top-k>      Number of results to return to the user (top-k value) [default = -k10]", parameter_top_k)
	);

/*
	ANYTIME()
	---------
*/
template <typename DECODER>
void anytime(JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, size_t postings_to_process, size_t top_k)
	{
	/*
		Extract the compression scheme from the index
	*/
	std::string codex_name;
	JASS::compress_integer &decompressor = index.codex(codex_name);
	auto decoder = new DECODER(index.document_count() + 4096);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

	/*
		Allocate the Score-at-a-Time table
	*/
	uint64_t *segment_order = new uint64_t [MAX_TERMS_PER_QUERY * MAX_QUANTUM];
	uint64_t *current_segment;

	/*
		Allocate a JASS query object
	*/
	JASS::query<uint16_t, MAX_DOCUMENTS, MAX_TOP_K> *jass_query;
	try
		{
		jass_query = new JASS::query<uint16_t, MAX_DOCUMENTS, MAX_TOP_K>(index.primary_keys(), index.document_count(), top_k);
		}
	catch (std::bad_array_new_length &error)
		{
		exit(printf("Can't load index as the number of documents is too large - change MAX_DOCUMENTS in %s\n", __FILE__));
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
		current_segment = segment_order;
		for (const auto &term : terms)
			{
//	std::cout << "TERM:" << term << "\n";

			/*
				Get the metadata for this term (and if this term isn't in the vocab them move on to the next term)
			*/
			JASS::deserialised_jass_v1::metadata metadata;
			if (!index.postings_details(metadata, term))
				continue;

			/*
				Add to the list of imact segments that need to be processed
			*/
			std::copy((uint64_t *)(metadata.offset), (uint64_t *)(metadata.offset) + metadata.impacts, current_segment);
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
		jass_query->rewind();

		size_t postings_processed = 0;
		for (uint64_t *current = segment_order; current < current_segment; current++)
			{
//	std::cout << "Process Segment->(" << ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + *current))->impact << ":" << ((JASS::deserialised_jass_v1::segment_header *)(index.postings() + *current))->segment_frequency << ")\n";
			const JASS::deserialised_jass_v1::segment_header &header = *reinterpret_cast<const JASS::deserialised_jass_v1::segment_header *>(index.postings() + *current);

			/*
				The anytime algorithms basically boils down to this... have we processed enough postings yet?  If so then stop
			*/
			if (postings_processed + header.segment_frequency > postings_to_process)
				break;
			postings_processed += header.segment_frequency;

			/*
				Process the postings
			*/
			uint16_t impact = header.impact;
			decoder->decode(decompressor, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);
			decoder->process(impact, *jass_query);
			}

		jass_query->sort();
		
		/*
			stop the timer
		*/
		output.search_time_in_ns += JASS::timer::stop(total_search_time).nanoseconds();

		/*
			Serialise the results list (don't time this)
		*/
		JASS::run_export(JASS::run_export::TREC, output.results_list, query_id.c_str(), *jass_query, "COMPILED", true);

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
	delete jass_query;
	delete [] segment_order;
	delete decoder;
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
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
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


	/*
		Set the Anytime stopping criteria
	*/
	size_t postings_to_process = (std::numeric_limits<size_t>::max)();

	/*
		Read from the query file into a list of queries array.
	*/
	JASS::channel_file input(parameter_queryfilename);		// read from here
	std::string query;												// the channel read goes into here

	/*
		Read the query set and bung it into a vector
	*/
	std::vector<JASS_anytime_query> query_list;
	input.gets(query);
	while (query.size() != 0)
		{
		query_list.push_back(query);
		input.gets(query);
		stats.number_of_queries++;
		}

	/*
		Allocate a thread pool and the place to put the answers
	*/
	std::vector<JASS::thread> thread_pool;
	std::vector<JASS_anytime_thread_result> output;
	output.resize(parameter_threads);

	/*
		Start the work
	*/
	std::string codex_name;
	index.codex(codex_name);
	uint32_t d_ness = codex_name == "None" ? 0 : 1;

	/*
		Start the work
	*/
	auto total_search_time = JASS::timer::start();
	if (parameter_threads == 1)
		{
		/*
			We have only 1 thread so don't bother to start a thread to do the work
		*/
		switch (d_ness)
			{
			case 0:
					anytime<JASS::decoder_d0>(output[0], index, query_list, postings_to_process, parameter_top_k);
				break;
			default:
					anytime<JASS::decoder_d1>(output[0], index, query_list, postings_to_process, parameter_top_k);
				break;
			}
		}
	else
		{
		/*
			Multiple threads, so start each worker
		*/
		for (size_t which = 0; which < parameter_threads ; which++)
			switch (d_ness)
				{
				case 0:
						thread_pool.push_back(JASS::thread(anytime<JASS::decoder_d0>, std::ref(output[which]), std::ref(index), std::ref(query_list), postings_to_process, parameter_top_k));
					break;
				default:
						thread_pool.push_back(JASS::thread(anytime<JASS::decoder_d1>, std::ref(output[which]), std::ref(index), std::ref(query_list), postings_to_process, parameter_top_k));
					break;
				}
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

	std::cout << stats;

	return 0;
	}
