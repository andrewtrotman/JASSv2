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
	/*
		CLASS HEAP_COMPARABLE
		---------------------
	*/
	/*!
		@brief A bare-bones implementaiton of a min-heap over an array passed by the caller. TYPE must implement <, <=, >
		@tparam TYPE The type to build the heap over (normall a pointer type).
	*/
	template <typename TYPE>
	class heap_comparable
		{
		private:
			TYPE *array;					///< The array to build the heap over
			size_t size;					///< The maximum size of the heap

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

				size_t left = left_of(position);
				size_t right = right_of(position);

				if ((left < size) && (array[left] < array[position]))
					smallest = left;
				else
					smallest = position;

				if ((right < size) && (array[right] < array[smallest]))
					smallest = right;

				if (smallest != position)
					{
					std::swap(array[position], array[smallest]);
					heapify(smallest);
					}
				}
#ifdef NEVER
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
				while (position < size)
					{
					size_t left = left_of(position);
					size_t right = right_of(position);

					// check array out of bound, it's also the stopping condition
					if (left < size && right < size)
						{
						if ((key <= array[left]) && (key <= array[right]))
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
					else if (left < size)			// and right >= size (because this is an else)
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
#endif
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
				array(array),
				size(size)
				{
				/* Nothing */
				}

			/*
				HEAP_COMPARABLE::PROMOTE()
				--------------------------
			*/
			/*!
				@brief Key has changed its value so re-build the heap from that point onwards (assumes keys are unique)
				@param old_key [in] The old value of the key
				@param new_key [in] The new value of the key
			*/
			forceinline void promote(TYPE old_key, TYPE new_key)
				{
				size_t position;

				for (position = 0; position < size; position++)
					if (array[position] == old_key)
						break;

if (position >= 1000)
	{
	for (size_t position = 0; position < size; position++)
		std::cout << "    HEAP: rsv:" << (array[position] >> 32) << " doc:" << (array[position] & 0xFFFF'FFFF) << "\n";

	int x = 0;
	}
array[position] = new_key;
make_heap();
//				insert_from(new_key, position);

				}

			/*
				HEAP_COMPARABLE::SET_TOP_K()
				----------------------------
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
				HEAP_COMPARABLE::MAKE_HEAP()
				----------------------------
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
				HEAP_COMPARABLE::PUSH_BACK()
				----------------------------
			*/
			/*!
				@brief Add and element to the heap.
				@param key [in] The element to add to the heap
				@details This method does not check to see if the element is already in the heap or that
				the element should be in the heap (i.e. > smallest).
			*/
			forceinline void push_back(TYPE key)
				{
array[0] = key;
make_heap();
//				insert_from(key, 0);
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

