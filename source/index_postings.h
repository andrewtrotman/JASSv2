/*
	INDEX_POSTINGS.H
	----------------
*/
#pragma once

#include "allocator.h"

namespace JASS
	{
	class index_postings
		{
		private:
			allocator pool;

			size_t highest_document_id;

			uint8_t *document_ids;
			uint16_t *term_frequencies;
			uint8_t *positions;
			
		private:
			index_postings()
				{
				/*
					Nothing
				*/
				}
			
		public:
			index_postings(allocator pool) :
				pool(pool)
				{
				document_ids = term_frequencies = positions = nullptr;
				}
			
			push_back(size_t document_id, size_t term_position)
				{
				
				}
		};
	}