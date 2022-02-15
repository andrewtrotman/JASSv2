/*
	JASS_ANYTIME_API.H
	------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief JASS API for calling from C++ and other languages (through SWIG)
	@author Andrew Trotman
	@copyright 2021 Andrew Trotman
*/
#pragma once

#include "top_k_limit.h"
#include "parser_query.h"
#include "JASS_anytime_query.h"
#include "JASS_anytime_stats.h"
#include "deserialised_jass_v2.h"
#include "JASS_anytime_result.h"
#include "JASS_anytime_thread_result.h"

/*
	ENUM JASS_ERROR
	---------------
*/
/*!
	@enum JASS_ERROR
*/
enum JASS_ERROR
	{
	JASS_ERROR_NO_INDEX = -1,				///< The index must be loaded before this operation can occur
	JASS_ERROR_OK = 0,						///< Completed successfully without error
	JASS_ERROR_BAD_INDEX_VERSION,			///< The index version number specified is not supported
	JASS_ERROR_FAIL,							///< An exception occurred - probably not caused by JASS (might be a C++ RTL exception)
	JASS_ERROR_TOO_MANY_DOCUMENTS,		///< This index cannot be loaded by this instance of the APIs because it contains more documents than the system-wide maximum
	JASS_ERROR_TOO_LARGE,					///< top-k is larger than the system-wide maximum top-k value (or the accumulator width is too large)
	};

