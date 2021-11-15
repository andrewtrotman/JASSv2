/*
	JASS_ANYTIME_API.H
	------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief JASS API for calling from C++ and through SWIG
	@author Andrew Trotman
	@copyright 2021 Andrew Trotman

*/
#include "threads.h"
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
		static void anytime_bootstrap(JASS_anytime_api *thiss, JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, JASS::top_k_limit *precomputed_minimum_rsv_table, size_t postings_to_process, size_t top_k);

	public:
		JASS_anytime_api();
		virtual ~JASS_anytime_api();

		JASS_ERROR load_index(size_t index_version = 1);
		JASS_ERROR load_oracle_scores(std::string filename);

		JASS_ERROR set_postings_to_process_proportion(double percent);
		JASS_ERROR set_postings_to_process(size_t count);
		JASS::query::DOCID_TYPE get_postings_to_process(void);
		JASS_ERROR set_top_k(size_t k);
		JASS::query::DOCID_TYPE get_document_count(void);
		JASS::query::DOCID_TYPE get_top_k(void);
		JASS::query::DOCID_TYPE get_max_top_k(void);
		JASS_ERROR get_encoding_scheme(std::string &codex_name, int32_t &d_ness);

		JASS_ERROR use_ascii_parser(void);
		JASS_ERROR use_query_parser(void);
		JASS_ERROR set_accumulator_width(size_t width);

		JASS_ERROR search(JASS_anytime_thread_result &output, std::string &query);
		JASS_ERROR search(std::vector<JASS_anytime_thread_result> &output, std::vector<JASS_anytime_query> &query_list, size_t thread_count);

	} ;
