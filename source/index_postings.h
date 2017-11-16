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
			static constexpr size_t initial_size = 4;		///< Initially allocate space for 4 elements
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
					uint32_t position;							///< The word position of this term in this document
				public:
					/*
						INDEX_POSTINGS::POSTING::POSTING()
						----------------------------------
					*/
					/*!
						@brief COnstructor.
						@param document_id [in] the document ID to use
						@param term_frequency [in] the term frequency to use
						@param position [in] the word position to use
					*/
					posting(uint32_t document_id, uint32_t term_frequency, uint32_t position) :
						document_id(document_id),
						term_frequency(term_frequency),
						position(position)
						{
						/* Nothing */
						}
				};
			
		private:
			size_t highest_document;							///< The higest document number seen in this postings list (counting from 1)
			size_t highest_position;							///< The higest position seen in this postings list (counting from 1)

			dynamic_array<uint32_t> document_ids;			///< Array holding the document IDs
			dynamic_array<uint16_t> term_frequencies;		///< Array holding the term frequencies
			dynamic_array<uint32_t> positions;				///< Array holding the term positions

			/*
				CLASS INDEX_POSTINGS::ITERATOR
				------------------------------
			*/
			/*!
				@brief C++ iterator for iterating over an index_postings object.
				@details See http://www.cprogramming.com/c++11/c++11-ranged-for-loop.html for details on how to write a C++11 iterator.
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
					dynamic_array<uint32_t>::iterator document;			///< The iterator for the documents (1 per document).
					dynamic_array<uint16_t>::iterator frequency;			///< The iterator for the frequencies (1 per document).
					dynamic_array<uint32_t>::iterator position;			///< The iterator for the word positions (frequency times per document).
					dynamic_array<uint16_t>::iterator frequency_end;	///< Use to know when we've walked past the end of the pstings.
					uint32_t frequencies_remaining;							///< The numner of word positions that have not yet been returned for this document.

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
						document(start == START ? parent.document_ids.begin() : parent.document_ids.end()),
						frequency(start == START ? parent.term_frequencies.begin() : parent.term_frequencies.end()),
						position(start == START ? parent.positions.begin() : parent.positions.end()),
						frequency_end(parent.term_frequencies.end()),
						frequencies_remaining(frequency != parent.term_frequencies.end() ? *frequency : 0)
						{
						/* Nothing */
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
						if (other.position != position)
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
					const posting operator*() const
						{
						return posting(*document, *frequency, *position);
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
						frequencies_remaining--;
						if (frequencies_remaining <= 0)
							{
							++document;
							++frequency;
							frequencies_remaining = frequency != frequency_end ? *frequency : 0;
							}
						++position;
						return *this;
						}
				};

			/*
				CLASS INDEX_POSTINGS::TF_ITERATOR
				---------------------------------
			*/
			/*!
				@brief C++ iterator for iterating over the <document_id, term_frequency> paira in an index_postings object.
			*/
			class tf_iterator
				{
				private:
					dynamic_array<uint32_t>::iterator document;			///< The iterator for the documents (1 per document).
					dynamic_array<uint16_t>::iterator frequency;			///< The iterator for the frequencies (1 per document).

				public:
					/*
						INDEX_POSTINGS::TF_ITERATOR::TF_ITERATOR()
						------------------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The object that this iterator is iterating over
						@param start [in] Whether this is an iterator for the START or END of the postings lists
					*/
					tf_iterator(const index_postings &parent, iterator::where start):
						document(start == iterator::START ? parent.document_ids.begin() : parent.document_ids.end()),
						frequency(start == iterator::START ? parent.term_frequencies.begin() : parent.term_frequencies.end())
						{
						/* Nothing */
						}

					/*
						INDEX_POSTINGS::TF_ITERATOR::OPERATOR!=()
						-----------------------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param other [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/

					bool operator!=(const tf_iterator &other) const
						{
						if (other.frequency != frequency)
							return true;
						else
							return false;
						}

					/*
						INDEX_POSTINGS::TF_ITERATOR::OPERATOR*()
						----------------------------------------
					*/
					/*!
						@brief Return a copy of the element pointed to by this iterator.
					*/
					const posting operator*() const
						{
						return posting(*document, *frequency, 0);
						}

					/*
						INDEX_POSTINGS::TF_ITERATOR::OPERATOR++()
						-----------------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					const tf_iterator &operator++()
						{
						++document;
						++frequency;

						return *this;
						}
				};

			/*
				CLASS INDEX_POSTINGS::TF_ITERATOR_THUNK
				---------------------------------------
			*/
			/*!
				@brief Intermediary object used to create iterators over the non-positional parts of the index
			*/
			class tf_iterator_thunk
				{
				private:
					const index_postings &parent;					///< The index_postings object we're thunking through

				public:
					/*
						INDEX_POSTINGS::TF_ITERATOR_THUNK::TF_ITERATOR_THUNK()
						------------------------------------------------------
					*/
					/*!
						@brief Constructor
						@param parent [in] The index_postings object that the tf_iterator will iterate over
					*/
					tf_iterator_thunk(const index_postings &parent) :
						parent(parent)
						{
						/* Nothing */
						}

					/*
						INDEX_POSTINGS::TF_ITERATOR_THUNK::BEGIN()
						------------------------------------------
					*/
					/*!
						@brief Return a tf_iterator pointing to the start of the postings.
						@return tf_iterator pointing to start of the postings.
					*/
					auto begin(void) const
						{
						return tf_iterator(parent, iterator::START);
						}
						
					/*
						INDEX_POSTINGS::TF_ITERATOR_THUNK::END()
						----------------------------------------
					*/
					/*!
						@brief Return an iterator pointing to the end of the postings.
						@return Iterator pointing to end of tne postings.
					*/
					auto end(void) const
						{
						return tf_iterator(parent, iterator::END);
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
				highest_position(0),															// starts at 0, counts from 1
				document_ids(memory_pool, initial_size, growth_factor),			// give the allocator to the array
				term_frequencies(memory_pool, initial_size, growth_factor),		// give the allocator to the array
				positions(memory_pool, initial_size, growth_factor)				// give the allocator to the array
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
				INDEX_POSTINGS::TF_ITERATE()
				----------------------------
			*/
			/*!
				@brief return an intermediary that can be used to iterate over the <document_id, term_frequency> pairs without positions
			*/
			const tf_iterator_thunk tf_iterate(void) const
				{
				return tf_iterator_thunk(*this);
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
					/*
						If this is the second or subseqent occurrence then just add 1 to the term frequency (and make sure it doesn't overflow).
					*/
					uint16_t &frequency = term_frequencies.back();
					if (frequency <= 0xFFFE)
						frequency++;			// cppcheck produces a false positive "Variable 'frequency' is modified but its new value is never used." - it looks like it doesn't fully understand that frequency is a reference.
					}
				else
					{
					/*
						First time we've seen this term in this document so add a new document ID and set the term frequency to 1.
					*/
					document_ids.push_back(document_id);
					highest_document = document_id;
					term_frequencies.push_back(1);
					}
				/*
					Always add a new position
				*/
				positions.push_back(position);
				highest_position = position;
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
				std::array<uint32_t, 0x10000> frequencies = {};
				size_t number_of_postings = 0;
				size_t highest_impact = 0;
				size_t lowest_impact = std::numeric_limits<size_t>::max();

				/*
					Count up the number of times each impact is seen and compute the highest and lowest impact scores
				*/
				for (const auto &posting : tf_iterate())
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
				size_t impact_value = 0;
				for (size_t which = 0; which <= 0xFFFF; which++)
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
				for (const auto &posting : tf_iterate())
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
				uint32_t previous_document_id = std::numeric_limits<uint32_t>::max();
				for (const auto &posting : *this)
					{
					if (posting.document_id != previous_document_id)
						{
						if (previous_document_id != std::numeric_limits<uint32_t>::max())
							stream << '>';
						stream << '<' << posting.document_id << ',' << posting.term_frequency << ',' << posting.position;
						previous_document_id = posting.document_id;
						}
					else
						stream << ',' << posting.position;
					}
				stream << '>';
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
				
				postings.push_back(1, 100);
				postings.push_back(1, 101);
				postings.push_back(2, 102);
				postings.push_back(2, 103);
				
				std::ostringstream result;
				
				postings.text_render(result);

				JASS_assert(strcmp(result.str().c_str(), "<1,2,100,101><2,2,102,103>") == 0);

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
