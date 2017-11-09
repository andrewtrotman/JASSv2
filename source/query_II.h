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

#include "heap.h"
#include "top_k_qsort.h"
#include "parser_query.h"
#include "query_term_list.h"
#include "allocator_memory.h"

namespace JASS
	{
	/*
		CLASS QUERY_II
		--------------
	*/
	/*!
		@brief Everything necessary to process a query is encapsulated in an object of this type
		@tparam ACCUMULATOR_TYPE The value-type for an accumulator (normally uint16_t or double).
		@tparam MAX_DOCUMENTS The maximum number of documents that are ever going to exist in this collection
		@tparam MAX_TOP_K The maximum top-k documents that are going to be asked for
	*/
	template <typename ACCUMULATOR_TYPE, size_t MAX_DOCUMENTS, size_t MAX_TOP_K>
	class query_II
		{
		public:
			/*
				CLASS QUERY::ADD_RSV_COMPARE
				----------------------------
			*/
			/*!
				@brief Functor that does the comparison (looking for a > b, if a > b then pointer compare).
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
					forceinline int operator() (ACCUMULATOR_TYPE *a, ACCUMULATOR_TYPE *b) const { return *a > *b ? 1 : *a < *b ? -1 : a - b; }
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
					const query_II<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> &parent;			///< The query object that this is iterating over
					size_t where;																				///< Where in the results list we are

				public:
					/*
						QUERY::ITERATOR::ITERATOR()
						---------------------------
					*/
					/*!
						@brief Constructor
						@param at [in] An iterator over the heap
						@param parent [in] The object we are iterating over
						@param where [in] Where in the results list this iterator starts
					*/
					iterator(const query_II<ACCUMULATOR_TYPE, MAX_DOCUMENTS, MAX_TOP_K> &parent, size_t where) :
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
						@param other [in] The iterator object to compare to.
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
						size_t id = parent.accumulator_pointers[where] - parent.accumulators;
						return docid_rsv_pair(id, parent.primary_keys[id], parent.accumulators[id]);
						}
					};


		private:
			allocator_pool memory;									///< All memory allocation happens in this "arena"

			ACCUMULATOR_TYPE *accumulator_pointers[MAX_TOP_K];
			size_t accumulators_shift;
			size_t accumulators_width;
			size_t accumulators_height;
			ACCUMULATOR_TYPE accumulators[MAX_DOCUMENTS];
			uint8_t clean_flags[MAX_DOCUMENTS];
			size_t results_list_length;

			heap<ACCUMULATOR_TYPE *, add_rsv_compare> top_results;

			parser_query parser;										///< Parser responsible for converting text into a parsed query
			query_term_list *parsed_query;							///< The parsed query
			const std::vector<std::string> &primary_keys;	///< A vector of strings, each the primary key for the document with an id equal to the vector index
			size_t top_k;												///< The number of results to track.

			add_rsv_compare cmp;

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
			query_II(const std::vector<std::string> &primary_keys, size_t documents = 1024, size_t top_k = 10) :
				accumulators_shift(log2(sqrt((double)documents))),
  				accumulators_width(1 << accumulators_shift),
				accumulators_height((documents + accumulators_width) / accumulators_width),
				top_results(*accumulator_pointers, top_k),
				parser(memory),
				parsed_query(new query_term_list(memory)),
				primary_keys(primary_keys),
				top_k(top_k)
				{
				memset(clean_flags, 0, accumulators_height);
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
				return iterator(*this, 0);
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
				return iterator(*this, results_list_length);
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
				results_list_length = 0;
		      memset(clean_flags, 0, accumulators_height);
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
				top_k_qsort(accumulator_pointers, results_list_length, top_k);
				}

			/*
				QUERY::ADD_RSV()
				----------------
			*/
			/*!
				@brief Add weight to the rsv for document docuument_id
				@param document_id [in] which document to increment
				@param weight [in] the amount of weight to add
			*/
			forceinline void add_rsv(size_t docid, ACCUMULATOR_TYPE score)
				{
				ACCUMULATOR_TYPE old_value;
				ACCUMULATOR_TYPE *which = accumulators + docid;

				/*
					Make sure the accumulator exists
				*/
				if (clean_flags[docid >> accumulators_shift] == 0)
					{
					clean_flags[docid >> accumulators_shift] = 1;
					memset(accumulators + (accumulators_width * (docid >> accumulators_shift)), 0, accumulators_width * sizeof(ACCUMULATOR_TYPE));
					}

				/*
					Maintain a heap
				*/
				if (results_list_length < top_k)
					{
					/*
						We haven't got enough to worry about the heap yet, so just plonk it in
					*/
					old_value = *which;
					*which += score;

					if (old_value == 0)
						accumulator_pointers[results_list_length++] = which;

					if (results_list_length == top_k)
						top_results.make_heap();
					}
				else if (cmp(which, accumulator_pointers[0]) >= 0)
					{
					/*
						We were already in the heap, so update
					*/
					*which += score;
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
				query_II<uint16_t, 1024, 10> query_object(keys, 1024, 2);
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
