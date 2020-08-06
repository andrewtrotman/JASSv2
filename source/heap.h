/*
	HEAP.H
	------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE code base where it was written by Xiang-Fei Jia and Andrew Trotman
*/
/*!
	@file
	@brief A bare-bones implementaiton of a min-heap.
	@author iang-Fei Jia and Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <random>
#include <sstream>
#include <algorithm>

#include "asserts.h"
#include "forceinline.h"

namespace JASS
	{
	/*
		CLASS HEAP
		----------
	*/
	/*!
		@brief A bare-bones implementaiton of a min-heap over an array passed by the caller.
		@tparam TYPE The type to build the heap over (normall a pointer type).
	*/
	template <typename TYPE>
	class heap
		{
		protected:
			TYPE *array;					///< The array to build the heap over
			size_t size;					///< The maximum size of the heap

		protected:
			/*
				HEAP::LEFT_OF()
				---------------
			*/
			/*!
				@brief Return the index of the left element of the heap given the current position in the heap.
				@param position [in] current position in the heap.
				@return index of left of position.
			*/
			forceinline size_t left_of(size_t position)
				{
				return (position << 1) + 1;
				}

			/*
				HEAP::RIGHT_OF()
				----------------
			*/
			/*!
				@brief Return the index of the right element of the heap given the current position in the heap.
				@param position [in] current position in the heap.
				@return index of right of position.
			*/
			forceinline size_t right_of(size_t position)
				{
				return (position << 1) + 2;
				}

			/*
				HEAP::HEAPIFY()
				---------------
			*/
			/*!
				@brief Recurively build the heap.
				@param position [in] where, in the array, to build the heap
			*/
			void heapify(size_t position)
				{
				size_t smallest;

				size_t left = left_of(position);
				size_t right = right_of(position);

				if (left < size && array[left] < array[position])
					smallest = left;
				else
					smallest = position;

				if (right < size && array[right] < array[smallest])
					smallest = right;

				if (smallest != position)
					{
					std::swap(array[position], array[smallest]);
					heapify(smallest);
					}
				}

			/*
				HEAP::INSERT_FROM()
				-------------------
			*/
			/*!
				@brief Add and element to the heap after the given position
				@param key [in] The element to add to the heap
				@param position [in] The element to start looking from.
				@details This method does not check to see if the element is already in the heap or that
				the element should be in the heap (i.e. > smallest).
			*/
			void insert_from(TYPE key, size_t position = 0)
				{
				while (position < size)
					{
					size_t left = left_of(position);
					size_t right = right_of(position);

					// check array out of bound, it's also the stopping condition
					if (left < size && right < size)
						{
						if (key <= array[left] && key <= array[right])
							break;			// we're smaller then the left and the right so we're done
						else if (array[left] < array[right])
							{
							array[position] = array[left];
							position = left;
							}
						else
							{
							array[position] = array[right];
							position = right;
							}
						}
					else if (left < size)			// and right > size (because this is an else)
						{
						if (key > array[left])
							{
							array[position] = array[left];
							position = left;
							}
						else
							break;
						}
					else
						break;			// both right and left exceed end of array
					}

				array[position] = key;
				}

		public:
			/*
				HEAP::HEAP()
				------------
			*/
			/*!
				@brief Constructor
				@param array [in] The array to maintain the heap over
				@param size [in] The maximum number of elements in the array (the size of the heap)
			*/
			heap(TYPE *array, size_t size = 0) :
				array(array),
				size(size)
				{
				/* Nothing */
				}

			/*
				HEAP::SET_TOP_K()
				-----------------
			*/
			/*!
				@brief change maximum number of elements in the array
				@param size [in] The maximum number of elements in the array (the size of the heap)
			*/
			void set_top_k(size_t size)
				{
				this->size = size;
				}

			/*
				HEAP::MAKE_HEAP()
				-----------------
			*/
			/*!
				@brief build the heap
			*/
			forceinline void make_heap(void)
				{
				for (int64_t position = size / 2 - 1; position >= 0; position--)
					heapify(position);
				}

			/*
				HEAP::PUSH_BACK()
				-----------------
			*/
			/*!
				@brief Add and element to the heap.
				@param key [in] The element to add to the heap
				@details This method does not check to see if the element is already in the heap or that
				the element should be in the heap (i.e. > smallest).
			*/
			forceinline void push_back(TYPE key)
				{
				insert_from(key, 0);
				}

			/*
				HEAP::FIND()
				------------
			*/
			/*!
				@brief Find an instance of key in the heap and return its index into array[].
				@param key [in] The key to look for.
				@return The index into array[] of an instance of key, or -1 if key cannot be found in the heap.
			*/
			int64_t find(const TYPE &key) const
				{
				size_t position;

				for (position = 0; position < size; position++)
					if (array[position] == key)
						return position;

				return -1;
				}


			/*
				HEAP::PROMOTE()
				--------------
			*/
			/*!
				@brief Key has changed its value so re-build the heap from that point onwards
				@param key [in] The element that has changed
			*/
			forceinline void promote(TYPE key, size_t position)
				{
				insert_from(key, position);
				}

			/*
				HEAP::UNITTEST()
				----------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Start with an ordered sequence of numbers
				*/
				int sequence[] =  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
				
				/*
					Seed the random number generator
				*/
				auto seed = std::chrono::system_clock::now().time_since_epoch().count();
				std::default_random_engine random((unsigned int)seed);
				
				/*
					Some relatively small number of times, we're going to shuffle the sequence then add elements to the heap
					then sort the heap then make sure we got the right answer
				*/
				for (auto iteration = 0; iteration < 5; iteration++)
					{
					/*
						Shuffle the sequence of numbers
					*/
					std::shuffle(&sequence[0], &sequence[10], random);

					/*
						Add them to a heap that keeps a top-k of less than the number of elements in the sequence
					*/
					std::vector<int> buffer(5);
					heap<int> heap(&buffer[0], buffer.size());

					for (const auto &element : sequence)
						{
						/*
							NOTE:  This code maintains the heap, it does not determine which values should be in it.
						*/
						if (element > buffer[0])
							heap.push_back(element);
						}

					/*
						Sort the contents of the heap
					*/
					std::sort(buffer.begin(), buffer.end(), std::greater<int>());
		
					/*
						Serialise and compare to the known correct result
					*/
					std::ostringstream result;
					for (const auto &value : buffer)
						result << value << " ";
					JASS_assert(result.str() == "9 8 7 6 5 ");
					}

				/*
					We passed!
				*/
				puts("heap::PASSED");
				}
		};
	}

