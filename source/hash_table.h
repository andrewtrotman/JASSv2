/*
	HASH_TABLE.H
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

#include <atomic>

#include "allocator.h"
#include "binary_tree.h"
#include "hash_pearson.h"

namespace JASS
	{
	template <typename KEY, TYPE, size_t BITS = 24>
	class hash_table
		{
		protected:
			constexpr size_t hash_table_size = size(bits);
			
		protected:
			std::atomic<binary_tree<KEY, ELEMENT> *> table[hash_table_size];
			allocator &memory_pool;
			
		protected:
			constexpr size_t size(size_t bits)
				{
				switch (bits)
					{
					case 8:
						return 256;
					case 16:
						return 65536;
					case 24:
						return 16777216
					default:
						static_assert(false, "hash_table must be of size 8, 16, or 24");
					}
				return 0;
				}

		public:
			hash_table(allocator &pool) : memory_pool(pool)
				{
				memset(table, 0, sizeof(table));
				}
				
			~hash_table()
				{
				/*
					Nothing
				*/
				}
			
			TYPE &operator[](const KEY &key)
				{
				size_t hash = hash_pearson<BITS>(key);
				if (table[hash] == nullptr)
					{
					binary_tree<KEY, ELEMENT> *empty = nullptr;
					binary_tree<KEY, ELEMENT> *new_tree = new (memory_pool->malloc(sizeof(*new_tree))) binary_tree<KEY, ELEMENT>(memory_pool);
					
					hash_table[hash].compare_exchange_strong(empty, new_tree);
					/*
						If the Compare and Swap fails then ignore as it simply means there is now a tree in the table, it doesn't mean the key is in the tree.
					*/
					}
				return table[hash][key];
				}

			static void unittest(void)
				{
				allocator_pool pool;
				hash_table<slice, slice> map(pool);
				
				map[slice("5")] = slice("five");
				map[slice("3")] = slice("three");
				map[slice("7")] = slice("seven");
				map[slice("4")] = slice("four");
				map[slice("2")] = slice("two");
				map[slice("1")] = slice("one");
				map[slice("9")] = slice("nine");
				map[slice("6")] = slice("six");
				map[slice("8")] = slice("eight");
				map[slice("0")];

				const char *answer = "    9->nine\n      8->eight\n  7->seven\n    6->six\n5->five\n    4->four\n  3->three\n    2->two\n      1->one\n        0->\n";
				
				std::ostringstream serialised;
				serialised << map;
				JASS_assert(strcmp(serialised.str().c_str(), answer) == 0);
	
				puts("hash_table::PASSED");
				/*
					HASH_PEARSON needs a unittest
				*/
				}
		};
	}
