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

#include <iostream>

#include "top_k_heap.h"
#include "pointer_box.h"
#include "parser_query.h"
#include "accumulator_2d.h"

namespace JASS
	{
	/*
		CLASS QUERY
		-----------
	*/
	/*!
		@brief Everything necessary to process a query is encapsulated in an object of this type
		@tparam ACCUMULATOR_TYPE The value-type for an accumulator (normally uint16_t or double).
	*/
	template <typename ACCUMULATOR_TYPE>
	class query
		{
		private:
			typedef typename top_k_heap<pointer_box<ACCUMULATOR_TYPE>>::iterator heap_iterator;			///< The heap object's begin() returns objects of this type

		private:
			allocator_pool memory;									///< All memory allocation happens in this "arena"
			parser_query parser;										///< Parser responsible for converting text into a parsed query
			query_term_list parsed_query;							///< The parsed query
			accumulator_2d<ACCUMULATOR_TYPE> accumulators;				///< The array of accumulators
			top_k_heap<pointer_box<ACCUMULATOR_TYPE>> heap;			///< The top-k heap containing the best results so far
			const std::vector<std::string> &primary_keys;	///< A vector of strings, each the primary key for the document with an id equal to the vector index
		

		public:
			/*
				CLASS QUERY::DOCID_RSV_PAIR()
				-----------------------------
			*/
			/*!
				@brief Literally a <document_id, rsv> ordered pair.
			*/
			class docid_rsv_pair
				{
				public:
					size_t document_id;					///< The document identifier
					const std::string &primary_key;			///< The external identifier of the document (the primary key)
					uint16_t rsv;							///< The rsv (Retrieval Status Value) relevance score
					
				public:
					/*
						QUERY::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
						---------------------------------------
					*/
					/*!
						@brief Constructor.
						@param document_id [in] The document Identifier.
						@param key [in] The external identifier of the document (the primary key).
						@param rsv [in] The rsv (Retrieval Status Value) relevance score.
					*/
					docid_rsv_pair(size_t document_id, const std::string &key, uint16_t rsv) :
						document_id(document_id),
						primary_key(key),
						rsv(rsv)
						{
						/* Nothing */
						}
				};
			/*
				CLASS QUERY::ITERATOR
				----------------------
			*/
			/*!
				@brief Iterate over the top-l
			*/
			class iterator
				{
				private:
					heap_iterator at;											///< This iterator's current location
					uint16_t *accumulator_base;							///< The accumulators array used to compute document ids through pointer subtraction
					const std::vector<std::string> &primary_keys;		///< The array of primary keys used to resolve document ids into external document identifiers.
					
				public:
					/*
						QUERY::ITERATOR::ITERATOR()
						---------------------------
					*/
					/*!
						@brief Constructor
						@param at [in] An iterator over the heap
						@param accumulator_base [in] The base address of the accumulators used to compute document ids through pointer subtraction
						@param primary_keys [in] The vector of primary keys
					*/
					iterator(heap_iterator at, uint16_t *accumulator_base, const std::vector<std::string> &primary_keys) :
						at(at),
						accumulator_base(accumulator_base),
						primary_keys(primary_keys)
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
					bool operator!=(const iterator &other) const
						{
						return at != other.at;
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
					docid_rsv_pair operator*() const
						{
						return docid_rsv_pair(at->pointer() - accumulator_base, primary_keys[at->pointer() - accumulator_base], *at->pointer());
						}
 
					/*
						QUERY::ITERATOR::OPERATOR++()
						-----------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					const iterator &operator++()
						{
						++at;
						return *this;
						}
				};
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
				parser(memory),
				parsed_query(memory),
				accumulators(documents, memory),
				heap(top_k, memory),
				primary_keys(primary_keys)
				{
				/* Nothing */
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
				parser.parse(parsed_query, query);
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
				return parsed_query;
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
			void add_rsv(size_t document_id, uint16_t weight)
				{
				if (accumulators[document_id] == 0)
					{
					/*
						If we're not in the heap then put is there if need-be
					*/
					accumulators[document_id] += weight;
					heap.push_back(&accumulators[document_id]);
					}
				else if (accumulators[document_id] < *heap.front())
					{
					/*
						we weren't in the heap, but we might become so
					*/
					if ((accumulators[document_id] += weight) > *heap.front())
						heap.push_back(&accumulators[document_id]);
					}
				else
					{
					/*
						We're already in the heap but we might have moved spots
					*/
					accumulators[document_id] += weight;
					heap.promote(&accumulators[document_id]);
					}
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
				heap.sort();
				heap_iterator it = heap.begin();
				return iterator(it, &accumulators[0], primary_keys);
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
				heap_iterator it = heap.end();
				return iterator(it, &accumulators[0], primary_keys);
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
				query<uint16_t> query_object(keys, 1024, 2);
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
