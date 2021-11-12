/*
	JASS_ANYTIME_API.H
	------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "top_k_limit.h"
#include "parser_query.h"
#include "JASS_anytime_query.h"
#include "JASS_anytime_stats.h"
#include "deserialised_jass_v2.h"
#include "JASS_anytime_thread_result.h"

enum JASS_ERROR
	{
	JASS_ERROR_OK = 0,							///< Completed successfully without error
	JASS_ERROR_BAD_INDEX_VERSION,				///< The index version number specified is not supported
	JASS_ERROR_FAIL,								///< An exception occurred - probably not caused by JASS (might be a C++ RTL exception)
	JASS_ERROR_TOO_MANY_DOCUMENTS,			///< This index cannot be loaded by this instance of the APIs because it contains more documents than the system-wide maximum
	JASS_ERROR_TOO_LARGE,						///< top-k is larger than the system-wide maximum top-k value (or the accumulator width is too large)
	JASS_ERROR_NO_INDEX,							///< The index must be loaded before this operation can occur
	};

class JASS_anytime_api
	{
	private:
		static constexpr size_t MAX_QUANTUM = 0x0FFF;
		static constexpr size_t MAX_TERMS_PER_QUERY = 1024;

	private:
		JASS::deserialised_jass_v1 *index;
		JASS::top_k_limit *precomputed_minimum_rsv_table;
		size_t postings_to_process;
		size_t top_k;
		JASS::parser_query::parser_type which_query_parser;
		size_t accumulator_width;
		JASS_anytime_stats stats;

	private:
		void anytime(JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, JASS::top_k_limit &precomputed_minimum_rsv_table, size_t postings_to_process, size_t top_k);

	public:
		JASS_anytime_api()
			{
			index = nullptr;
			precomputed_minimum_rsv_table = new JASS::top_k_limit;
			postings_to_process = (std::numeric_limits<size_t>::max)();
			top_k = 10;
			which_query_parser = JASS::parser_query::parser_type::query;
			accumulator_width = 0;
			stats.threads = 1;
			}

		virtual ~JASS_anytime_api()
			{
			delete index;
			delete precomputed_minimum_rsv_table;
			}

		JASS_ERROR load_index(size_t index_version = 1)
			{
			try
				{
				switch (index_version)
					{
					case 1:
						index = new JASS::deserialised_jass_v2(true);
						break;
					case 2:
						index = new JASS::deserialised_jass_v1(true);
						break;
					default:
						return JASS_ERROR_BAD_INDEX_VERSION;
					}
				index->read_index();

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

		JASS_ERROR load_oracle_scores(std::string filename)
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

		JASS_ERROR set_postings_to_process_proportion(double percent)
			{
			if (index == nullptr)
				return JASS_ERROR_NO_INDEX;

			postings_to_process = (size_t)((double)index->document_count() * percent / 100.0);

			return JASS_ERROR_OK;
			}

		JASS_ERROR set_postings_to_process(size_t count)
			{
			postings_to_process = count;

			return JASS_ERROR_OK;
			}

		JASS::query::DOCID_TYPE get_postings_to_process(void)
			{
			return postings_to_process;
			}

		JASS_ERROR set_top_k(size_t k)
			{
			if (k > JASS::query::MAX_TOP_K)
				return JASS_ERROR_TOO_LARGE;
				
			top_k = k;

			return JASS_ERROR_OK;
			}

		JASS::query::DOCID_TYPE get_document_count(void)
			{
			if (index == nullptr)
				return JASS_ERROR_NO_INDEX;

			return index->document_count();
			}

		JASS::query::DOCID_TYPE get_top_k(void)
			{
			return top_k;
			}

		JASS::query::DOCID_TYPE get_max_top_k(void)
			{
			return JASS::query::MAX_TOP_K;
			}

		JASS_ERROR get_encoding_scheme(std::string &codex_name, int32_t &d_ness)
			{
			if (index == nullptr)
				return JASS_ERROR_NO_INDEX;

			index->codex(codex_name, d_ness);

			return JASS_ERROR_OK;
			}

		JASS_ERROR use_ascii_parser(void)
			{
			which_query_parser = JASS::parser_query::parser_type::raw;
			return JASS_ERROR_OK;
			}

		JASS_ERROR use_query_parser(void)
			{
			which_query_parser = JASS::parser_query::parser_type::query;
			return JASS_ERROR_OK;
			}

		JASS_ERROR set_accumulator_width(size_t width)
			{
			if (width > 32)
				return JASS_ERROR_TOO_LARGE;

			accumulator_width = width;
			return JASS_ERROR_OK;
			}

		JASS_ERROR search(std::string &query)
			{
			JASS_anytime_thread_result output;
			std::vector<JASS_anytime_query> query_list;

			query_list.push_back(query);
			anytime(output, *index, query_list, *precomputed_minimum_rsv_table, postings_to_process, top_k);

			return JASS_ERROR_OK;
			}
	} ;
