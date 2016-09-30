/*
	HASH_TABLE.H
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

namespace JASS
	{
	template <typename KEY, TYPE, size_t bits = 24>
	class hash_table
		{
		protected:
			constexpr size_t hash_table_size = size(bits);
		
		protected:
			TYPE *table[hash_table_size];
			
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
			hash_table()
				{
				memset(table, 0, sizeof(table));
				}
				
			~hash_table()
				{
				/*
					Nothing
				*/
				}
			
			TYPE &operator[](KEY key)
				{
				hash = 2;
				return table[hash];
				}

			static void unittest(void)
				{
				}
		};
	}
