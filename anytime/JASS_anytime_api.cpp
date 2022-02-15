/*
	JASS_ANYTIME_API.CPP
	--------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "timer.h"
#include "threads.h"
#include "run_export.h"
#include "top_k_limit.h"
#include "parser_query.h"
#include "JASS_anytime_api.h"
#include "JASS_anytime_query.h"
#include "JASS_anytime_stats.h"
#include "deserialised_jass_v2.h"
#include "JASS_anytime_thread_result.h"

/*
	JASS_ANYTIME_API::ANYTIME_BOOTSTRAP()
	-------------------------------------
*/
void JASS_anytime_api::anytime_bootstrap(JASS_anytime_api *thiss, JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, JASS::top_k_limit *precomputed_minimum_rsv_table, size_t postings_to_process, size_t top_k)
	{
	thiss->anytime(output, index, query_list, *precomputed_minimum_rsv_table, postings_to_process, top_k);
	}

/*
	JASS_ANYTIME_API::JASS_ANYTIME_API()
	------------------------------------
*/
JASS_anytime_api::JASS_anytime_api()
	{
	index = nullptr;
	precomputed_minimum_rsv_table = new JASS::top_k_limit;
	postings_to_process = (std::numeric_limits<size_t>::max)();
	postings_to_process_min = 0;
	relative_postings_to_process = 1;
	top_k = 10;
	which_query_parser = JASS::parser_query::parser_type::query;
	accumulator_width = 0;
	stats.threads = 1;
	}

/*
	JASS_ANYTIME_API::~JASS_ANYTIME_API()
	-------------------------------------
*/
JASS_anytime_api::~JASS_anytime_api()
	{
	delete index;
	delete precomputed_minimum_rsv_table;
	}

/*
	JASS_ANYTIME_API::LOAD_INDEX()
	------------------------------
*/
JASS_ERROR JASS_anytime_api::load_index(size_t index_version, const std::string &directory, bool verbose)
	{
	try
		{
		switch (index_version)
			{
			case 1:
				index = new JASS::deserialised_jass_v1(verbose);
				break;
			case 2:
				index = new JASS::deserialised_jass_v2(verbose);
				break;
			default:
				return JASS_ERROR_BAD_INDEX_VERSION;
			}

		index->read_index(directory);

		if (index->document_count() > JASS::query::MAX_DOCUMENTS)
			{
			delete index;
			index = nullptr;
			return JASS_ERROR_TOO_MANY_DOCUMENTS;
			}

		return JASS_ERROR_OK;
		}
	catch (...)
		{
		return JASS_ERROR_FAIL;
		}
	}

