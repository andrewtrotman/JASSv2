/*
	QUERY.H
	-------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query.  Subclassed in order to get add_rsv()
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <immintrin.h>

#include "top_k_qsort.h"
#include "parser_query.h"
#include "query_term_list.h"
#include "allocator_memory.h"

namespace JASS
	{
	/*
		CLASS QUERY
		-----------
	*/
	/*!
		@brief Everything necessary to process a query is encapsulated in an object of this type
		@tparam ACCUMULATOR_TYPE The value-type for an accumulator (normally uint16_t or double).
		@tparam MAX_DOCUMENTS The maximum number of documents that are ever going to exist in this collection
		@tparam MAX_TOP_K The maximum top-k documents that are going to be asked for
	*/
	template <typename ACCUMULATOR_TYPE, size_t MAX_DOCUMENTS, size_t MAX_TOP_K>
	class query
		{
		public:
			/*
				CLASS QUERY::ADD_RSV_COMPARE
				----------------------------
			*/
			/*!
				@brief Functor that does the comparison (looking for a < b, if a == b then pointer compare).
			*/
			class add_rsv_compare
				{
				public:
					/*
						QUERY::ADD_RSV_COMPARE::OPERATOR()
						----------------------------------
					*/
					/*!
						@brief Compare value then pointer for greater, equal, less then.
						@param a [in] first pointer.
						@param b [in] second pointer
						@return 1 if greater, 0 if equal, -1 is less
					*/
					forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const
						{
						/*
							The most likely case is that the value at a is less than the value at b so do that check first.
						*/
						return *a < *b ? -1 : *a > *b ? 1 : a < b ? -1 : a == b ? 0 : 1;
						}
				};
			/*
				CLASS QUERY::SORT_RSV_COMPARE
				-----------------------------
			*/
			/*!
				@brief Functor that does the comparison (looking for a > b, if a == b then pointer compare).
			*/
			class sort_rsv_compare
				{
				public:
					/*
						QUERY::SORT_RSV_COMPARE::OPERATOR()
						----------------------------------
					*/
					/*!
						@brief Compare value then pointer for greater, equal, less then.
						@param a [in] first pointer.
						@param b [in] second pointer
						@return 1 if greater, 0 if equal, -1 is less
					*/
					forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const
						{
						/*
							The most likely case is that the value at a is less than the value at b so do that check first.
						*/
						return *a < *b ? 1 : *a > *b ? -1 : a < b ? 1 : a == b ? 0 : -1;
						}
				};

		protected:
			allocator_pool memory;														///< All memory allocation happens in this "arena"
			ACCUMULATOR_TYPE impact;													///< The impact score to be added on a call to push_back()

			parser_query parser;															///< Parser responsible for converting text into a parsed query
			query_term_list *parsed_query;											///< The parsed query
			const std::vector<std::string> &primary_keys;						///< A vector of strings, each the primary key for the document with an id equal to the vector index

		public:
			size_t top_k;																	///< The number of results to track.
			add_rsv_compare cmp;															///< Comparison during addition (used to order low to high a min heap)
			sort_rsv_compare final_sort_cmp;											///< Comparison after search (used to order high to low)

		public:
			/*
				QUERY::QUERY()
				--------------
			*/
			/*!
				@brief Constructor
				@param primary_keys [in] Vector of the document primary keys used to convert from internal document ids to external primary keys.
				@param documents [in] The number of documents in the collection.
				@param top_k [in]	The top-k documents to return from the query once executed.
			*/
			query(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10) :
				impact(0),
				parser(memory),
				parsed_query(nullptr),
				primary_keys(primary_keys),
				top_k(top_k)
				{
				rewind();
				}

			/*
				QUERY::~QUERY()
				---------------
			*/
			/*!
				@brief Destructor
			*/
			~query()
				{
				delete parsed_query;
				}

			/*
				QUERY::PARSE()
				--------------
			*/
			/*!
				@brief Take the given query and parse it.
				@tparam STRING_TYPE Either a std::string or JASS::string.
				@param query [in] The query to parse.
			*/
			template <typename STRING_TYPE>
			void parse(const STRING_TYPE &query)
				{
				parser.parse(*parsed_query, query);
				}

			/*
				QUERY::TERMS()
				--------------
			*/
			/*!
				@brief Return a reference to the parsed query.
				@return A reference to the parsed query.
			*/
			query_term_list &terms(void)
				{
				return *parsed_query;
				}

			/*
				QUERY::REWIND()
				---------------
			*/
			/*!
				@brief Clear this object after use and ready for re-use
			*/
			void rewind(void)
				{
				delete parsed_query;
				parsed_query = new query_term_list(memory);
				}

			/*
				QUERY::SET_SCORE()
				------------------
			*/
			/*!
				@brief Set the impact score to use in a push_back().
				@param score [in] The impact score to be added to accumulators.
			*/
			forceinline void set_score(ACCUMULATOR_TYPE score)
				{
				this->impact = score;
				}

			/*
				QUERY::PUSH_BACK()
				------------------
			*/
			/*!
				@brief Add the impact score to a bunch of accumulators
				@param document_ids [in] The document IDs that the impact should be added to.
				@details The first valid document id is 1, any calls with a document id of 0 will add to
				the accumulator for document id 0, but since that is a non-existant document, the value is later
				ignored.  So it IS safe to pad documet_ids with 0s.
			*/
			forceinline void push_back(__m256i document_ids)
				{
				uint32_t each[8];
				__m256i *into = (__m256i *)each;

				_mm256_storeu_si256(into, document_ids);
				add_rsv(each[0], impact);
				add_rsv(each[1], impact);
				add_rsv(each[2], impact);
				add_rsv(each[3], impact);
				add_rsv(each[4], impact);
				add_rsv(each[5], impact);
				add_rsv(each[6], impact);
				add_rsv(each[7], impact);
				}
		};
	}
