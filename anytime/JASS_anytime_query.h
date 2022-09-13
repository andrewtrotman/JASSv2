/*
	JASS_ANYTIME_QUERY.H
	--------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A query object - used to facilitate parallel query resolution
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include <atomic>
#include <string>
#include <vector>

/*
	CLASS JASS_ANYTIME_QUERY
	------------------------
*/
/*!
	@brief A query within the anytime parallel search system.
*/
class JASS_anytime_query
	{
	public:
		std::atomic<uint8_t> taken;				///< Has this query been "taken" by a thread and processed
		std::string query;							///< The query.

	public:
		/*
			JASS_ANYTIME_QUERY::JASS_ANYTIME_QUERY()
			----------------------------------------
		*/
		/*!
			@brief Constructor
		*/
		JASS_anytime_query() = delete;

		/*
			JASS_ANYTIME_QUERY::JASS_ANYTIME_QUERY()
			----------------------------------------
		*/
		/*!
			@brief Constructor
			@param query [in] this node represents this query (which is copied)
		*/
		JASS_anytime_query(const std::string &query) :
			taken(false),
			query(query)
				{
				/* Nothing */
				}

		/*
			JASS_ANYTIME_QUERY::JASS_ANYTIME_QUERY()
			----------------------------------------
		*/
		/*!
			@brief Move Constructor
			@param original [in] the node to copy
		*/
		JASS_anytime_query(JASS_anytime_query &&original) :
			taken(original.taken.load()),
			query(original.query)
				{
				/*
					Invalidate the original object.
				*/
				original.query = "";
				original.taken = true;
				}

		/*
			JASS_ANYTIME_QUERY::GET_NEXT_QUERY()
			------------------------------------
		*/
		/*!
			@brief Given a list of queries, return the next un-taken query
			@param list [in] The list to search in
			@param starging_from [in/out] Where to start searching (should initially be 0, updated to the current node)
			@return A copy of the string representing the query
		*/
		static std::string get_next_query(std::vector<JASS_anytime_query>&list, size_t &starting_from)
			{
			auto total_queries = list.size();
			while (starting_from < total_queries)
				{
				while (!list[starting_from].taken)					/// remember that compare_exchange_strong might unexpectedly fail so we loop over the node we're looking at
					{
					uint8_t expected = false;
					if (list[starting_from].taken.compare_exchange_strong(expected, true))
						return list[starting_from].query;
					}
				starting_from++;
				}

			return std::string();
			}
	};
