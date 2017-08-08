/*
	INDEX_POSTINGS_IMPACT.H
	-----------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Holder class for an impact ordered postings list
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <vector>

#include "allocator.h"

namespace JASS
	{
	/*
		CLASS INDEX_POSTINGS_IMPACT
		---------------------------
	*/
	/*!
		@brief Holder class for an impact ordered postings list
	*/
	class index_postings_impact
		{
		public:
			/*
				CLASS INDEX_POSTINGS_IMPACT::IMPACT
				-----------------------------------
			*/
			/*!
				@brief Each impact is represented as an impact score, and a start and end pointer.
			*/
			class impact
				{
				public:
					size_t impact_score;				///< The impact score.
					size_t *start;						///< Pointer into postings of the start of the document list for this impact score.
					size_t *finish;					///< Pointer into postings of the end of the document list for this impact score.

				public:
					/*
						INDEX_POSTINGS_IMPACT::IMPACT::BEGIN()
						--------------------------------------
					*/
					/*!
						@brief Return a pointer to the start of the document identifiers array (for use as an iterator).
						@return Pointer to first document id.
					*/
					size_t *begin(void) const
						{
						return start;
						}

					/*
						INDEX_POSTINGS_IMPACT::IMPACT::END()
						------------------------------------
					*/
					/*!
						@brief Return a pointer to the end of the document identifiers array (for use as an iterator).
						@return Pointer to the element immediately after the last document id.
					*/
					size_t *end(void) const
						{
						return finish;
						}

					/*
						INDEX_POSTINGS_IMPACT::IMPACT::SIZE()
						-------------------------------------
					*/
					/*!
						@brief Return the numner of postings with this impact score.
						@return number of postings with this impact score.
					*/
					size_t size(void) const
						{
						return static_cast<size_t>(finish - start);
						}
				};

		protected:
			allocator &memory;						///< All allocation  happens in this arena.
			size_t number_of_impacts;				///< The number of impact objects in the impacts array.
			impact *impacts;							///< List of impact pointers (the impact header).
			size_t number_of_postings;				///< The length of the pistings array measured in size_t.
			size_t *postings;							///< The list of document IDs, strung together for each postings segment.

		public:
			/*
				INDEX_POSTINGS_IMPACT::INDEX_POSTINGS_IMPACT()
				----------------------------------------------
			*/
			/*!
				@brief Constructor
				@details The postings lists are typvically stored <impact><d><d>...<d><0>,  The headers point to the list
				of <d>s (not the <impact> or the <0>.
				@param unique_impacts [in] The number of unique impact vales seen n the postings list
				@param total_postings [in] The number of integers in the potings list (including any imact scores and '0' terminators
				@param memory [in] All memory allocation happens in this arena
			*/
			index_postings_impact(size_t unique_impacts, size_t total_postings, allocator &memory):
				memory(memory),
				number_of_impacts(unique_impacts),
				impacts(static_cast<impact *>(memory.malloc(unique_impacts * sizeof(*impacts)))),
				number_of_postings(total_postings),
				postings(static_cast<size_t *>(memory.malloc(total_postings * sizeof(*postings))))
				{
				/* Nothing */
				}

			/*
				INDEX_POSTINGS_IMPACT::OPERATOR[]()
				-----------------------------------
			*/
			/*!
				@brief return a reference to the size_t at position index in the postings array.
				@details Node taht this is astrict index and not range checked.  Access out of range ill resilt in
				undefined behaviour.  The access is to elements in the array so if the postings are stored <impact><d><d>...<d><0>
				then posting[0] will be the impact score.
				@return a reference to the size_t at postng[index].
			*/
			size_t &operator[](size_t index) const
				{
				return postings[index];
				}

			/*
				INDEX_POSTINGS_IMPACT::HEADER()
				-------------------------------
			*/
			/*!
				@brief Set the value of the impact header object at the given index
				@param index [in] Which header to set
				@param score [in] The impact value for all the documents in this range
				@param postings_start [in] The start of the range if postings that share this impact score
				@param postings_end [in] The end of the range if postings that share this impact score

			*/
			void header(size_t index, size_t score, size_t *postings_start, size_t *postings_end) const
				{
				impacts[index].impact_score = score;
				impacts[index].start = postings_start;
				impacts[index].finish = postings_end;
				}

			/*
				INDEX_POSTINGS_IMPACT::IMPACT_SIZE()
				------------------------------------
			*/
			/*!
				@brief Return the number of unique impact scores in the postings list.
				@brief The number of unique impact scores.
			*/
			size_t impact_size(void) const
				{
				return number_of_impacts;
				}

			/*
				INDEX_POSTINGS_IMPACT::BEGIN()
				------------------------------
			*/
			/*!
				@brief Return a pointer to the first impact header (for use in an iterator).
				@return A pointer to the first impact header.
			*/
			impact *begin(void) const
				{
				return impacts;
				}

			/*
				INDEX_POSTINGS_IMPACT::END()
				----------------------------
			*/
			/*!
				@brief Return a pointer to one element past the end of the impact headers (for use in an iterator).
				@return A pointer to one element past the end of the impact headers.
			*/
			impact *end(void) const
				{
				return impacts + number_of_impacts;
				}

			/*
				INDEX_POSTINGS_IMPACT::UNITTEST()
				---------------------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void)
				{
				allocator_pool memory;
				index_postings_impact postings(2, 7, memory);

				/*
					Set up some postings
				*/
				postings[0] = 255;			// impact
				postings[1] = 10;				// docid
				postings[2] = 0;				// termiator
				postings[3] = 128;			// impact
				postings[4] = 2;				// docid
				postings[5] = 5;				// docis
				postings[6] = 0;				// terminator

				/*
					Set up the headers
				*/
				postings.header(0, 255, &postings[1], &postings[2]);
				postings.header(1, 128, &postings[4], &postings[6]);

				/*
					Check the data got into the right places.
				*/
				const size_t answer[] = {255, 10, 0, 128, 2, 5, 0};
				JASS_assert(memcmp(&postings[0], answer, sizeof(answer)) == 0);

				JASS_assert(postings.impacts[0].impact_score == 255);
				JASS_assert(postings.impacts[0].start == &postings.postings[1]);
				JASS_assert(postings.impacts[0].finish == &postings.postings[2]);

				JASS_assert(postings.impacts[1].impact_score == 128);
				JASS_assert(postings.impacts[1].start == &postings.postings[4]);
				JASS_assert(postings.impacts[1].finish == &postings.postings[6]);

				JASS_assert(postings.number_of_postings == 7);
				JASS_assert(postings.impact_size() == 2);

				/*
					Check iteration
				*/
				std::string serialised_answer = "255:10 \n128:2 5 \n";
				std::stringstream output;
				for (const auto &header : postings)
					{
					output << header.impact_score << ":";
					for (const auto &document_id : header)
						output << document_id << " ";
					output << '\n';
					}
				JASS_assert(output.str() == serialised_answer);

				puts("index_postings_impact::PASSED");
				}
		};
	}
