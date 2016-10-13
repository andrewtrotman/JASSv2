/*
	INDEX_POSTINGS.H
	----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The structure used to store the postings during indexing.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "allocator.h"

namespace JASS
	{
	class index_postings
		{
		private:
			allocator pool;

			uint64_t highest_document_id;

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
			
			push_back(uint64_t document_id, uint64_t term_position)
				{
				
				}
		};
	}