/*
	CLASS JASS_ANYTIME_API
	----------------------
*/
/*!
	@class JASS_anytime_api
	@brief The API to JASS's anytime seach engine
*/
class JASS_anytime_api
	{
	private:
		static constexpr size_t MAX_QUANTUM = 0x0FFF;			///< The maximum number of segments in a query
		static constexpr size_t MAX_TERMS_PER_QUERY = 1024;	///< The maximum number of terms in a query

	private:
		JASS::deserialised_jass_v1 *index;							///< The index
		JASS::top_k_limit *precomputed_minimum_rsv_table;		///< Oracle scores (estimates of the rsv for the document at k)
		size_t postings_to_process;									///< The maximunm number of postings to process
		size_t postings_to_process_min;								///< Process at least this number of postings
		double relative_postings_to_process;						///< If not 1 then then this is the proportion of this query's postings that should be processed
		size_t top_k;														///< The number of documents we want in the results list
		JASS::parser_query::parser_type which_query_parser;	///< Use the simple ASCII parser or the regular query parser
		size_t accumulator_width;										///< Width of the accumulator array
		JASS_anytime_stats stats;										///< Stats for this "session"

	private:
		/*
			JASS_ANYTIME_API::JASS_ANYTIME_API()
			------------------------------------
			Boilerplate the class to prevent assignment and copying.
		*/
		JASS_anytime_api(JASS_anytime_api &from) = delete;
		JASS_anytime_api(JASS_anytime_api &&from) = delete;

		/*
			JASS_ANYTIME_API::OPERATOR=()
			-----------------------------
			Boilerplate the class to prevent assignment and copying.
		*/
		JASS_anytime_api &operator=(JASS_anytime_api &from) = delete;
		JASS_anytime_api &operator=(JASS_anytime_api &&from) = delete;

		/*
			JASS_ANYTIME_API::ANYTIME()
			---------------------------
		*/
		/*!
         @brief This method calls into the search engine with a set of queries and retrieves a set of results for each
         @param output [out] The results for each query
         @param index [in] The indexes to use to search
         @param query_ist [in] The list of queries to perform
         @param precomputed_minimum_rsv_table [in] The list of rsv@k scores for each query, keyed on the query id
         @param postings_to_process [in] The maximum number of postings to process
         @param top_k [in] The number of results that should be in the results list
		*/
		void anytime(JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, JASS::top_k_limit &precomputed_minimum_rsv_table, size_t postings_to_process, size_t top_k);

		/*
			JASS_ANYTIME_API::ANYTIME_BOOTSTRAP()
			-------------------------------------
		*/
		/*!
         @brief Bootstrapping method for a thread to call into anyrime()
         @param thiss [in] Pointer to the object to call into
         @param output [out] The results for each query
         @param index [in] The indexes to use to search
         @param query_ist [in] The list of queries to perform
         @param precomputed_minimum_rsv_table [in] The list of rsv@k scores for each query, keyed on the query id
         @param postings_to_process [in] The maximum number of postings to process
         @param top_k [in] The number of results that should be in the results list
		*/
		static void anytime_bootstrap(JASS_anytime_api *thiss, JASS_anytime_thread_result &output, const JASS::deserialised_jass_v1 &index, std::vector<JASS_anytime_query> &query_list, JASS::top_k_limit *precomputed_minimum_rsv_table, size_t postings_to_process, size_t top_k);

	public:

		/*
			JASS_ANYTIME_API::JASS_ANYTIME_API()
			------------------------------------
		*/
		/*!
         @brief Constructor
		*/
		JASS_anytime_api();

		/*
			JASS_ANYTIME_API::~JASS_ANYTIME_API()
			-------------------------------------
		*/
		/*!
         @brief Destructor
		*/
		virtual ~JASS_anytime_api();

		/*
			JASS_ANYTIME_API::LOAD_INDEX()
			------------------------------
		*/
		/*!
         @brief Load a JASS index from the given directory.
         @param index_version [in] What verison of the index is this - normally 2.
         @param directory[in] The path to the index, default = "."
         @param verbose [in] if true, diagnostics are printed while the index is loading, default = false
         @return JASS_ERROR_OK on success, else an error code.
		*/
		JASS_ERROR load_index(size_t index_version, const std::string &directory = "", bool verbose = false);		// verbose prints progress as it loads the index

		/*
			JASS_ANYTIME_API::GET_DOCUMENT_COUNT()
			--------------------------------------
		*/
		/*!
         @brief Return the number of documents in the index
         @return The number of documetns in the collection, or 0 if no index has been loaded
		*/
		uint32_t get_document_count(void);

		/*
			JASS_ANYTIME_API::GET_ENCODING_SCHEME()
			---------------------------------------
		*/
		/*!
         @brief Return the name of the compression algorithm and the delta (d-gap) size
         @details This method might not be exposed to languages (such as Python) that do not support non-const references to strings, use get_encoding_scheme_name() or get_encoding_scheme_d() instead.
         @param codex_name [out] The compression scheme
         @param d_ness [out] The d-gap size (normally 1)]
         @return JASS_ERROR_OK or JASS_ERROR_NO_INDEX if an index has not yet been loaded
		*/
		JASS_ERROR get_encoding_scheme(std::string &codex_name, int32_t &d_ness);

		/*
			JASS_ANYTIME_API::GET_ENCODING_SCHEME_NAME()
			--------------------------------------------
		*/
		/*!
         @brief Return the name of the compression algorithm used on this index
         @return The name of the compression algorithm, or an empty string if no index has been loaded
		*/
		std::string get_encoding_scheme_name(void);

		/*
			JASS_ANYTIME_API::GET_ENCODING_SCHEME_D()
			-----------------------------------------
		*/
		/*!
         @brief Return the d-gap value used in this index
         @return The d-gap value or JASS_ERROR_NO_INDEX if no index has been loaded.
		*/
		int32_t get_encoding_scheme_d(void);


		/*
			JASS_ANYTIME_API::LOAD_ORACLE_SCORES()
			--------------------------------------
		*/
		/*!
         @brief Load a JASS index
         @param filename [in] The name of the file containing the oracle scores
         @details The format of the orace file is lines of  "<QueryId> <rsv>" Where <QueryId> is the ID of the query, and <rsv> is the lowest rsv required to enter the top-k
         @return JASS_ERROR_OK on success, else an error code
		*/
		JASS_ERROR load_oracle_scores(std::string filename);

		/*
			JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_PROPORTION()
			------------------------------------------------------
		*/
		/*!
         @brief Set the number of postings to process as a proportion of the number of documents in the collection.
         @details An index must be loaded before this method is called, if not it returns JASS_ERROR_NO_INDEX and has no effect Which ever of set_postings_to_process() and set_postings_to_process_proportion() is called last takes presidence.  By default all postings are processed.
         @param percent [in] The percent to use (for example, 10 is use 10% of the postings)
         @return JASS_ERROR_OK or JASS_ERROR_NO_INDEX
		*/
		JASS_ERROR set_postings_to_process_proportion(double percent);


		/*
			JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_PROPORTION_MINIMUM()
			--------------------------------------------------------------
		*/
		/*!
         @brief Set the minimum number of postings to process as a proportion of the number of documents in the collection.
         @details An index must be loaded before this method is called, if not it returns JASS_ERROR_NO_INDEX and has no effect.  By default all postings are processed.
         @param percent [in] The percent to use (for example, 10 is use 10% of the postings)
         @return JASS_ERROR_OK or JASS_ERROR_NO_INDEX
		*/
		JASS_ERROR set_postings_to_process_proportion_minimum(double percent);

		/*
			JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS_RELATIVE()
			----------------------------------------------------
		*/
		/*!
         @brief Set the number of postings to process as a proportion of the number of postings for this query.
         @details An index does not need to be loaded first.  This method takes precidence over set_postings_to_process() and set_postings_to_process_proportion().  By default all postings are processed.
         @param percent [in] The percent to use (for example, 10 is use 10% of the postings)
         @return JASS_ERROR_OK or JASS_ERROR_NO_INDEX
		*/
		JASS_ERROR set_postings_to_process_relative(double percent);


		/*
			JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS()
			-------------------------------------------
		*/
		/*!
         @brief Set the number of postings to process as an absolute number.
         @details An index does not need to be loaded first.  Which ever of set_postings_to_process() and set_postings_to_process_proportion() is called last takes presidence.  By default all postings are processed.
         @param count [in] The maximum number of postings to process
         @return JASS_ERROR_OK
		*/
		JASS_ERROR set_postings_to_process(size_t count);


		/*
			JASS_ANYTIME_API::SET_POSTINGS_TO_PROCESS__MINIMUM()
			----------------------------------------------------
		*/
		/*!
         @brief Set the minimum number of postings to process as an absolute number.
         @details An index does not need to be loaded first.  By default all postings are processed.
         @param count [in] The minimum number of postings to process
         @return JASS_ERROR_OK
		*/
		JASS_ERROR set_postings_to_process_minimum(size_t count);


		/*
			JASS_ANYTIME_API::GET_POSTINGS_TO_PROCESS()
			-------------------------------------------
		*/
		/*!
         @brief Return the current maimum number of postings to process value.  This might be very large.
         @return The  current maimum number of postings to process in a query.
		*/
		uint32_t get_postings_to_process(void);

		/*
			JASS_ANYTIME_API::GET_MAX_TOP_K()
			---------------------------------
		*/
		/*!
         @brief Return the largest possible top_k value, which might be smaller than the number of documents in the collection
         @return The maximum top-k value (normally 1000)
		*/
		uint32_t get_max_top_k(void);

		/*
			JASS_ANYTIME_API::SET_TOP_K()
			-----------------------------
		*/
		/*!
         @brief Set the maximum numbre of documents to return in a resuts list
         @param k [in] The k value for top-k search
         @return JASS_ERROR_OK or JASS_ERROR_TOO_LARGE if k > get_max_top_k()
		*/
		JASS_ERROR set_top_k(size_t k);

		/*
			JASS_ANYTIME_API::GET_TOP_K()
			-----------------------------
		*/
		/*!
         @brief Return the current top-k value
         @return The current top-k value, the maximum number of documents to put into the results list
		*/
		uint32_t get_top_k(void);

		/*
			JASS_ANYTIME_API::SET_ACCUMULATOR_WIDTH()
			-----------------------------------------
		*/
		/*!
         @brief Set the accumulator page-table width (assuming a page-table like approach is being used)
         @param width [in[ The width of the table will be set to 2^width
         @return JASS_ERROR_OK, or JASS_ERROR_TOO_LARGE if width is too large
		*/
		JASS_ERROR set_accumulator_width(size_t width);

		/*
			JASS_ANYTIME_API::USE_ASCII_PARSER()
			------------------------------------
		*/
		/*!
         @brief Use the query parser that seperates tokens on whitespace along (this metho is not normally needed).
         @return Always returns JASS_ERROR_OK
		*/
		JASS_ERROR use_ascii_parser(void);

		/*
			JASS_ANYTIME_API::USE_QUERY_PARSER()
			------------------------------------
		*/
		/*!
         @brief Use the default query parser that understands alphanumnerics, spaces, unicode, and so on  (this metho is not normally needed).
         @return Always returns JASS_ERROR_OK
		*/
		JASS_ERROR use_query_parser(void);

		/*
			JASS_ANYTIME_API::SEARCH()
			--------------------------
		*/
		/*!
         @brief Search using the current index and the current parameters
         @param query[in] The query.  If the query starts with a numner that number is assumed to be the query_id and NOT a search term (so it is not searched for).
         @return a JASS_anytime_result object with the results.  If not index has been loaded then an empty JASS_anytime_result is returned.
		*/
		JASS_anytime_result search(const std::string &query);

		/*
			JASS_ANYTIME_API::SEARCH()
			--------------------------
		*/
		/*!
         @brief Search using the current index and the current parameters
         @details This method might not be exposed to languages (such as Python) that do not support non-const references to objects.  Search one query at a time using search() and do the threading in the calling laguage if need be.
         @param output [out] a vector of results, one for each thread
         @param query_list [in] A vector of queries to be spread over all the threads
         @param thread_count [in] The number of threads to use for searching
         @return JASS_ERROR_OK, or JASS_ERROR_NO_INDEX if no index has been loaded.
		*/
		JASS_ERROR search(std::vector<JASS_anytime_thread_result> &output, std::vector<JASS_anytime_query> &query_list, size_t thread_count);

		/*
			JASS_ANYTIME_API::THREADED_SEARCH()
			-----------------------------------
		*/
		/*!
         @brief Search using the current index and the current parameters
         @details This method might not be exposed to languages (such as Python) that do not support non-const references to objects.  Search one query at a time using search() and do the threading in the calling laguage if need be.
         @param query_list [in] A vector of queries to be spread over all the threads
         @param thread_count [in] The number of threads to use for searching
         @return On success, a vector of results, one for each thread.  On failure, an empty vvector or results
		*/
		std::vector<JASS_anytime_thread_result> threaded_search(std::vector<std::string> &query_list, size_t thread_count);
	} ;

/*
	CLASS ANYTIME
	-------------
*/
/*!
   @class anytime
   @brief An alias for JASS_anytime_api
*/
class anytime : public JASS_anytime_api
	{
	/*
		Nothing
	*/
	};
