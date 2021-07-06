
//#define ADD_SENTINALS 1

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

#include "maths.h"
#include "posting.h"
#include "dynamic_array.h"
#include "compress_integer.h"
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
		@brief Non-thread-safe object that accumulates a single postings list during indexing.
	*/
	class index_postings
		{
		private:
			static constexpr size_t initial_size = 16;		///< Initially allocate space for 4 elements
			static constexpr double growth_factor = 1.5;		///< Grow dynamic arrays by a factor of 1.5

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
					compress_integer::integer document_id;						///< Internal document identifier
					index_postings_impact::impact_type term_frequency;		///< Number of times the term occurs in this document

				public:
					/*
						INDEX_POSTINGS::POSTING::POSTING()
						----------------------------------
					*/
					/*!
						@brief Constructor.
						@param document_id [in] the document ID to use
						@param term_frequency [in] the term frequency to use
					*/
					posting(uint32_t document_id, uint32_t term_frequency) :
						document_id(document_id),
						term_frequency(term_frequency)
						{
						/* Nothing */
						}
				};

		private:
			compress_integer::integer highest_document;									///< The higest document number seen in this postings list (counting from 1)
			dynamic_array<uint8_t> document_ids;											///< Array holding the docids (variable byte encoded)
			dynamic_array<index_postings_impact::impact_type> term_frequencies;	///< Array holding the term frequencies (as integers)

		public:
			index_postings() = delete;

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
				document_ids(memory_pool, initial_size, growth_factor),					// give the allocator to the array
				term_frequencies(memory_pool, initial_size, growth_factor)					// give the allocator to the array
				{
				/* Nothing */
				}

			/*
				INDEX_POSTINGS::PUSH_BACK()
				---------------------------
			*/
			/*!
				@brief Add to the end of the postings list for this term a term frequency of the given amount.
				@param document_id [in] The document whose term count is to be incremented.
				@param amount [in] The amount to add to the score.  default = 1.
				@details The amount is generally not useful unless pre-computed term frequencies (or impact scores) are known in advance.  This might
				happen if a forward index is being inverted (i.e. term:count values are known).

			*/
			virtual void push_back(JASS::compress_integer::integer document_id, index_postings_impact::impact_type amount = 1)
				{
				if (document_id == highest_document)
					{
					/*
						If this is the second or subseqent occurrence then just add 1 to the term frequency (and make sure it doesn't overflow).
					*/
					index_postings_impact::impact_type &frequency = term_frequencies.back();


					if (index_postings_impact::largest_impact - frequency > amount)			// that is, without overflow: if (frequency + amount < index_postings_impact::largest_impact)
						frequency += amount;			// cppcheck produces a false positive "Variable 'frequency' is modified but its new value is never used." - it looks like it doesn't fully understand that frequency is a reference.
					else
						frequency = index_postings_impact::largest_impact;
					}
				else
					{
					/*
						First time we've seen this term in this document so add a new document ID and set the term frequency to 1.
					*/
					uint8_t space[10];				// worst case for a 64-bit integer (70 bits / 7 bits per byte = 10 bytes)
					uint8_t *ending = space;		// write into here;

					/*
						Compress into the temporary buffer then copy into the postings
					*/
					compress_integer_variable_byte::compress_into(ending, document_id - highest_document);

					for (uint8_t *byte = space; byte < ending; byte++)
						document_ids.push_back(*byte);
					term_frequencies.push_back(amount);

					highest_document = document_id;
					}
				}

			/*
				INDEX_POSTINGS::PUSH_BACK()
				---------------------------
			*/
			/*!
				@brief Add a set of postings to the end of the postings list (warning)
				@details This method assumes the caller is either adding postings to an index one at a time or
				is adding postings to an index block at a time.  WARNING: it with JASS_assert() if there are
				already postings in the index from the document-at-a-time adding approach.

				@param data [in] The postings list to use.  Document ids are assumed to be D1-encoded.
			*/
			virtual void push_back(const std::vector<JASS::posting> &data)
				{
				/*
					Make sure the user isn't mixing push_back(docid) and push_back(postings_lists) calls.
					But note that it does set highest_document correctly at the end.
				*/
				JASS_assert(highest_document == 0);

				size_t postings_list_length = data.size();
				for (size_t current = 0; current < postings_list_length; current++)
					{
					uint8_t space[10];					// worst case for a 64-bit integer (70 bits / 7 bits per byte = 10 bytes)
					uint8_t *ending = space;			// write into here;
					
					/*
						Compress into the temporary buffer then copy into the postings
					*/
					highest_document += data[current].docid;
					compress_integer_variable_byte::compress_into(ending, data[current].docid);
					for (uint8_t *byte = space; byte < ending; byte++)
						document_ids.push_back(*byte);

					/*
						Set the term frequency
					*/
					decltype(index_postings_impact::largest_impact) frequency = data[current].term_frequency;
					term_frequencies.push_back(JASS::maths::minimum(frequency, index_postings_impact::largest_impact));
					}
				}

			/*
				INDEX_POSTINGS::LINEARIZE()
				---------------------------
			*/
			/*!
				@brief Turn the internal format used to accumulate postings into a docid and term-frequencies array.
				@param temporary [in] Buffer used as scratch space.
				@param temporary_size [in] Length, in bytes, of temporary.
				@param ids [out] Buffer to store the document ids.
				@param frequencies [out] Buffer to store the term frequencies.
				@param id_and_frequencies_length [in] The length of the id and frequencies buffers.

				@return Returns the document frequency of this term, or 0 on failure.
			*/
			compress_integer::integer linearize(uint8_t *temporary, size_t temporary_size, compress_integer::integer *ids, index_postings_impact::impact_type *frequencies, size_t id_and_frequencies_length) const
				{
				/*
					Linearize the term frequencies
				*/
				size_t document_frequency = term_frequencies.serialise(frequencies, id_and_frequencies_length);

				if (document_frequency > id_and_frequencies_length)
					return 0;

				/*
					Linearize the document ids, which will still be variable byte encoded
				*/
				auto needs = document_ids.serialise(temporary, temporary_size);
				if (needs > temporary_size)
					return 0;

				/*
					Decompress the document ids
				*/
				compress_integer_variable_byte::static_decode(ids, document_frequency, temporary, needs);

				/*
					Decode the deltas
				*/
				compress_integer::integer sum = 0;
				auto end = ids + document_frequency;
				for (auto current = ids; current < end; current++)
					{
					sum += *current;
					*current = sum;
					}

				/*
					And return the document frequency
				*/
				return static_cast<compress_integer::integer>(document_frequency);
				}

			/*
				INDEX_POSTINGS::IMPACT_ORDER()
				------------------------------
			*/
			/*!
				@brief Return the postings list impact ordered postings list with impact headers.
				@param documents_in_collection [in] The number of documents in the collection,
				@param postings_list [in / out] The constructed impact ordered postings list.
			*/
			void impact_order(size_t documents_in_collection, index_postings_impact &postings_list) const
				{
				auto document_frequency = linearize(postings_list.temporary, postings_list.temporary_size, postings_list.document_ids, postings_list.term_frequencies, postings_list.number_of_postings);
				impact_order(documents_in_collection, postings_list, document_frequency, postings_list.document_ids, postings_list.term_frequencies);
				}

			/*
				INDEX_POSTINGS::IMPACT_ORDER()
				------------------------------
			*/
			/*!
				@brief Return the postings list impact ordered postings list with impact headers.
				@param documents_in_collection [in] The number of documents in the collection,
				@param postings_list [out] The constructed impact ordered postings list.
				@param document_frequency [in] the document frequency of this term (the length of id_list and tf_list).
				@param document_ids [in] The list of document ids.
				@param term_frequencies [in] The list of term frequencies.
			*/
			void impact_order(size_t documents_in_collection, index_postings_impact &postings_list, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies) const
				{
#ifdef ADD_SENTINALS
#define SENTINAL (documents_in_collection + 1)
std::ostream &operator<<(std::ostream &stream, const index_postings &data);
//if (document_frequency < 10)
//	std::cout << "DocOrder:" << *this << "\n";
#endif
				std::array<compress_integer::integer, index_postings_impact::largest_impact + 1> frequencies = {};			// +1 because it counts from 0.
//				size_t number_of_postings = 0;
				index_postings_impact::impact_type highest_impact = 0;
				index_postings_impact::impact_type lowest_impact = (std::numeric_limits<decltype(lowest_impact)>::max)();

				/*
					Count up the number of times each impact is seen and compute the highest and lowest impact scores
				*/
				index_postings_impact::impact_type *end = term_frequencies + document_frequency;
				for (index_postings_impact::impact_type *current_tf = term_frequencies; current_tf < end; current_tf++)
					{
					/*
						Calculate the maximum and minimum impact scores seen
					*/
					if (*current_tf > highest_impact)
						highest_impact = *current_tf;
					if (*current_tf < lowest_impact)
						lowest_impact = *current_tf;

					/*
						Count the number of times each frequency is seen
					*/
					frequencies[*current_tf]++;
//					number_of_postings++;
					}

				/*
					Count the number of unique impacts
				*/
				size_t number_of_impacts = 0;
				for (size_t which = lowest_impact; which <= highest_impact; which++)
					if (frequencies[which] != 0)
						{
						number_of_impacts++;
#ifdef ADD_SENTINALS
						frequencies[which]++;			// Include room for the sentinal
#endif
						}
				postings_list.set_impact_count(number_of_impacts);

				/*
					Put the headers into place and turn the frequencies into pointers
				*/
				uint32_t cumulative = 0;
				uint32_t current_impact = 0;
				index_postings_impact::impact_type impact_value = lowest_impact;
				for (size_t which = lowest_impact; which <= highest_impact; which++)
					{
					uint32_t *times = &frequencies[which];
					if (*times != 0)
						{
						auto prior = cumulative;
						
						/*
							Put the header in place.
						*/
						postings_list.header(current_impact, impact_value, &(postings_list.get_postings()[prior]), &(postings_list.get_postings()[prior + *times]));

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
				compress_integer::integer *current_id = document_ids;
				for (index_postings_impact::impact_type *current_tf = term_frequencies; current_tf < end; current_tf++)
					{
					postings_list.get_postings()[frequencies[*current_tf]] = *current_id;
					frequencies[*current_tf]++;
					current_id++;
					}
#ifdef ADD_SENTINALS
				/*
					Put the sentinals in place.
				*/
				for (size_t which = lowest_impact; which <= highest_impact; which++)
					if (frequencies[which] != 0)
						postings_list.get_postings()[frequencies[which]] = SENTINAL;

//if (document_frequency < 10)
//	std::cout << "ImpOrder:" << postings_list << "\n";
#endif
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
				size_t document_frequency = term_frequencies.serialise(nullptr, 0);

				/*
					Serialise the postings
				*/
				auto id_list = std::make_unique<compress_integer::integer []>(document_frequency);
				auto tf_list = std::make_unique<index_postings_impact::impact_type []>(document_frequency);
				size_t temporary_size = document_frequency * 10;		 // worst case is that each integer is encoded in 10 bytes and so the linear buffer is 10 times the number of document frequencies
				auto temporary = std::make_unique<uint8_t []>(temporary_size);

				linearize(temporary.get(), temporary_size, id_list.get(), tf_list.get(), document_frequency);

				/*
					write out the postings
				*/
				auto end = id_list.get() + document_frequency;
				auto current_tf = tf_list.get();
				for (compress_integer::integer *current_id = id_list.get(); current_id < end; current_id++, current_tf++)
					stream << '<' << *current_id << ',' << (size_t)*current_tf << '>';
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
