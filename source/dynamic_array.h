/*
	DYNAMIC_ARRAY.H
	---------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Thread-safe grow-only dynamic array using the thread-safe allocator.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <array>
#include <atomic>
#include <thread>

#include "asserts.h"
#include "allocator_pool.h"

namespace JASS
	{
	/*
		CLASS DYNAMIC_ARRAY
		-------------------
	*/
	/*!
		@brief Thread-safe grow-only dynamic array using the thread-safe allocator.
		@details The array data is stored in a linked list of chunks where each chunk is larger then the previous as the array is growing.  Although random access
		is supported, it is slow as it is necessary to walk the linked list to find the given element (see operator[]()).  The iterator, however, does not need to do this and has
		O(1) access time to each element.
		@tparam TYPE The dynamic array is an array of this type.
	*/
	template <typename TYPE>
	class dynamic_array
		{
		protected:
			/*
				CLASS DYNAMIC_ARRAY::NODE
				-------------------------
			*/
			/*!
				@brief The array is stored as a linked list of nodes where each node points to some number of elements.
			*/
			class node
				{
				public:
					TYPE *data;									///< The array data for this node.
					node *next;									///< Pointer to the next node in the chain.
					size_t allocated;							///< The size of this node's data object (in elements).
					std::atomic<size_t> used;				///< The number of elements in data that are used (always <= allocated).
					
				public:
					/*
						DYNAMIC_ARRAY::NODE::NODE()
						---------------------------
					*/
					/*!
						@brief Constructor
						@param pool [in] The pool allocator used to allocate the data controled by this node.
						@param size [in] The size (in elements) of the data to be controlled by this node.
					*/
					node(allocator &pool, size_t size):
						next(nullptr),				// this is the end of the linked list.
						allocated(size),			// the data array is this size (in elements).
						used(0)						// the data array is empty.
						{
						/*
							Allocate the data this node controls.
						*/
						data = new (pool.malloc(size * sizeof(TYPE))) TYPE[size];
						}
				};

		public:
			/*
				CLASS DYNAMIC_ARRAY::ITERATOR
				-----------------------------
			*/
			/*!
				@brief C++ iterator for iterating over a dynamic_array object.
				@details See http://www.cprogramming.com/c++11/c++11-ranged-for-loop.html for details on how to write a C++11 iterator.
			*/
			class iterator
				{
				private:
					const node *current_node;					///< The node that this iterator is currently looking at.
					TYPE *data;										///< Pointer to the element within current_node that this object is looking at.
					
				public:
					/*
						DYNAMIC_ARRAY::ITERATOR::ITERATOR()
						-----------------------------------
					*/
					/*!
						@brief constructor
						@param node [in] The node that this iterator should start looking at.
						@param element [in] Which element within node this iterator should start looking at (normally 0).
					*/
					iterator(node *node, size_t element):
						current_node(node),
						data(node == nullptr ? nullptr : node->data + element)					// the "end" is represented as (NULL, NULL)
						{
						/*
							Nothing
						*/
						}
					/*
						DYNAMIC_ARRAY::ITERATOR::OPERATOR!=()
						-------------------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param other [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/
					bool operator!=(const iterator &other) const
						{
						/*
							If the data pointers are different then the two must be different.
						*/
						return data != other.data;
						}
 
					/*
						DYNAMIC_ARRAY::ITERATOR::OPERATOR*()
						------------------------------------
					*/
					/*!
						@brief Return a reference to the element pointed to by this iterator.
					*/
					TYPE &operator*() const
						{
						return *data;
						}
 
					/*
						DYNAMIC_ARRAY::ITERATOR::OPERATOR++()
						------------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					const iterator &operator++()
						{
						/*
							Just move on to the next element
						*/
						data++;
						
						/*
							but if we're past the end of the current node then move on to the next node
						*/
						if (data >= current_node->data + current_node->used)
							{
							current_node = current_node->next;
							
							/*
								If we're past the end of the list then we're done.
							*/
							if (current_node == nullptr)
								data = nullptr;
							else
								data = current_node->data;
							}
						return *this;
						}
				};
			
		public:
			allocator &pool;							///< The pool allocator used for all allocation by this object.
			node *head;									///< Pointer to the head of the linked list of blocks of data.
			std::atomic<node *> tail;				///< Pointer to the tail of the linked list of blocks of data.  It std::atomic<> so that it can grow lock-free
			double growth_factor;					///< The next chunk in the linked list is this much larger than the previous.
			
		public:
			/*
				DYNAMIC_ARRAY::DYNAMIC_ARRAY()
				------------------------------
			*/
			/*!
				@brief Constructor
			*/
			dynamic_array() = delete;

			/*
				DYNAMIC_ARRAY::DYNAMIC_ARRAY()
				------------------------------
			*/
			/*!
				@brief Constructor.
				@param pool [in] The pool allocator used for all allocation done by this object.
				@param initial_size [in] The size (in elements) of the initial allocation in the linked list.
				@param growth_factor [in] The next node in the linked list stored an element this many times larger than the previous (as an integer).
			*/
			dynamic_array(allocator &pool, size_t initial_size = 1, double growth_factor = 1.5) :
				pool(pool),
				growth_factor(growth_factor)
				{
				/*
					Allocate space for the first write
				*/
				head = tail = new (pool.malloc(sizeof(node))) node(pool, initial_size);
				}
			
			/*
				DYNAMIC_ARRAY::BEGIN()
				----------------------
			*/
			/*!
				@brief Return an iterator pointing to the start of the array.
				@return Iterator pointing to start of array.
			*/
			iterator begin(void) const
				{
				/*
					If there's nothing in the array then we're at the end, else we're at the head.
				*/
				if (head->used == 0)
					return end();
				else
					return iterator(head, 0);
				}
			
			/*
				DYNAMIC_ARRAY::END()
				--------------------
			*/
			/*!
				@brief Return an iterator pointing to the end of the array.
				@return Iterator pointing to end of array.
			*/
			iterator end(void) const
				{
				return iterator(nullptr, 0);
				}

			/*
				DYNAMIC_ARRAY::BACK()
				---------------------
			*/
			/*!
				@brief Return an reference to the final (used) element in the dynamic array.
				@return Reference to the last used element in the array.
			*/
			TYPE &back(void) const
				{
				auto tail_pointer = tail.load();
				return tail_pointer->data[tail_pointer->used - 1];
				}
			
			/*
				DYNAMIC_ARRAY::PUSH_BACK()
				--------------------------
			*/
			/*!
				@brief Add an element to the end of the array.
				@param element [in] The element to add.
			*/
			void push_back(const TYPE &element)
				{
				do
					{
					/*
						Take a copy of the pointer to the end of the list
					*/
					node *last = tail;
					
					/*
						Take a slot (using std::atomic<>++)
					*/
					size_t slot = last->used++;

					/*
						If that slot is within range then copy the element into the array
					*/
					if (slot < last->allocated)
						{
						/*
							Copy and done.
						*/
						last->data[slot] = element;
						break;
						}
					else
						{
						/*
							We've walked past the end so we allocate space for a new node (and elements in that node) and add it to the list.
						*/
						last->used = last->allocated;
						node *another = new (pool.malloc(sizeof(node))) node(pool, (size_t)(last->allocated * growth_factor));
						/*
							Atomicly make it the tail and if we succeed than make the previous node in the list point to this one.
							If we fail then the pool allocator won't take the memory back so ignore and re-try
						*/
						if (tail.compare_exchange_strong(last, another))
							last->next = another;
						}
					}
				while(true);
				}

			/*
				DYNAMIC_ARRAY::OPERATOR[]()
				---------------------------
			*/
			/*!
				@brief Return a reference to the given element (counting from 0).
				@details This method must walk the linked list to find
				the requested element and then returns a reference to it.
				Since the growth factor might be 1 and the initial
				allocation size might be 1, the worst case for requesting
				the final element is O(n) where n is the number of
				elements in the array.  Walking through the array
				accessing each element is therefore O(n^2) - so don't do
				this.  The preferred method for iterating over the array
				is to use a for each iterator (i.e. through begin() and
				end()). The C++ std::array has "undefined behavior" if
				the given index is out-of-range.  This, too, has
				undefined behaviour in that case.
				@param element [in] The element to find.
			*/
			TYPE &operator[](size_t element)
				{
				/*
					Walk the linked list until we find the requested element
				*/
				for (node *current = head; current != nullptr; current = current->next)
					if (element < current->used)
						return current->data[element];				// got it
					else
						element -= current->used;						// its further down the list
					
				/*
					The undefined behaviour is to return the first element in the array.
					Which will probably cause a crash if there are no elements in the array.
				*/
				return head->data[0];
				}

			/*
				DYNAMIC_ARRAY::SERIALISE()
				--------------------------
			*/
			/*!
				@brief Serialise the dynamic array into a single linear sequence.
				@param into [out] pointer to the buffer to serialise (at most size_of_into elements) into.
				@param size_of_into [in] the amount of space (in TYPE elements) available in into (so for long[1] it would be 1).
				@return The amount of space it would take to store the entire serialised array in TYPE elements.
			*/
			size_t serialise(TYPE *into, size_t size_of_into) const
				{
				size_t would_take = 0;

				if (into == nullptr)
					{
					/*
						Just compute the length (but do not serialise).
					*/
					for (node *current = head; current != nullptr; current = current->next)
						would_take += current->used;
					}
				else
					{
					/*
						Compute the length and do the copy.
					*/
					for (node *current = head; current != nullptr; current = current->next)
						{
						std::copy(current->data, current->data + (current->used <= size_of_into ? current->used.load() : size_of_into), into);
						size_of_into -= current->used;
						would_take += current->used;
						into += current->used;
						}
					}
				return would_take;
				}

			/*
				DYNAMIC_ARRAY::UNITTEST_THREAD()
				--------------------------------
			*/
			/*!
				@brief Part of the unit test this method adds elements 0..high to the array.
				@param high [in] How many integer elements to add to the array.
			*/
			void unittest_thread(size_t high)
				{
				for (size_t counter = 0; counter < high; counter++)
					push_back(counter);
				}

			/*
				DYNAMIC_ARRAY::UNITTEST()
				-------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				static constexpr size_t highest_value = 100000;					// add integers 0..highest_value to a dynamic_array<size_t>.
				static constexpr size_t thread_count = 50;						// use this many concurrent threads to add those elements.
				allocator_pool memory;													// pool allocator that holds all the memory.
				dynamic_array<size_t> array(memory, 1, 1);						// the dynamic_array.
				std::array<std::thread, thread_count> thread_pool;				// a thread pool used to ensure all the threads are done.

				/*
					The "test" is to have thread_count threads each adding integers 0..highest_value to the array at once.  The test is
					considered passed if each integer appears in the array thread_count times.
				*/
				/*
					Creare a number of threads all compeating to add to the same array
				*/
				for (size_t which = 0; which < thread_count; which++)
					thread_pool[which] = std::thread(&dynamic_array<size_t>::unittest_thread, &array, highest_value);
					
				/*
					Wait until they are all done
				*/
				for (size_t which = 0; which < thread_count; which++)
					thread_pool[which].join();

				/*
					We're going to walk through the array once counting the number of times each integer occurs so
					allocate an array of the right size, and initialise it to all 0.
				*/
				size_t *check = new size_t[highest_value];
				for (size_t *element = check; element < check + highest_value; element++)
					*element = 0;
					
				/*
					Walk through the array taking a talley
				*/
				for (const auto &element : array)
					check[element]++;
				
				/*
					Check each talley to make sure it is the right size.
				*/
				for (size_t *element  = check; element < check + highest_value; element++)
					JASS_assert(*element == thread_count);

				delete [] check;

				/*
					Simple check to make sure indexing into the array works.  Simply fill the array with numbers and make sure array[element] == element.
					As this is O(n^2), the size of the array will be kept small so that it completes in reasonable time.
				*/
				static constexpr size_t highest_index = 100;

				dynamic_array<size_t> indexable(memory, 1, 1);
				indexable.unittest_thread(highest_index);
				for (size_t index = 0; index < highest_index; index++)
					{
					JASS_assert(indexable[index] == index);
					}

				/*
					Check the back() method.
				*/
				dynamic_array<size_t> another(memory, 1, 1);
				for (size_t which = 0; which < 10; which++)
					{
					another.push_back(which);
					JASS_assert(another.back() == which);
					}

				/*
					Check out-of-range behaviour
				*/
				JASS_assert(&another[1024] == &another[0]);

				/*
					Passed!
				*/
				puts("dynamic_array::PASSED");
				}
		};
	}
