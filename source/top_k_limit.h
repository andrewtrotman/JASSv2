/*
	TOP_K_LIMIT.H
	-------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Management of top-k rsv limiting.
	@author Andrew Trotman
	@copyright 2021 Andrew Trotman
*/
#pragma once

#include <string>
#include <unordered_map>

namespace JASS
	{
	/*
		CLASS TOP_K_LIMIT
		-----------------
	*/
	/*!
		@brief Management of limits for the top-k management, essentially a lookup table of expected rsv values
	*/
	class top_k_limit
		{
		protected:
			std::unordered_map<std::string, uint32_t> table;			///<  The rsv scores are stored in this lookup table

		public:
			/*
				TOP_K_LIMIT::TOP_K_LIMIT()
				--------------------------
			*/
			/*!
				@brief Constructor
				@details Make an empty object
			*/
			top_k_limit()
				{
				/* Nothing */
				}

			/*
				TOP_K_LIMIT::TOP_K_LIMIT()
				--------------------------
			*/
			/*!
				@brief Constructor
				@details The format of the file is one line per query, each line being <id> <rsv>, where <id> is the query id (a string)
				and <rsv> is the minimum rsv necessary for entering the heap.  Any documents scoring less than rsv will not be
				allowed to enter the heap and so will not be in the results list for the given query.
				@param filename [in] the name of the file containing the rsv limit for each query
			*/
			top_k_limit(std::string &filename);

			/*
				TOP_K_LIMIT::~TOP_K_LIMIT()
				---------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~top_k_limit()
				{
				/* Nothing */
				}

			/*
				TOP_K_LIMIT::OPERATOR[]()
				-------------------------
			*/
			uint32_t operator[](std::string query_id)
				{
				return table[query_id];
				}

			/*
				TOP_K_LIMIT::EMPTY()
				--------------------
			*/
			bool empty(void)
				{
				return table.empty();
				}
		};
	}
