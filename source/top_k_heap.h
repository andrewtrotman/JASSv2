/*
	TOP_K_HEAP.H
	------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Implementation of a top-k keeper uwing a heap
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <array>
#include <random>
#include <algorithm>

namespace JASS
	{
	template <typename HEAP_ELEMENT>
	class top_k_heap
		{
		private:
			std::vector<HEAP_ELEMENT> members;
			size_t maximum_members;
			size_t current_members;
			size_t total_insertions;

		public:
			top_k_heap(size_t maximum_members) :
				maximum_members(maximum_members),
				current_members(0),
				total_insertions(0)
				{
				}

			void push_back(const HEAP_ELEMENT &element)
				{
				total_insertions++;

				current_members++;

				if (current_members ==	maximum_members)
					{
					members.push_back(element);
					make_heap(members.begin(), members.end());
					}
				else if (current_members > maximum_members)
					{
					if (element < members.front())
						{
						pop_heap(members.begin(), members.end());
						members.pop_back();
						members.push_back(element);
						push_heap(members.begin(), members.end());
						}
					}
				else
					members.push_back(element);
				}

			void text_render(void) const
				{
				for (size_t element = 0; element < maximum_members; element++)
					std::cout << members[element] << " ";
				std::cout << "\n" << std::ends;
				}

			static void unittest(void)
				{
				int sequence[] =  {0,1,2,3,4,5,6,7,8,9};

				for (auto iteration = 0; iteration < 5; iteration++)
					{
					std::random_shuffle(&sequence[0], &sequence[10]);
					top_k_heap<int> heap(5);

					for (const auto &element : sequence)
						heap.push_back(element);

					heap.text_render();
					}
				puts("top_k_heap::PASS");
				}
		};
	}
