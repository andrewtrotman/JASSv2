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
#include "accumulator_2d.h"


namespace JASS
	{
	/*
		CLASS QUERY
		-----------
	*/
	/*!
		@brief Everything necessary to process a query is encapsulated in an object of this type
	*/
	class query
		{
		private:
			typedef typename top_k_heap<const pointer_box<uint16_t>>::iterator heap_iterator;			///< The heap object's begin() returns objects of this type

		private:
			accumulator_2d<uint16_t> accumulators;						///< The array of accumulators
			top_k_heap<pointer_box<uint16_t>> heap;					///< The top-k heap containing the best results so far

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
					size_t document_id;				///< The document identifier
					uint16_t rsv;						///< The rsv (Retrieval Status Value) relevance score
					
				public:
					/*
						QUERYL::DOCID_RSV_PAIR::DOCID_RSV_PAIR()
						----------------------------------------
					*/
					/*!
						@Brief Constructor.
						@param document_id [in] The document Identifier.
						@param rsv [in] The rsv (Retrieval Status Value) relevance score.
					*/
					docid_rsv_pair(size_t document_id, uint16_t rsv) :
						document_id(document_id),
						rsv(rsv)
						{
						/* Nothing */
						}
				};
			/*
				CLASS QUERYL::ITERATOR
				----------------------
			*/
			/*!
				@brief Iterate over the top-l
			*/
			class iterator
				{
				private:
					heap_iterator at;									///< This iterator's current location
					uint16_t *accumulator_base;					///< The accumulators array used to compute document ids through pointer subtraction
					
				public:
					/*
						QUERY::ITERATOR::ITERATOR()
						---------------------------
					*/
					/*!
						@brief Constructor
						@param at [in] An iterator over the heap
						@param accumulator_base [in] The base address of the accumulators used to compute document ids through pointer subtraction
					*/
					iterator(heap_iterator &at, uint16_t *accumulator_base) :
						at(at),
						accumulator_base(accumulator_base)
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
						return docid_rsv_pair(at->pointer() - accumulator_base, *at->pointer());
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
			*/
			query() :
				accumulators(1024),
				heap(10)
				{
				/* Nothing */
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
					heap.push_back(JASS::pointer_box<uint16_t>(&accumulators[document_id]));
					}
				else if (accumulators[document_id] < *heap.front())
					{
					/*
						we weren't in the heap, but we might become so
					*/
					if (accumulators[document_id] += weight > *heap.front())
						{
						accumulators[document_id] += weight;
						heap.push_back(JASS::pointer_box<uint16_t>(&accumulators[document_id]));
						}
					}
				else
					{
					/*
						We're already in the heap but we might have moved spots
					*/
					accumulators[document_id] += weight;
					heap.promote(JASS::pointer_box<uint16_t>(&accumulators[document_id]));
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
				auto heap_looper = heap.begin();
				return iterator (heap_looper, &accumulators[0]);
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
				auto heap_looper = heap.end();
				return iterator(heap_looper, &accumulators[0]);
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
				query query_object;
				std::ostringstream string;

				query_object.add_rsv(10, 10);
				query_object.add_rsv(20, 20);

				for (const auto &rsv : query_object)
					string << "<" << rsv.document_id << "," << rsv.rsv << ">";

				JASS_assert(string.str() == "<20,20><10,10>");
				puts("query::PASS");
				}
		};
	}
