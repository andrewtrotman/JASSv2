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

#include <map>
#include <tuple>
#include <sstream>
#include <iostream>

#include "dynamic_array.h"
#include "allocator_pool.h"
#include "index_postings_impact.h"
#include "compress_integer_variable_byte.h"

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
			static constexpr size_t initial_size = 16;		///< Initially allocate space for 4 elements
			static constexpr double growth_factor = 1.5;	///< Grow dynamic arrays by a factor of 1.5

		protected:
			/*
				INDEX_POSTINGS::POSTING
				-----------------------
			*/
			/*!
				@brief The representation of a single postings as a tuple of docid, term frequency and position.
			*/
			class posting
				{
				public:
					uint32_t document_id;						///< Internal document identifier
					uint32_t term_frequency;					///< Number of times the term occurs in this dociument

				public:
					/*
						INDEX_POSTINGS::POSTING::POSTING()
						----------------------------------
					*/
					/*!
						@brief Constructor.
						@param document_id [in] the document ID to use
						@param term_frequency [in] the term frequency to use
						@param position [in] the word position to use
					*/
					posting(uint32_t document_id, uint32_t term_frequency) :
						document_id(document_id),
						term_frequency(term_frequency)
						{
						/* Nothing */
						}
				};

		private:
			size_t highest_document;							///< The higest document number seen in this postings list (counting from 1)

			dynamic_array<uint8_t> postings;			///< Array holding the docids and the term frequencies (variable byte encoded docid first <d bytes>, then term frequency <1 byte>)

			/*
				CLASS INDEX_POSTINGS::ITERATOR
				------------------------------
			*/
			/*!
				@brief C++ iterator for iterating over an index_postings object.
			*/
			class iterator
				{
				public:
					/*
						ENUM INDEX_POSTINGS::ITERATOR::WHERE
						------------------------------------
					*/
					/*!
						@brief Whether the iterator is at the start of the end of the postings lists
					*/
					enum where
						{
						START = 0,			///< Iterator is from the start of the postings
						END = 1				///< Iterator is from the end of the postings
						};

				private:
					dynamic_array<uint8_t>::iterator positon_in_postings_list;			///< The iterator for the documents (1 per document).
					uint32_t last_docid;

				public:
					/*
						INDEX_POSTINGS::ITERATOR::ITERATOR()
						------------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object that this iterator is iterating over
						@param start [in] Whether this is an iterator for the START or END of the postings lists
					*/
					iterator(const index_postings &parent, where start):
						positon_in_postings_list(start == START ? parent.postings.begin() : parent.postings.end()),
						last_docid(0)
						{
						/*
							Nothing
						*/
						}

					/*
						INDEX_POSTINGS::ITERATOR::OPERATOR!=()
						--------------------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param other [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/

					bool operator!=(const iterator &other) const
						{
						/*
							If the positions are the same then the document and term frequency must be the same.
						*/
						if (other.positon_in_postings_list != positon_in_postings_list)
							return true;
						else
							return false;
						}

					/*
						INDEX_POSTINGS::ITERATOR::OPERATOR*()
						-------------------------------------
					*/
					/*!
						@brief Return a copy of the element pointed to by this iterator.
					*/
					const posting operator*()
						{
						compress_integer::integer delta;
						compress_integer_variable_byte::decompress_into(&delta, positon_in_postings_list);
						last_docid += delta;
						auto answer = posting(last_docid, *positon_in_postings_list);
						++positon_in_postings_list;
						return answer;
						}

					/*
						INDEX_POSTINGS::ITERATOR::OPERATOR++()
						--------------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					const iterator &operator++()
						{
						return *this;
						}
				};

		private:
			/*
				INDEX_POSTINGS::INDEX_POSTINGS()
				--------------------------------
			*/
			/*!
				@brief Parameterless construction is forbidden (so private).
			*/
			index_postings() :
				index_postings(*new allocator_pool(1024))
				{
				assert(0);
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
				highest_document(0),															// starts at 0, counts from 1
				postings(memory_pool, initial_size, growth_factor)					// give the allocator to the array
				{
				/* Nothing */
				}

			/*
				INDEX_POSTINGS::BEGIN()
				-----------------------
			*/
			/*!
				@brief Return an iterator pointing to the start of the postings.
				@return Iterator pointing to start of the postings.
			*/
			iterator begin(void) const
				{
				return iterator(*this, iterator::START);
				}

			/*
				INDEX_POSTINGS::END()
				---------------------
			*/
			/*!
				@brief Return an iterator pointing to the end of the postings.
				@return Iterator pointing to end of tne postings.
			*/
			iterator end(void) const
				{
				return iterator(*this, iterator::END);
				}

			/*
				INDEX_POSTINGS::PUSH_BACK()
				---------------------------
			*/
			/*!
				@brief Add to the end of the postings list.
			*/
			virtual void push_back(size_t document_id)
				{
				if (document_id == highest_document)
					{
					/*
						If this is the second or subseqent occurrence then just add 1 to the term frequency (and make sure it doesn't overflow).
					*/
					uint8_t &frequency = postings.back();
					if (frequency <= 0xFF)
						frequency++;			// cppcheck produces a false positive "Variable 'frequency' is modified but its new value is never used." - it looks like it doesn't fully understand that frequency is a reference.
					}
				else
					{
					/*
						First time we've seen this term in this document so add a new document ID and set the term frequency to 1.
					*/
					uint8_t space[10];				// worst case for a 64-bit integer (70 bits  / 7 bits per byte = 10 bytes)
					uint8_t *ending = space;		// write into here;

					/*
						Compress into the temporary buffer then copy into the postings
					*/
					compress_integer_variable_byte::compress_into(ending, document_id - highest_document);

					for (uint8_t *byte = space; byte < ending; byte++)
						postings.push_back(*byte);
					postings.push_back(1);

					highest_document = document_id;
					}
				}
			
			/*
				INDEX_POSTINGS::IMPACT_ORDER()
				------------------------------
			*/
			/*!
				@brief Take a postings list and turn it into an impact ordered postings list with impact headers.
				@details Take a postings list ordered \<d, tf\>... \<d, tf\> with d being in increasing order, and trurn into
				an impact ordered postings list ordered \<i\>\<d\>...\<d\>\<0\>\<i\>\<d\>...\<d\>\<0\> with \<i\> being in decreasing order and \<d\>
				begin in increasing order for each chunk.  It also generates a set of headers that poing to each \<d\>..\<d\> range,
				excluding \<i\> and excluding \<0\>.
				@param memory [in] All allocation to do with this process, including the result, is allocated in this arena.
				@result A reference to the impact ordered posting list allocated in the arena passed as paramter memory.
			*/
			index_postings_impact &impact_order(allocator &memory) const
				{
				std::array<uint32_t, 0x100> frequencies = {};
				size_t number_of_postings = 0;
				size_t highest_impact = 0;
				size_t lowest_impact = std::numeric_limits<size_t>::max();

				/*
					Count up the number of times each impact is seen and compute the highest and lowest impact scores
				*/
				for (const auto &posting : *this)
					{
					/*
						Calculate the maximum and minimum impact scores seen
					*/
					if (posting.term_frequency > highest_impact)
						highest_impact = posting.term_frequency;
					if (posting.term_frequency < lowest_impact)
						lowest_impact = posting.term_frequency;

					/*
						Count the number of times each frequency is seen
					*/
					frequencies[posting.term_frequency]++;
					number_of_postings++;
					}

				/*
					Count the number of unique impacts
				*/
				size_t number_of_impacts = 0;
				for (size_t which = lowest_impact; which <= highest_impact; which++)
					if (frequencies[which] != 0)
						number_of_impacts++;

				/*
					Allocate the postings list
					This is a little awkward, but it works... The object itself if allocated using the allocator
					passed by the caller, and as such it remains valid until that allocator's space is freed - which
					is not done by this method or any it calls.  Thus its possible to return a reference to the object
					and know its still valid once this method terminates
				*/
				index_postings_impact *postings_list_memory;
				postings_list_memory = new (reinterpret_cast<index_postings_impact *>(memory.malloc(sizeof(*postings_list_memory)))) index_postings_impact(number_of_impacts, number_of_postings, memory);
				index_postings_impact &postings_list = *postings_list_memory;

				/*
					Put the headers into place and turn the frequencies into pointers
				*/
				size_t cumulative = 0;
				size_t current_impact = 0;
				size_t impact_value = lowest_impact;
				for (size_t which = lowest_impact; which <= highest_impact; which++)
					{
					uint32_t *times = &frequencies[which];
					if (*times != 0)
						{
						auto prior = cumulative;
						/*
							Put the header in place.
						*/
						postings_list.header(current_impact, impact_value, &postings_list[prior], &postings_list[prior + *times]);

						/*
							Keep track of where in the list we are.
						*/
						current_impact++;
						cumulative += *times;
						*times = prior;
						}
					impact_value++;
					}

				/*
					Now place the postings in the right places
				*/
				for (const auto &posting : *this)
					{
					postings_list[frequencies[posting.term_frequency]] = posting.document_id;
					frequencies[posting.term_frequency]++;
					}
				return postings_list;
				}

			/*
				INDEX_POSTINGS::TEXT_RENDER()
				-----------------------------
			*/
			/*!
				@brief Dump a human-readable version of the postings list down the stream. Format is: <DocID, TF, Pos, Pos, Pos>...
				@param stream [in] The stream to write to.
			*/
			void text_render(std::ostream &stream) const
				{
				for (const auto &posting : *this)
					stream << '<' << posting.document_id << ',' << posting.term_frequency << '>';
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
				allocator_pool pool;
				index_postings postings(pool);
				
				postings.push_back(1);
				postings.push_back(1);
				postings.push_back(2);
				postings.push_back(173252);
				
				std::ostringstream result;
				
				postings.text_render(result);

				JASS_assert(strcmp(result.str().c_str(), "<1,2><2,1><173252,1>") == 0);

				puts("index_postings::PASSED");
				}
		};
		
	/*
		OPERATOR<<()
		------------
	*/
	/*!
		@brief Dump a human readable version of the postings list down an output stream.
		@param stream [in] The stream to write to.
		@param data [in] The postings list to write.
		@return The stream once the postings list has been written.
	*/
	inline std::ostream &operator<<(std::ostream &stream, const index_postings &data)
		{
		data.text_render(stream);
		return stream;
		}
	}
