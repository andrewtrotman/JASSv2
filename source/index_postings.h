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
			size_t highest_document;
			size_t highest_position;

			dynamic_array<uint32_t> document_ids;
			dynamic_array<uint16_t> term_frequencies;
			dynamic_array<uint32_t > positions;
			
		private:
	
			/*
				private to avoid parameterless construction (this can't be called, so can't leak either).
			*/
			index_postings() :
				index_postings(*new allocator_pool(1024))
				{
				/*
					Nothing
				*/
				}

		public:
			index_postings(allocator &pool) :
				highest_document(0),
				highest_position(0),
				document_ids(pool),
				term_frequencies(pool),
				positions(pool)
				{
				/*
					Nothing
				*/
				}
			
			virtual void push_back(size_t document_id, size_t position)
				{
				if (document_id == highest_document)
					{
					uint16_t &frequency = term_frequencies.back();
					if (frequency <= 0xFFFE)
						frequency++;
					}
				else
					{
					document_ids.push_back(document_id);
					term_frequencies.push_back(1);
					}
				positions.push_back(position);
				highest_position = position;
				}
			
			static void unittest(void)
				{
				index_postings postings;
				}
		};
	}