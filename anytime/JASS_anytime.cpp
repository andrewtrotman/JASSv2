/*
	JASS_ANYTIME.CPP
	----------------
	Written bu Andrew Trotman.
	Based on JASS v1, which was written by Andrew Trotman and Jimmy Lin
*/
#include <stdio.h>
#include <stdlib.h>

#include <limits>
#include <fstream>
#include <algorithm>

#include "file.h"
#include "timer.h"
#include "query16_t.h"
#include "decode_d0.h"
#include "run_export.h"
#include "commandline.h"
#include "query_atire.h"
#include "channel_file.h"
#include "compress_integer.h"
#include "JASS_anytime_stats.h"
#include "JASS_anytime_query.h"
#include "query_atire_global.h"
#include "deserialised_jass_v1.h"

#define MAX_QUANTUM 0x0FFF
#define MAX_TERMS_PER_QUERY 1024

/*
	PARAMETERS
	----------
*/
std::string parameter_queryfilename;				///< Name of file containing the queries
size_t parameter_threads = 1;							///< Number of concurrent queries

std::string parameters_errors;						///< Any errors as a result of command line parsing
auto parameters = std::make_tuple					///< The  command line parameter block
	(
	JASS::commandline::parameter("-q", "--queryfile", "Name of file containing a list of queries (1 per line, each line prefixed with query-id)", parameter_queryfilename),
	JASS::commandline::parameter("-t", "--threads", "Number of threads to use (one query per thread) [default = 1]", parameter_threads)
	);

/*
	ANYTIME()
	---------
*/
void anytime(std::ostream &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, size_t postings_to_process)
	{
	size_t segments = 0;
	size_t sum_time = 0;
	size_t zero_time = 0;
	size_t decomp_time = 0;
	size_t process_time = 0;
	/*
		Extract the compression scheme as a decoder
	*/
	JASS::compress_integer &decompressor = index.codex();
	JASS::decoder_d0 decoder(index.document_count() + 1024);			// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

	/*
		Allocate the Score-at-a-Time table
	*/
	uint64_t *segment_order = new uint64_t [MAX_TERMS_PER_QUERY * MAX_QUANTUM];
	uint64_t *current_segment;

	/*
		Now start searching
	*/
	size_t next_query = 0;
	std::string query = JASS_anytime_query::get_next_query(query_list, next_query);

	auto thread_search_time = JASS::timer::start();

	while (query.size() != 0)
		{
		auto preamble_timer = JASS::timer::start();

		/*
			Allocate a JASS query object
		*/
//		JASS::query16_t jass_query(index.primary_keys(), index.document_count(), 10);	// allocate a JASS query object
//		JASS::query_atire<uint16_t> jass_query(index.primary_keys(), index.document_count(), 10);
		JASS::query_atire_global<uint16_t> jass_query(index.primary_keys(), index.document_count(), 10);

		jass_query.parse(query);
		auto &terms = jass_query.terms();
		auto query_id = terms[0];

		/*
			Parse the query and extract the list of impact segments
		*/
		current_segment = segment_order;
		size_t term_id = 0;
		for (const auto &term : terms)
			{
			/*
				Count which term we're on (and ignore the first as its the TREC topic ID)
			*/
			term_id++;
			if (term_id == 1)
				continue;

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
		auto zero_took = JASS::timer::stop(preamble_timer).nanoseconds();
		zero_time += zero_took;

	auto timer = JASS::timer::start();
jass_query.rewind();

		size_t postings_processed = 0;
		for (uint64_t *current = segment_order; current < current_segment; current++)
			{
			segments++;
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
auto decompress_timer = JASS::timer::start();
			decoder.decode(decompressor, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);
auto decompress_took = JASS::timer::stop(decompress_timer).nanoseconds();
decomp_time += decompress_took;


auto process_timer = JASS::timer::start();
			decoder.process(impact, jass_query);
auto process_took = JASS::timer::stop(process_timer).nanoseconds();
process_time += process_took;
			}

jass_query.sort();

		auto took = JASS::timer::stop(timer).nanoseconds();
		sum_time += took;

	//	JASS::run_export(JASS::run_export::TREC, output, (char *)query_id.token().address(), jass_query, "COMPILED", true);
		query = JASS_anytime_query::get_next_query(query_list, next_query);
		}

	auto took = JASS::timer::stop(thread_search_time).nanoseconds();
	std::cout << "time      :" << took << " ns\n";
	std::cout << "Q time    :" << took / 150 << " ns\n";
	std::cout << "sum time  :" << sum_time << " ns\n";
	std::cout << "Qsum time :" << sum_time / 150 << " ns\n";
	std::cout << "zero time :" << zero_time << " ns\n";
	std::cout << "Qzero time:" << zero_time / 150 << " ns\n";
	std::cout << "decode time :" << decomp_time << " ns\n";
	std::cout << "Qdecode time:" << decomp_time / 150 << " ns\n";
	std::cout << "process time :" << process_time << " ns\n";
	std::cout << "Qprocess time:" << process_time / 150 << " ns\n";
	std::cout << "segments processed:" << segments << "\n";
	delete [] segment_order;
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

	/*
		Run-time statistics
	*/
	anytime_stats stats;
	stats.threads = parameter_threads;

	/*
		Read the index
	*/
	JASS::deserialised_jass_v1 index(true);
	index.read_index();

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
	std::vector<std::thread> thread_pool;
	std::vector<std::ostringstream> output;
	output.resize(parameter_threads);

	/*
		Start the work
	*/
	auto total_search_time = JASS::timer::start();
	if (parameter_threads == 1)
		{
		/*
			We have only 1 thread so don't bother to start a thread to do the work
		*/
		anytime(output[0], index, query_list, postings_to_process);
		}
	else
		{
		/*
			Multiple threads, so start each worker
		*/
		for (size_t which = 0; which < parameter_threads ; which++)
			thread_pool.push_back(std::thread(anytime, std::ref(output[which]), std::ref(index), std::ref(query_list), postings_to_process));

		/*
			Wait until they're all done (blocking on the completion of each thread in turn)
		*/
		for (auto &thread : thread_pool)
			thread.join();
		}
	stats.total_search_time_in_ns = JASS::timer::stop(total_search_time).nanoseconds();

	/*
		Dump the answer
	*/
	std::ostringstream TREC_file;
	for (auto &result : output)
		if (result.tellp() != 0)
			TREC_file << result.str();

	if (TREC_file.tellp() != 0)
		JASS::file::write_entire_file("ranking.txt", TREC_file.str());

	std::cout << stats;

	return 0;
	}