/*
	JASS_ANYTIME_API::LOAD_ORACLE_SCORES()
	--------------------------------------
*/
JASS_ERROR JASS_anytime_api::load_oracle_scores(std::string filename)
	{
	try
		{
		delete precomputed_minimum_rsv_table;
		precomputed_minimum_rsv_table = new JASS::top_k_limit(filename);
		}
	catch(...)
		{
		return JASS_ERROR_FAIL;
		}

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_PROPORTION()
	------------------------------------------------------
*/
JASS_ERROR JASS_anytime_api::set_postings_to_process_proportion(double percent)
	{
	if (index == nullptr)
		return JASS_ERROR_NO_INDEX;

	postings_to_process = (size_t)((double)index->document_count() * percent / 100.0);

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_PROPORTION_MINIMUM()
	--------------------------------------------------------------
*/
JASS_ERROR JASS_anytime_api::set_postings_to_process_proportion_minimum(double percent)
	{
	if (index == nullptr)
		return JASS_ERROR_NO_INDEX;

	postings_to_process_min = (size_t)((double)index->document_count() * percent / 100.0);

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_RELATIVE()
	----------------------------------------------------
*/
JASS_ERROR JASS_anytime_api::set_postings_to_process_relative(double percent)
	{
	relative_postings_to_process = percent / 100.0;

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS()
	-------------------------------------------
*/
JASS_ERROR JASS_anytime_api::set_postings_to_process(size_t count)
	{
	postings_to_process = count;

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_MINIMUM()
	---------------------------------------------------
*/
JASS_ERROR JASS_anytime_api::set_postings_to_process_minimum(size_t count)
	{
	postings_to_process_min = count;

	return JASS_ERROR_OK;
	}


/*
	JASS_ANYTIME_API::GET_POSTINGS_TO_PROCESS()
	-------------------------------------------
*/
JASS::query::DOCID_TYPE JASS_anytime_api::get_postings_to_process(void)
	{
	return postings_to_process;
	}

/*
	JASS_ANYTIME_API::SET_TOP_K()
	-----------------------------
*/
JASS_ERROR JASS_anytime_api::set_top_k(size_t k)
	{
	if (k > JASS::query::MAX_TOP_K)
		return JASS_ERROR_TOO_LARGE;

	top_k = k;

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::GET_DOCUMENT_COUNT()
	--------------------------------------
*/
JASS::query::DOCID_TYPE JASS_anytime_api::get_document_count(void)
	{
	if (index == nullptr)
		return 0;				// no index has been loaded

	return index->document_count();
	}

/*
	JASS_ANYTIME_API::GET_TOP_K()
	-----------------------------
*/
JASS::query::DOCID_TYPE JASS_anytime_api::get_top_k(void)
	{
	return top_k;
	}

/*
	JASS_ANYTIME_API::GET_MAX_TOP_K()
	---------------------------------
*/
JASS::query::DOCID_TYPE JASS_anytime_api::get_max_top_k(void)
	{
	return JASS::query::MAX_TOP_K;
	}

/*
	JASS_ANYTIME_API::GET_ENCODING_SCHEME()
	---------------------------------------
*/
JASS_ERROR JASS_anytime_api::get_encoding_scheme(std::string &codex_name, int32_t &d_ness)
	{
	if (index == nullptr)
		return JASS_ERROR_NO_INDEX;

	index->codex(codex_name, d_ness);

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::GET_ENCODING_SCHEME_NAME()
	--------------------------------------------
*/
std::string JASS_anytime_api::get_encoding_scheme_name(void)
	{
	std::string codex_name;
	int32_t d_ness;

	if (get_encoding_scheme(codex_name, d_ness) == JASS_ERROR_OK)
		return codex_name;
	else
		return "";
	}

/*
	JASS_ANYTIME_API::GET_ENCODING_SCHEME_D()
	-----------------------------------------
*/
int32_t JASS_anytime_api::get_encoding_scheme_d(void)
	{
	std::string codex_name;
	int32_t d_ness;

	if (get_encoding_scheme(codex_name, d_ness) == JASS_ERROR_OK)
		return d_ness;
	else
		return JASS_ERROR_NO_INDEX;
	}

/*
	JASS_ANYTIME_API::USE_ASCII_PARSER()
	------------------------------------
*/
JASS_ERROR JASS_anytime_api::use_ascii_parser(void)
	{
	which_query_parser = JASS::parser_query::parser_type::raw;
	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::USE_QUERY_PARSER()
	------------------------------------
*/
JASS_ERROR JASS_anytime_api::use_query_parser(void)
	{
	which_query_parser = JASS::parser_query::parser_type::query;
	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SET_ACCUMULATOR_WIDTH()
	-----------------------------------------
*/
JASS_ERROR JASS_anytime_api::set_accumulator_width(size_t width)
	{
	if (width > 32)
		return JASS_ERROR_TOO_LARGE;

	accumulator_width = width;
	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::SEARCH()
	--------------------------
*/
JASS_anytime_result JASS_anytime_api::search(const std::string &query)
	{
	if (index == nullptr)
		return JASS_anytime_result();
		
	JASS_anytime_thread_result output;
	std::vector<JASS_anytime_query> query_list;

	query_list.push_back(query);
	anytime(output, *index, query_list, *precomputed_minimum_rsv_table, postings_to_process, top_k);

	return output.results.begin()->second;
	}

/*
	JASS_ANYTIME_API::THREADED_SEARCH()
	-----------------------------------
*/
std::vector<JASS_anytime_thread_result> JASS_anytime_api::threaded_search(std::vector<std::string> &query_list, size_t thread_count)
	{
	if (index == nullptr)
		return std::vector<JASS_anytime_thread_result>();

	std::vector<JASS_anytime_query> queries;
	std::vector<JASS_anytime_thread_result> output;

	for (const auto & text : query_list)
		queries.push_back(text);

	if (search(output, queries, thread_count) == JASS_ERROR_OK)
		return output;

	return std::vector<JASS_anytime_thread_result>();
	}

/*
	JASS_ANYTIME_API::SEARCH()
	---------------------------
*/
JASS_ERROR JASS_anytime_api::search(std::vector<JASS_anytime_thread_result> &output, std::vector<JASS_anytime_query> &query_list, size_t thread_count)
	{
	if (index == nullptr)
		return JASS_ERROR_NO_INDEX;

	/*
		Allocate a thread pool and the place to put the answers
	*/
	std::vector<JASS::thread> thread_pool;
	output.resize(thread_count);

	/*
		Do the work either single or multiple threaded
	*/
	if (thread_count == 1)
		anytime(output[0], *index, query_list, *precomputed_minimum_rsv_table, postings_to_process, top_k);
	else
		{
		/*
			Multiple threads, so start each worker
		*/
		for (size_t which = 0; which < thread_count ; which++)
			thread_pool.push_back(JASS::thread(anytime_bootstrap, this, std::ref(output[which]), std::ref(*index), std::ref(query_list), precomputed_minimum_rsv_table, postings_to_process, top_k));

		/*
			Wait until they're all done (blocking on the completion of each thread in turn)
		*/
		for (auto &thread : thread_pool)
			thread.join();
		}

	return JASS_ERROR_OK;
	}

/*
	JASS_ANYTIME_API::ANYTIME()
	---------------------------
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
		uint64_t total_postings_for_query = 0;
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
			JASS::query::DOCID_TYPE document_frequency;
			current_segment += index.get_segment_list(current_segment, metadata, term.frequency(), term_smallest_impact, term_largest_impact, document_frequency);
			total_postings_for_query += document_frequency;

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
			largest_possible_rsv = (uint32_t)((double)largest_possible_rsv / (double)largest_possible_rsv * (double)JASS::query::MAX_RSV);

			/*
				This line (commented out) re-scales the rsv_at_k value, which we need to do if that score comes
				from some other search engine (which is unlikely to occur).
			*/
//			rsv_at_k = (JASS::query::ACCUMULATOR_TYPE)((double)rsv_at_k / (double)largest_possible_rsv * (double)JASS::query::MAX_RSV);

			/*
				Check for zeros
			*/
			smallest_possible_rsv = smallest_possible_rsv == 0 ? 1 : smallest_possible_rsv;
			}
		rsv_at_k = rsv_at_k == 0 ? 1 : rsv_at_k;			// rsv_at_k cannot be 0 (because at least one search term must be in the document)

		jass_query->rewind(smallest_possible_rsv, rsv_at_k, largest_possible_rsv);
//std::cout << "MAXRSV:" << largest_possible_rsv << " MINRSV:" << smallest_possible_rsv << "\n";

		/*
			Check to see if we've got a rho stopping conditio relative to the number of postings in this query.
		*/
		if (relative_postings_to_process != 1)
			postings_to_process = total_postings_for_query * relative_postings_to_process;

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

			/*
				Early terminate if we have filled the heap with documents having rsv scores higher than the rsv_at_k oracle score.
			*/
			if (rsv_at_k > 1 && jass_query->size() >= top_k && postings_processed >= postings_to_process_min)
				break;
			}
		/*
			If were using the oracle rsv_at_k predictions and we have fewer than top_k documents in the top_k list
			then it might be becasue the oracle prediction was too high.  If this is the case then we need to top-up
			the top-k
		*/
		if (rsv_at_k > 1 && jass_query->size() < top_k)
			jass_query->top_up();

		/*
			Finally we have the results list in the heap, no sort it.
		*/
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

