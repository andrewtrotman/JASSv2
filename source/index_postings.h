/*
	INDEX_POSTINGS.H
	----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Non-thread-Safe object that holds a single postings list during indexing.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "dynamic_array.h"
#include "allocator_pool.h"

namespace JASS
	{
	/*
		CLASS INDEX_POSTINGS
		--------------------
	*/
	/*!
		@brief Non-thread-Safe object that accumulates a single postings list during indexing.
	*/
	class index_postings
		{
		private:
			size_t highest_document;							///< The higest document number seen in this postings list (counting from 1)
			size_t highest_position;							///< The higest position seen in this postings list (counting from 1)

			dynamic_array<uint32_t> document_ids;			///< Array holding the document IDs
			dynamic_array<uint16_t> term_frequencies;		///< Array holding the term frequencies
			dynamic_array<uint32_t > positions;				///< Array holding the term positions
			
		private:
			/*
				INDEX_POSTINGS::INDEX_POSTINGS()
				--------------------------------
			*/
			/*!
				@brief Parameterless construction is forbidden (private).
			*/
			index_postings() :
				index_postings(*new allocator_pool(1024))
				{
				/*
					Nothing
				*/
				}

		public:
			/*
				INDEX_POSTINGS::INDEX_POSTINGS()
				--------------------------------
			*/
			/*!
				@brief Constructor.
				@param memory_pool [in] All allocation is from this allocator.
			*/
			index_postings(allocator &memory_pool) :
				highest_document(0),									// starts at 0, counts from 1
				highest_position(0),									// starts at 0, counts from 1
				document_ids(memory_pool),							// give the allocator to the array
				term_frequencies(memory_pool),					// give the allocator to the array
				positions(memory_pool)								// give the allocator to the array
				{
				/*
					Nothing
				*/
				}
			
			/*
				INDEX_POSTINGS::PUSH_BACK()
				---------------------------
			*/
			/*!
				@brief Add to the end of the postings list.
			*/
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
			/*
				INDEX_POSTINGS::TEXT_RENDER()
				-----------------------------
			*/
			/*!
				@brief Dump a human-readable version of the postings list down the stream.
				@param stream [in] The stream to write to.
			*/
			void text_render(std::ostream &stream) const
				{
				auto frequency = term_frequencies.begin();
				auto position = positions.begin();
				for (const auto &id : document_ids)
					{
					stream << '<' << id << ',' << *frequency << ',';
					
					for (size_t which = 0; which < *frequency; which++)
						{
						stream << *position << ',';
						++position;
						}

					stream << '>';
						
					++frequency;
					}
				}
			
			/*
				INDEX_POSTINGS::UNITTEST()
				--------------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void)
				{
				index_postings postings;
				}
		};
		
	/*
		OPERATOR<<()
		------------
		@brief Dump a human readable version of the postings list down an output stream.
		@param stream [in] The stream to write to.
		@param tree [in] The postings list to write.
		@return The stream once the postings list has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const index_postings &data)
		{
		data.text_render(stream);
		return stream;
		}
	}