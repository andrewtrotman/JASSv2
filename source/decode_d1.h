/*
	DECODE_D1
	---------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Decode D1 encoded integer sequences.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "query.h"
#include "compress_integer_none.h"

namespace JASS
	{
	/*
		CLASS DECODER_D1
		----------------
	*/
	/*!
		@brief Decode for D1 encoded integer sequences
		@details Many of the integer encoders compute differences (d-gaps, or deltas) between consequtive
		integers before encoding (so-called D1).  This makes the integers smaller and thus easier to encode.
		This class decodes and adds to the accumulators D1 delta encoded sequences.
	*/
	class decoder_d1
		{
		private:
			size_t integers;													///< The number of integers in the decompress buffer.
			std::vector<uint32_t> decompress_buffer;					///< The delta-encoded decopressed integer sequence.

		private:
			/*
				CLASS DECODER_D1::ITERATOR
				--------------------------
			*/
			/*!
				@brief Iterator over D1 encoded integer sequences reconstituting the original sequence (i.e. doing the cumulative add)
			*/
			class iterator
				{
				private:
					uint32_t cumulative;					///< The cumulative sum so far
					const uint32_t *current;			///< Pointer to the current D1 encoded integer

				public:
					/*
						DECODER_D1::ITERATOR::ITERATOR()
						--------------------------------
					*/
					/*!
						@brief Constructor
						@param array [in] The vector to use in the iterator
						@param offset [in] The offset within the vector to use in the iterator
					*/
					iterator(const std::vector<uint32_t> &array, size_t offset) :
						cumulative(0),
						current(array.data() + offset)
						{
						/* Nothing */
						}

					/*
						DECODER_D1::ITERATOR::OPERATOR*()
						---------------------------------
					*/
					/*!
						@brief Return a reference to the element pointed to by this iterator.
					*/
					uint32_t operator*(void)
						{
						cumulative += *current;
						return cumulative;
						}

					/*
						DECODER_D1::ITERATOR::OPERATOR!=()
						----------------------------------
					*/
					/*!
						@brief Compare two iterator objects for non-equality.
						@param another [in] The iterator object to compare to.
						@return true if they differ, else false.
					*/
					bool operator!=(iterator &another)
						{
						return current != another.current;
						}

					/*
						DECODER_D1::ITERATOR::OPERATOR++()
						----------------------------------
					*/
					/*!
						@brief Increment this iterator.
					*/
					iterator &operator++(void)
						{
						current++;
						return *this;
						}
				};

		private:
			/*
				DECODER_D1::BEGIN()
				-------------------
			*/
			/*!
				@brief Return an iterator pointing to the start of the sequence (using this iterator will D1 decode each integer in sequence).
				@return Iterator pointing to start of sequence.
			*/
			iterator begin() const
				{
				return iterator(decompress_buffer, 0);
				}

			/*
				DECODER_D1::END()
				-----------------
			*/
			/*!
				@brief Return an iterator pointing to the end of the sequence (using this iterator will D1 decode each integer in sequence).
				@return Iterator pointing to end of sequence.
			*/
			iterator end() const
				{
				return iterator(decompress_buffer, integers);
				}

		public:
			/*
				DECODER_D1::DECODER_D1()
				------------------------
			*/
			/*!
				@brief Constructor
			*/
			decoder_d1() = delete;

			/*
				DECODER_D1::DECODER_D1()
				------------------------
			*/
			/*!
				@brief Constructor
				@param max_integers [in] The maximum number of integers that will ever need to be decoded using this object (i.e. the number of documents in the collection + overflow).
			*/
			explicit decoder_d1(size_t max_integers) :
				integers(0),
				decompress_buffer(max_integers, 0)
				{
				/* Nothing */
				}

			/*
				DECODER_D1::DECODE()
				--------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param decoder [in] The codex to use to decompress into the D1 sequence.
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			void decode(JASS::compress_integer &decoder, size_t integers, const void *compressed, size_t compressed_size)
				{
				decoder.decode(decompress_buffer.data(), integers, compressed, compressed_size);
				this->integers = integers;
				}

			/*
				DECODER_D1::PROCESS()
				---------------------
				We put the processing code here so that a decoder can work in parallel - if needed.
			*/
			/*!
				@brief Process the integer sequence as a D1 impact-ordered sequence into the accumulators
				@param impact [in] The impact score to add for each document id in the list.
				@param accumulators [in] The accumulators to add to
			*/
			template <typename QUERY_T>
			void process(uint16_t impact, QUERY_T &accumulators) const
				{
				for (auto document : *this)
					accumulators.add_rsv(document, impact);
				}

			/*
				DECODER_D1::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<uint32_t>integer_sequence = {2, 1, 2, 2, 4, 2, 4, 2};
				std::vector<std::string>primary_keys = { "zero" "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty" };
				compress_integer_none identity;
				query<uint16_t, 100, 100> query(primary_keys, 20, 5);
				std::ostringstream result;

				decoder_d1 decoder(20);
				decoder.decode(identity, integer_sequence.size(), integer_sequence.data(), sizeof(integer_sequence[0]) * integer_sequence.size());
				decoder.process(1, query);
				for (const auto &answer : query)
					result << answer.document_id << " ";

				JASS_assert(result.str() == "19 17 13 11 7 ");
				puts("decoder_d1::PASSED");
				}
		};
	}
