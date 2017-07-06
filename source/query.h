
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
			accumulator_2d<uint16_t> accumulators;
			top_k_heap<pointer_box<uint16_t>> heap;
		
		public:
			query() :
				accumulators(1024),
				heap(10)
				{
				/* Nothing */
				}

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

		void text_render(void)
			{
			heap.sort();
			
			for (const auto &element : heap)
				std::cout << element.pointer() - &accumulators[0] << ":" << *element << std::endl;
			}
		};
	}
