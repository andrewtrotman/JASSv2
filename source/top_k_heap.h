/*
	TOP_K_HEAP.H
	------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Implementation of a top-k keeper using a heap
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <array>
#include <random>
#include <chrono>
#include <sstream>
#include <algorithm>
#include <functional>

#include "asserts.h"
#include "forceinline.h"
#include "pointer_box.h"
#include "allocator_cpp.h"
#include "allocator_pool.h"

namespace JASS
	{
	/*
		CLASS TOP_K_HEAP
		----------------
	*/

	/*!
		@brief A heap that keeps the top-k elements and discards the remainder. SLOW, consider using JASS::heap instead.
		@details A heap that keeps at most the top K elements. Comparison is done using operator>().  The underlying data structure is
		a std::vector.  Elements are added to the vector using std::vector::push_back() and removed using std::vector::pop_back().  To add
		an element to a top-k heap call top_k_heap::push_back().
		@tparam ELEMENT The heap is a heap of these elements.
	*/
	template <typename ELEMENT>
	class top_k_heap
		{
		/*!
			@brief Output a human readable serialisation to an ostream
			@relates top_k_heap
		*/
		template<typename A> friend std::ostream &operator<<(std::ostream &stream, const top_k_heap<A> &data);

		private:
			std::vector<ELEMENT, allocator_cpp<ELEMENT>> members;			///< The array over which the heap is constructed
			size_t total_insertions;				///< Total number of calls to insert
			bool heap_ordered;						///< is this heap ordered as a heap yet, or not (i.e. has top_k_heap::sort() or top_k_heap::move() been called?).
			bool front_correct;						///< Is the front of the heap correct (which it might not be if its never been heap ordered)
			size_t k;								///< The k in top-k
		
		public:
			/*!
				@typedef iterator
				@brief Type representing an iterator over an object of this type.
			*/
			typedef typename std::vector<ELEMENT, allocator_cpp<ELEMENT>>::iterator iterator;
			
		public:
			/*
				TOP_K_HEAP::TOP_K_HEAP()
				------------------------
			*/
			/*!
				@brief Constructor
			*/
			top_k_heap(size_t k, allocator &allocator) :
				members(allocator_cpp<ELEMENT>(allocator)),
				total_insertions(0),
				heap_ordered(false),
				front_correct(false),
				k(k)
				{
				/* Nothing */
				}

			/*
				TOP_K_HEAP::PUSH_BACK()
				-----------------------
			*/
			/*!
				@brief Add and element to the heap
				@param element [in] The element to add to the heap
			*/
			void push_back(const ELEMENT &element)
				{
				/*
					Count the number of calls to this method
				*/
				total_insertions++;
				
				if (total_insertions < k)
					{
					/*
						If adding this element hasn't yet filled the heap then don't make a heap
					*/
					members.push_back(element);
					}
				else if (total_insertions == k)
					{
					/*
						If adding this element causes the array to fill then make a heap
					*/
					members.push_back(element);
					make_heap(members.begin(), members.end(), std::greater<ELEMENT>());
					front_correct = heap_ordered = true;
					}
				else if (total_insertions > k)
					{
					/*
						If adding this element will overflow the heap then check to see if it belongs
					*/
					if (element > front())
						{
						/*
							make sure we have a heap before we manipulate it
						*/
						if (!heap_ordered)
							{
							make_heap(members.begin(), members.end(), std::greater<ELEMENT>());
							front_correct = heap_ordered = true;
							}
						
						/*
							This element belongs in the heap so remove the smallest (according to operator>()) element and insert this element
						*/
						pop_heap(members.begin(), members.end(), std::greater<ELEMENT>());
						members.pop_back();
						members.push_back(element);
						push_heap(members.begin(), members.end(), std::greater<ELEMENT>());
						}
					}
				}
			
			/*
				TOP_K_HEAP::PROMOTE()
				---------------------
			*/
			/*!
				@brief Promote an element within the heap.
				@details This method provides the illusion that an element within the heap has moved, which it does
				by making sure front() is correct, but shuffling the remainder of the heap (which is no longer a heap). The
				heap will be correctly ordered on a call to push_back() that results in a eviction or many other methods.
				This is used whe, for exmaple, a heap of pointers is being used and the value at the pointer has become larger (but not smaller).
				@param element [in] The element to move within the heap.
			*/
			void promote(const ELEMENT &element)
				{
				/*
					If we're the front element of the heap then we might get a new front so we need to make the heap
				*/
				if (element == members.front())
					{
					make_heap(members.begin(), members.end(), std::greater<ELEMENT>());
					front_correct = heap_ordered = true;
					}
				else
					{
					front_correct = true;
					heap_ordered = false;
					}
				}

			/*
				TOP_K_HEAP::FRONT()
				-------------------
			*/
			/*!
				@brief Return a reference to the first (i.e. smallest by operator>()) element in the heap.
				@return Reference to the first element.
			*/
			forceinline auto front()
				{
				/*
					If we're sorted then turn it back into a heap
				*/
				if (!front_correct)
					{
					make_heap(members.begin(), members.end(), std::greater<ELEMENT>());
					front_correct = heap_ordered = true;
					}

				return members.front();
				}

			/*
				TOP_K_HEAP::BEGIN()
				-------------------
			*/
			/*!
				@brief Return an iterator pointing to start of the heap
				@return Iterator pointing to start of the heap.
			*/
			auto begin()
				{
				return members.begin();
				}

			/*
				TOP_K_HEAP::BEGIN()
				-------------------
			*/
			/*!
				@brief Return an iterator pointing to start of the heap
				@return Iterator pointing to start of the heap.
			*/
			auto begin() const
				{
				return members.begin();
				}
			
			/*
				TOP_K_HEAP::END()
				-----------------
			*/
			/*!
				@brief Return an iterator pointing past the end of the heap.
				@return Iterator pointing past the end of the heap.
			*/
			auto end()
				{
				return members.end();
				}

			/*
				TOP_K_HEAP::END()
				-----------------
			*/
			/*!
				@brief Return an iterator pointing past the end of the heap.
				@return Iterator pointing past the end of the heap.
			*/
			auto end() const
				{
				return members.end();
				}
			
			/*
				TOP_K_HEAP::SORT()
				------------------
			*/
			/*!
				@brief Sort the heap so that an iteration over it will produce the results in sorted (rather than heap) order.
			*/
			void sort(void)
				{
				if (heap_ordered)
					sort_heap(members.begin(), members.end(), std::greater<ELEMENT>());
				else
					std::sort(members.begin(), members.end(), std::greater<ELEMENT>());
				front_correct = heap_ordered = false;				// because we're not ordered high to low rather than min-heap low to high
				}
			
			/*
				TOP_K_HEAP::SIZE()
				------------------
			*/
			/*!
				@brief Return the numner of element in the heap (which might be smaller than k, but never larger).
				@return The number of elements in the heap.
			*/
			size_t size(void) const
				{
				return members.size();
				}

			/*
				TOP_K_HEAP::CAPICITY()
				----------------------
			*/
			/*!
				@brief Return the maximum number of elements the heap can hold (i.e. k)
				@return k (from top-k).
			*/
			size_t capacity(void) const
				{
				return k;
				}
			
			/*
				TOP_K_HEAP::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Memory to allocate into
				*/
				allocator_pool memory;
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
					top_k_heap<int> heap(5, memory);
					for (const auto &element : sequence)
						heap.push_back(element);

					/*
						Make sure we have a heap and its the right size
					*/
					JASS_assert(heap.end() - heap.begin() > 0);
					JASS_assert(static_cast<size_t>(heap.end() - heap.begin()) == heap.size());
					JASS_assert(std::is_heap(heap.begin(), heap.end(), std::greater<ELEMENT>()));

					/*
						Sort the heap
					*/
					heap.sort();
		
					/*
						Serialise and compare to the known correct result
					*/
					std::ostringstream result;
					result << heap;
					JASS_assert(result.str() == "9 8 7 6 5");
					}

				/*
					Add fewer elements to the heap than the heap size to make sure short answer lists work too,
				*/
				top_k_heap<int> big_heap(20, memory);
				for (const auto &element : sequence)
					big_heap.push_back(element);


				/*
					Make sure the heap's stats are correct.
				*/
				JASS_assert(big_heap.size() == 10);
				JASS_assert(big_heap.capacity() == 20);
				JASS_assert(static_cast<size_t>(big_heap.end() - big_heap.begin()) == big_heap.size());
				
				/*
					as size() is less than capacity() its unlikely (but possible) that we have a heap, so either is correct (so there's nothing to check)
				*/
				//JASS_assert(std::is_heap(big_heap.begin(), big_heap.end()));
					
				/*
					Sort the heap
				*/
				big_heap.sort();
	
				/*
					Serialise and compare to the known correct result
				*/
				std::ostringstream big_result;
				big_result << big_heap;
				JASS_assert(big_result.str() == "9 8 7 6 5 4 3 2 1 0");

				/*
					Add exactly the correct number of elements to the heap to make sure that works
				*/
				top_k_heap<int> exact_heap(10, memory);
				for (const auto &element : sequence)
					exact_heap.push_back(element);
					
				/*
					Make sure the heap's stats are correct.
				*/
				JASS_assert(exact_heap.size() == exact_heap.capacity());
				JASS_assert(static_cast<size_t>(exact_heap.end() - exact_heap.begin()) == exact_heap.size());
				JASS_assert(std::is_heap(exact_heap.begin(), exact_heap.end(), std::greater<ELEMENT>()));

				/*
					Check the front() methods
				*/
				const int const_got = exact_heap.front();
				int got = exact_heap.front();
				JASS_assert(got == const_got);
				JASS_assert(got == 0);

				/*
					Sort the heap
				*/
				exact_heap.sort();
	
				/*
					Serialise and compare to the known correct result
				*/
				std::ostringstream exact_result;
				exact_result << exact_heap;
				JASS_assert(exact_result.str() == "9 8 7 6 5 4 3 2 1 0");

				/*
					Check insetion once we've sorted the heap
				*/
				top_k_heap<int> small_heap(2, memory);
				for (const auto element : sequence)
					{
					small_heap.sort();
					small_heap.push_back(element);
					}
				small_heap.sort();
				auto front = small_heap.front();
				JASS_assert(front == 8);
				small_heap.sort();
				std::ostringstream small_result;
				small_result << small_heap;
				JASS_assert(small_result.str() == "9 8");

				/*
					Test promote()
				*/
				int eight = 8;
				int *eight_pointer = &eight;
				top_k_heap<pointer_box<int>> pointer_heap(2, memory);
				for (size_t element = 0; element < sizeof(sequence) / sizeof(*sequence); element++)
					{
					pointer_heap.sort();
					if (sequence[element] == 8)
						pointer_heap.push_back(eight_pointer);
					else
						pointer_heap.push_back(&sequence[element]);
					}
				pointer_heap.sort();
				auto pointer_front = pointer_heap.front();
				JASS_assert(*pointer_front == 8);
				pointer_heap.sort();

				*eight_pointer += 3;
				pointer_heap.promote(&eight);

				int ten = 10;
				pointer_heap.push_back(&ten);
				pointer_heap.sort();

				std::ostringstream pointer_result;
				for (const auto &pointer : pointer_heap)
					pointer_result << *pointer;
				JASS_assert(pointer_result.str() == "1110");

				/*
					We passed!
				*/
				puts("top_k_heap::PASSED");
				}
		};
		
	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump the contents of a heap down an output stream.
		@param stream [in] The stream to write to.
		@param data [in] The heap to write.
		@tparam ELEMENT The type used as the key to the element (must include KEY(allocator, KEY) copy constructor)
		@tparam K The element data returned given the key (must include ELEMENT(allocator) constructur)
		@return The stream once the tree has been written.
	*/
	template <typename ELEMENT>
	inline std::ostream &operator<<(std::ostream &stream, const top_k_heap<ELEMENT> &data)
		{
		bool first_time = true;
		
		for (const auto &element : data)
			{
			if (!first_time)
				stream << " ";
			stream << element;
			first_time = false;
			}
		return stream;
		}
	}
