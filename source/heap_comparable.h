/*
	HEAP_COMPARABLE.H
	-----------------
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

#include "heap.h"

namespace JASS
	{
	namespace privates
		{
		class compare
			{
			public:
				int operator()(int a, int b)
					{
					return a < b ? -1 : a == b ? 0 : 1;
					}
			};
		}
	/*
		CLASS HEAP_COMPARABLE
		---------------------
	*/
	/*!
		@brief A bare-bones implementaiton of a min-heap over an array passed by the caller. TYPE mustimplement <, <=, >
		@tparam TYPE The type to build the heap over (normall a pointer type).
	*/
	template <typename TYPE>
	class heap_comparable : public heap<TYPE, privates::compare>
		{
		private:
			/*
				HEAP_COMPARABLE::HEAPIFY()
				--------------------------
			*/
			/*!
				@brief Recurively build the heap.
				@param position [in] where, in the array, to build the heap
			*/
			void heapify(size_t position)
				{
				size_t smallest;

				size_t left = this->left_of(position);
				size_t right = this->right_of(position);

				if ((left < this->size) && (this->array[left] < this->array[position]))
					smallest = left;
				else
					smallest = position;

				if ((right < this->size) && (this->array[right] < this->array[smallest]))
					smallest = right;

				if (smallest != position)
					{
					std::swap(this->array[position], this->array[smallest]);
					heapify(smallest);
					}
				}

			/*
				HEAP_COMPARABLE::INSERT_FROM()
				------------------------------
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
				while (position < this->size)
					{
					size_t left = this->left_of(position);
					size_t right = this->right_of(position);

					// check array out of bound, it's also the stopping condition
					if (left < this->size && right < this->size)
						{
						if ((key <= this->array[left]) && (key <= this->array[right]))
							break;			// we're smaller then the left and the right so we're done
						else if (this->array[left] < this->array[right])
							{
							this->array[position] = this->array[left];
							position = left;
							}
						else
							{
							this->array[position] = this->array[right];
							position = right;
							}
						}
					else if (left < this->size)			// and right > size (because this is an else)
						{
						if (key > this->array[left])
							{
							this->array[position] = this->array[left];
							position = left;
							}
						else
							break;
						}
					else
						break;			// both right and left exceed end of array
					}

				this->array[position] = key;
				}

		public:
			/*
				HEAP_COMPARABLE::HEAP_COMPARABLE()
				----------------------------------
			*/
			/*!
				@brief Constructor
				@param array [in] The array to maintain the heap over
				@param size [in] The maximum number of elements in the array (the size of the heap)
			*/
			heap_comparable(TYPE *array, size_t size = 0) :
				heap<TYPE, privates::compare>(array, size)
				{
				/* Nothing */
				}

			/*
				HEAP_COMPARABLE::UNITTEST()
				---------------------------
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

					std::vector<int> buffer(5);
					heap_comparable heap(&buffer[0], buffer.size());

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
				puts("heap_comparable::PASSED");
				}
		};
	}

