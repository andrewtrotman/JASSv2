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
			typedef typename top_k_heap<pointer_box<uint16_t>>::iterator heap_iterator;

		private:
			accumulator_2d<uint16_t> accumulators;					///< The array of accumulators
			top_k_heap<pointer_box<uint16_t>> heap;					///< The top-k heap containing the best results so far

		public:
			class pair
				{
				public:
					size_t document_id;
					uint16_t rsv;
					
				public:
					pair(size_t document_id, uint16_t rsv) :
						document_id(document_id),
						rsv(rsv)
						{
						/* Nothing */
						}
				};
			/*
				CLASS ITERATOR
				--------------
			*/
			class iterator
				{
				private:
					heap_iterator &at;
					
				public:
					/*
						QUERY::ITERATOR::ITERATOR()
						---------------------------
					*/
					iterator(heap_iterator &at) :
						at(at)
						{
						/* Nothing */
						}
					/*
						QUERY::ITERATOR::OPERATOR!=()
						-----------------------------
					*/
					bool operator!=(const heap_iterator &other) const
						{
						return at != other;
						}
 
					/*
						QUERY::ITERATOR::OPERATOR*()
						----------------------------
					*/
					pair operator*() const
						{
						return pair(6,6);
						}
 
					/*
						QUERY::ITERATOR::OPERATOR++()
						-----------------------------
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
					if (accumulators[document_id] += weight > *heap.front());
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
				return iterator{heap.begin()};
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
				return iterator(heap.end());
				}
			
			
			
			void text_render(void)
				{
				heap.sort();
				
				for (const auto &element : *this)
					std::cout << element.pointer() - &accumulators[0] << ":" << *element << std::endl;
				}
		};
	}
