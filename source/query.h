/*
	QUERY.H
	-------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Everything necessary to process a query
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

//#define DONT_INLINE_ADD_RSV
//#define JASSv1_ADD_RSV

#include "heap.h"
#include "top_k_qsort.h"
#include "parser_query.h"
#include "accumulator_2d.h"
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

		public:
			/*
				CLASS QUERY::ITERATOR
				----------------------
			*/
			/*!
				@brief Iterate over the top-k
			*/
			class iterator
				{
				/*
					CLASS QUERY::ITERATOR::DOCID_RSV_PAIR()
					---------------------------------------
				*/
				/*!
					@brief Literally a <document_id, rsv> ordered pair.
				*/
				class docid_rsv_pair
					{
					public:
						size_t document_id;							///< The document identifier
						const std::string &primary_key;			///< The external identifier of the document (the primary key)
						ACCUMULATOR_TYPE rsv;						///< The rsv (Retrieval Status Value) relevance score

					public:
						/*
							QUERY::ITERATOR::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
							-------------------------------------------------
						*/
						/*!
							@brief Constructor.
							@param document_id [in] The document Identifier.
							@param key [in] The external identifier of the document (the primary key).
							@param rsv [in] The rsv (Retrieval Status Value) relevance score.
						*/
						docid_rsv_pair(size_t document_id, const std::string &key, ACCUMULATOR_TYPE rsv) :
							document_id(document_id),
							primary_key(key),
							rsv(rsv)
							{
							/* Nothing */
							}
					};

				public:
					query<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> &parent;			///< The query object that this is iterating over
					size_t where;																				///< Where in the results list we are

				public:
					/*
						QUERY::ITERATOR::ITERATOR()
						---------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(query<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> &parent, size_t where) :
						parent(parent),
						where(where)
						{
						/* Nothing */
						}

					/*
						QUERY::ITERATOR::OPERATOR!=()
						-----------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param with [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/
					bool operator!=(const iterator &with) const
						{
						return with.where != where;
						}

					/*
						QUERY::ITERATOR::OPERATOR++()
						-----------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					iterator &operator++(void)
						{
						where++;
						return *this;
						}

					/*
						QUERY::ITERATOR::OPERATOR*()
						----------------------------
					*/
					/*!
						@brief Return a reference to the <document_id,rsv> pair at the current location.
						@details This method uses ppointer arithmatic to work out the document id from a pointer to the rsv, and
						having done so it constructs an orderer pair <document_id,rsv> to return to the caller.
						@return The current object.
					*/
					docid_rsv_pair operator*()
						{
						size_t id = parent.accumulator_pointers[where] - &parent.accumulators[0];
						return docid_rsv_pair(id, parent.primary_keys[id], parent.accumulators[id]);
						}
					};

		private:
			ACCUMULATOR_TYPE zero;														///< Constant zero used for pointer dereferenced comparisons
			allocator_pool memory;														///< All memory allocation happens in this "arena"
			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];					///< Array of pointers to the top k accumulators
			accumulator_2d<ACCUMULATOR_TYPE, MAX_DOCUMENTS> accumulators;	///< The accumulators, one per document in the collection
			size_t needed_for_top_k;													///< The number of results we still need in order to fill the top-k
			heap<ACCUMULATOR_TYPE *, add_rsv_compare> top_results;			///< Heap containing the top-k results

			parser_query parser;															///< Parser responsible for converting text into a parsed query
			query_term_list *parsed_query;											///< The parsed query
			const std::vector<std::string> &primary_keys;						///< A vector of strings, each the primary key for the document with an id equal to the vector index
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
				zero(0),
				accumulators(documents),
				top_results(*accumulator_pointers, top_k),
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
				QUERY::BEGIN()
				--------------
			*/
			/*!
				@brief Return an iterator pointing to start of the top-k
				@return Iterator pointing to start of the top-k
			*/
			auto begin(void)
				{
				sort();
				return iterator(*this, needed_for_top_k);
				}

			/*
				QUERY::END()
				------------
			*/
			/*!
				@brief Return an iterator pointing to end of the top-k
				@return Iterator pointing to the end of the top-k
			*/
			auto end(void)
				{
				return iterator(*this, top_k);
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
				accumulator_pointers[0] = &zero;
				accumulators.rewind();
				needed_for_top_k = top_k;
				delete parsed_query;
				parsed_query = new query_term_list(memory);
				}

			/*
				QUERY::SORT()
				-------------
			*/
			/*!
				@brief sort this resuls list before iteration over it.
			*/
			void sort(void)
				{
				top_k_qsort::sort(accumulator_pointers + needed_for_top_k, top_k - needed_for_top_k, top_k, final_sort_cmp);
				}

