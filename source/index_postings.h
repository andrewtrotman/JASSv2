/*
	INDEX_POSTINGS.H
	----------------
*/
#pragma once

#include "dynamic_array.h"
#include "allocator_pool.h"

namespace JASS
	{
	class index_postings
		{
		private:
			allocator &pool;

			size_t highest_document_id;

			dynamic_array<uint8_t> document_ids;
			dynamic_array<uint16_t> term_frequencies;
			dynamic_array<uint8_t > positions;
			
		private:
			index_postings() :
				pool(*new allocator_pool),
				document_ids(pool),
				term_frequencies(pool),
				positions(pool)
				{
				highest_document_id = 0;
				}
			
		public:
			index_postings(allocator &pool) :
				pool(pool),
				document_ids(pool),
				term_frequencies(pool),
				positions(pool)
				{
				highest_document_id = 0;
				}
			
			void push_back(size_t document_id, size_t term_position)
				{
				
				}
			static void unittest(void)
				{
				index_postings postings;
				postings.push_back(1,1);
				}
		};
	}