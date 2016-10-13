/*
	DYNAMIC_ARRAY.H
	---------------
*/
#pragma once

#include <array>
#include <atomic>
#include <thread>

#include "allocator.h"

namespace JASS
	{
	template <typename TYPE>
	class dynamic_array
		{
		protected:
			class node
				{
				public:
					TYPE *data;
					node *next;
					size_t allocated;
					std::atomic<size_t> used;
					
				public:
					node(allocator &pool, size_t size):
						next(nullptr),
						allocated(size),
						used(0)
						{
						data = new (pool.malloc(size * sizeof(TYPE))) TYPE[size];
						}
				};

			class iterator
				{
				private:
					const node *current_node;
					TYPE *data;
					
				public:
					iterator(node *node, size_t element):
						current_node(node),
						data(node == nullptr ? nullptr : node->data + element)
						{
						/*
							Nothing
						*/
						}
					
					bool operator!= (const iterator &other) const
						{
						if (current_node != other.current_node)
							return true;
						else if (data != other.data)
							return true;
						return false;
						}
 
					int operator* () const
						{
						return *data;
						}
 
					const iterator &operator++()
						{
						data++;
						if (data >= current_node->data + current_node->used)
							{
							current_node = current_node->next;
							if (current_node == nullptr)
								data = nullptr;
							else
								data = current_node->data;
							}
						return *this;
						}
				};
			
		private:
			dynamic_array()
				{
				/*
					Nothing
				*/
				}

		public:
			allocator &pool;
			node *head;
			std::atomic<node *> tail;
			double growth_factor;
			
		public:
			dynamic_array(allocator &pool, size_t initial_size = 1, double growth_factor = 1.4) :
				pool(pool),
				growth_factor(growth_factor)
				{
				head = tail = new (pool.malloc(sizeof(node))) node(pool, initial_size);
				}
			
			iterator begin(void) const
				{
				return iterator(head, 0);
				}
			
			iterator end(void) const
				{
				return iterator(nullptr, 0);
				}
			
			void push_back(const TYPE &element)
				{
				do
					{
					node *last = tail;
					
					size_t slot = last->used++;

					if (slot < last->allocated)
						{
						last->data[slot] = element;
						break;
						}
					else
						{
						last->used = last->allocated;
						node *another = new (pool.malloc(sizeof(node))) node(pool, last->allocated * growth_factor);
						if (tail.compare_exchange_strong(last, another))
							last->next = another;
						}
					}
				while(true);
				}
			
			
			
			void unittest_thread(size_t high)
				{
				for (size_t counter = 0; counter < high; counter++)
					push_back(counter);
				}
			
			static void unittest(void)
				{
				static constexpr size_t highest_value = 100000;
				static constexpr size_t thread_count = 50;
				allocator_pool memory;
				dynamic_array<size_t> array(memory, 1, 1);
				std::thread thread_pool[thread_count];
				
				for (size_t which = 0; which < thread_count; which++)
					thread_pool[which] = std::thread(&dynamic_array<size_t>::unittest_thread, &array, highest_value);
				for (size_t which = 0; which < thread_count; which++)
					thread_pool[which].join();
					
					
				std::array<size_t, highest_value> check;
				for (const auto &val : array)
					check[val] = 0;
					
				for (const auto &val : array)
					check[val]++;
				
				for (const auto &val : check)
					{
					JASS_assert(val == thread_count);
					}
					
				puts("dynamic_array::PASSED");
				}
		};
	}