#ifdef JASSv1_ADD_RSV
			/*
				ADD_RSV()
				---------
			*/
#ifndef DONT_INLINE_ADD_RSV
			forceinline
#endif
			void add_rsv(size_t document_id, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE old_value;
				ACCUMULATOR_TYPE *which = &accumulators[document_id];			// this will also make sure the accumulator exists
				add_rsv_compare cmp;

				/*
					Maintain the heap
				*/
				if (needed_for_top_k > 0)
					{
					/*
						We haven't got enough to worry about the heap yet, so just plonk it in
					*/
					old_value = *which;
					*which += score;

					if (old_value == 0)
						accumulator_pointers[--needed_for_top_k] = which;

					if (needed_for_top_k == 0)
						top_results.make_heap();
					}
				else if (cmp(which, accumulator_pointers[0]) >= 0)
					{
					/*
						We were already in the heap, so update
					*/
					*which +=score;
					top_results.promote(which);
					}
				else
					{
					/*
						We weren't in the heap, but we could get put there
					*/
					*which += score;
					if (cmp(which, accumulator_pointers[0]) > 0)
						top_results.push_back(which);
					}
				}
#else
			/*
				QUERY::ADD_RSV()
				----------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param score [in] the amount of weight to add
			*/
#ifndef DONT_INLINE_ADD_RSV
			forceinline
#endif
			void add_rsv(size_t document_id, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE *which = &accumulators[document_id];			// This will create the accumulator if it doesn't already exist.

				/*
					By doing the add first its possible to reduce the "usual" path through the code to a single comparison.  The JASS v1 "usual" path took three comparisons.
				*/
				*which += score;
				if (cmp(which, accumulator_pointers[0]) >= 0)			// ==0 is the case where we're the current bottom of heap so might need to be promoted
					{
					/*
						We end up in the top-k, now to work out why.  As this is a rare occurence, we've got a little bit of time on our hands
					*/
					if (needed_for_top_k > 0)
						{
						/*
							the heap isn't full yet - so change only happens if we're a new addition (i.e. the old value was a 0)
						*/
						if (*which == score)
							{
							accumulator_pointers[--needed_for_top_k] = which;
							if (needed_for_top_k == 0)
								top_results.make_heap();
							}
						}
					else
						{
						*which -= score;
						int prior_compare = cmp(which, accumulator_pointers[0]);
						*which += score;

						if (prior_compare < 0)
							top_results.push_back(which);				// we're not in the heap so add this accumulator to the heap
						else
							top_results.promote(which);				// we're already in the heap so promote this document
						}
					}
				}
#endif
			/*
				QUERY::UNITTEST()
				-----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<std::string> keys = {"one", "two", "three", "four"};
				query<uint16_t, 1024, 10> query_object(keys, 1024, 2);
				std::ostringstream string;

				/*
					Check the rsv stuff
				*/
				query_object.add_rsv(2, 10);
				query_object.add_rsv(3, 20);
				query_object.add_rsv(2, 2);
				query_object.add_rsv(1, 1);
				query_object.add_rsv(1, 14);

				for (const auto &rsv : query_object)
					string << "<" << rsv.document_id << "," << rsv.rsv << ">";
				JASS_assert(string.str() == "<3,20><1,15>");

				/*
					Check the parser
				*/
				size_t times = 0;
				query_object.parse(std::string("one two three"));
				for (const auto &term : query_object.terms())
					{
					times++;
					if (times == 1)
						JASS_assert(term.token() == "one");
					else if (times == 2)
						JASS_assert(term.token() == "two");
					else if (times == 3)
						JASS_assert(term.token() == "three");
					}

				puts("query::PASSED");
				}
		};
	}
