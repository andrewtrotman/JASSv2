/*
	JASS_ANYTIME_THREAD_RESULT.H
	----------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The results of a single thread in the Anytime engine.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <map>

#include "JASS_anytime_result.h"

/*
	CLASS JASS_ANYTIME_THREAD_RESULT
	--------------------------------
*/
/*!
	@brief Results from a single thread of execution in parallel search.
*/
class JASS_anytime_thread_result
	{
	public:
		std::map<std::string, JASS_anytime_result> results;		///< The results from each query (keyed on the query id)

	public:
		/*
			JASS_ANYTIME_THREAD_RESULT::JASS_ANYTIME_THREAD_RESULT()
			--------------------------------------------------------
		*/
		JASS_anytime_thread_result()
			{
			/* Nothing */
			}

		/*
			JASS_ANYTIME_THREAD_RESULT::PUSH_BACK()
			---------------------------------------
		*/
		/*!
         @param query_id [in] The query ID
         @param query [in] The query
         @param results_list [in] The results list (normally in TREC format)
         @param postings_processed [in] The numvber of postings processed (that is, <docid, impact> pairs)
         @param search_time_in_ns [in] The time it took to resolve the query
		*/
		void push_back(const std::string &query_id, const std::string &query, const std::string &results_list, size_t postings_processed, size_t search_time_in_ns)
			{
			results[query_id] = JASS_anytime_result(query_id, query, results_list, postings_processed, search_time_in_ns);
			}

		/*
			JASS_ANYTIME_THREAD_RESULT::BEGIN()
			-----------------------------------
		*/
		std::map<std::string, JASS_anytime_result>::iterator begin()
			{
			return results.begin();
			}

		/*
			JASS_ANYTIME_THREAD_RESULT::END()
			---------------------------------
		*/
		std::map<std::string, JASS_anytime_result>::iterator end()
			{
			return results.end();
			}
	};
