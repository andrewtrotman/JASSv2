/*
	JASS_ANYTIME_API.CPP
	--------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "timer.h"
#include "run_export.h"
#include "JASS_anytime_api.h"
#include "JASS_anytime_query.h"

/*
	ANYTIME()
	---------
*/
void JASS_anytime_api::anytime(JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, JASS::top_k_limit &precomputed_minimum_rsv_table, size_t postings_to_process, size_t top_k)
	{
	/*
		Allocate the Score-at-a-Time table
	*/
	JASS::deserialised_jass_v1::segment_header *segment_order = new JASS::deserialised_jass_v1::segment_header[MAX_TERMS_PER_QUERY * MAX_QUANTUM];

	/*
		Allocate a JASS query object
	*/
	std::string codex_name;
	int32_t d_ness;
	std::unique_ptr<JASS::compress_integer> jass_query = index.codex(codex_name, d_ness);

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

//std::cout << "QUERY:" << query_id << "\n";
		/*
			Process the query
		*/
		jass_query->parse(query, which_query_parser);

		/*
			Parse the query and extract the list of impact segments
		*/
		JASS::deserialised_jass_v1::segment_header *current_segment = segment_order;
		uint32_t largest_possible_rsv = (std::numeric_limits<decltype(largest_possible_rsv)>::min)();
		uint32_t largest_possible_rsv_with_overflow;
		uint32_t smallest_possible_rsv = (std::numeric_limits<decltype(smallest_possible_rsv)>::max)();
//std::cout << "\n";
		for (const auto &term : jass_query->terms())
			{
//std::cout << "TERM:" << term << " ";

			/*
				Get the metadata for this term (and if this term isn't in the vocab them move on to the next term)
			*/
			JASS::deserialised_jass_v1::metadata metadata;
			if (!index.postings_details(metadata, term))
				continue;

			/*
				Add the segments to the list to process
			*/
			uint32_t term_smallest_impact;
			uint32_t term_largest_impact;
			current_segment += index.get_segment_list(current_segment, metadata, term.frequency(), term_smallest_impact, term_largest_impact);

			/*
				Compute the largest and smallest possible rsv values
			*/
			largest_possible_rsv += term_largest_impact;
			smallest_possible_rsv = JASS::maths::minimum(smallest_possible_rsv, (decltype(smallest_possible_rsv))term_smallest_impact);
			}

		/*
			Sort the segments from highest impact to lowest impact
		*/
		std::sort
			(
			segment_order,
			current_segment,
			[postings = index.postings()](JASS::deserialised_jass_v1::segment_header &lhs, JASS::deserialised_jass_v1::segment_header &rhs)
				{

				/*
					sort from highest to lowest impact, but break ties by placing the lowest quantum-frequency first and the highest quantum-frequency last
				*/
				if (lhs.impact < rhs.impact)
					return false;
				else if (lhs.impact > rhs.impact)
					return true;
				else			// impact scores are the same, so tie break on the length of the segment
					return lhs.segment_frequency < rhs.segment_frequency;
				}
			);

		/*
			0 terminate the list of segments by setting the impact score to zero
		*/
		current_segment->impact = 0;

		/*
			Compute the minimum rsv necessary to get into the top k.
			its not yet clear whether we can set the default to the highest segment score, segment_order->impact
		*/
		bool scale_rsv_scores = false;
		uint32_t rsv_at_k = precomputed_minimum_rsv_table.empty() ? 1 : precomputed_minimum_rsv_table[query_id];
		largest_possible_rsv_with_overflow = largest_possible_rsv;
		if (largest_possible_rsv > JASS::query::MAX_RSV)
			{
			scale_rsv_scores = true;
			smallest_possible_rsv = (uint32_t)((double)largest_possible_rsv / (double)largest_possible_rsv * (double)JASS::query::MAX_RSV);
//			rsv_at_k = (JASS::query::ACCUMULATOR_TYPE)((double)rsv_at_k / (double)largest_possible_rsv * (double)JASS::query::MAX_RSV);
			largest_possible_rsv = (uint32_t)((double)largest_possible_rsv / (double)largest_possible_rsv * (double)JASS::query::MAX_RSV);

			/*
				Check for zeros
			*/
			smallest_possible_rsv = smallest_possible_rsv == 0 ? 1 : smallest_possible_rsv;
			}
		rsv_at_k = rsv_at_k == 0 ? 1 : rsv_at_k;			// rsv_at_k cannot be 0 (because at least one search term must be in the document)

		jass_query->rewind(smallest_possible_rsv, rsv_at_k, largest_possible_rsv);
//std::cout << "MAXRSV:" << largest_possible_rsv << " MINRSV:" << smallest_possible_rsv << "\n";

		/*
			Process the segments
		*/
		size_t postings_processed = 0;
		for (auto *header = segment_order; header < current_segment; header++)
			{
			if (scale_rsv_scores)
				header->impact = (JASS::query::ACCUMULATOR_TYPE)((double)header->impact / (double)largest_possible_rsv_with_overflow * (double)JASS::query::MAX_RSV);

//std::cout << "Process Segment->(" << header->impact << ":" << header->segment_frequency << ")\n";
			/*
				The anytime algorithms basically boils down to this... have we processed enough postings yet?  If so then stop
				The definition of "enough" is that processing the next segment will exceed postings_to_process so we wil be over
				the "time limit" so we must not do it.
			*/
			if (postings_processed + header->segment_frequency > postings_to_process)
				break;
			postings_processed += header->segment_frequency;

			/*
				Process the postings
			*/
			JASS::query::ACCUMULATOR_TYPE impact = header->impact;
			jass_query->decode_and_process(impact, header->segment_frequency, index.postings() + header->offset, header->end - header->offset);
#define EARLY_TERMINATE 1
#ifdef EARLY_TERMINATE
			if (rsv_at_k > 1 && jass_query->size() >= top_k)
				break;
#endif
			}

		jass_query->sort();
		
		/*
			stop the timer
		*/
		auto time_taken = JASS::timer::stop(total_search_time).nanoseconds();

		/*
			Serialise the results list (don't time this)
		*/
		std::ostringstream results_list;
#if defined(ACCUMULATOR_64s) || defined(QUERY_HEAP) || defined(QUERY_MAXBLOCK_HEAP)
		JASS::run_export(JASS::run_export::TREC, results_list, query_id.c_str(), *jass_query, "JASSv2", true, true);
#else
		JASS::run_export(JASS::run_export::TREC, results_list, query_id.c_str(), *jass_query, "JASSv2", true, false);
#endif
		/*
			Store the results (and the time it took)
		*/
		output.push_back(query_id, query, results_list.str(), postings_processed, time_taken);

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

