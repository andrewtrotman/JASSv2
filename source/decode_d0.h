/*
	DECODE_D0
	---------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Decode D0 encoded integer sequences.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <vector>

#include "query.h"
#include "compress_integer_none.h"

namespace JASS
	{
	/*
		CLASS DECODER_D0
		----------------
	*/
	/*!
		@brief Decode for D0 encoded integer sequences
		@details Many of the integer encoders compute differences (d-gaps, or deltas) between consequtive
		integers before encoding (so-called D1).  This makes the integers smaller and thus easier to encode.  Some, however
		do not (D0), and others compute the difference between integers 4 away from each other (D4) so that SIMD instructions
		can be used to reconstruct the integer sequence.  This class decodes and adds to the accumulators D0 (i.e. not)
		delta encoded sequences.
	*/
	class decoder_d0
		{
		private:
			size_t integers;													///< The number of integers in the decompress buffer.
			std::vector<uint32_t> decompress_buffer;					///< The delta-encoded decopressed integer sequence.

		private:
			/*
				DECODER_D0::BEGIN()
				-------------------
			*/
			/*!
				@brief Return an iterator pointing to the start of the sequence (using this iterator will D0 decode each integer in sequence).
				@return Iterator pointing to start of sequence.
			*/
			auto begin() const
				{
				return decompress_buffer.data();
				}

			/*
				DECODER_D0::END()
				-----------------
			*/
			/*!
				@brief Return an iterator pointing to the end of the sequence (using this iterator will D0 decode each integer in sequence).
				@return Iterator pointing to end of sequence.
			*/
			auto end() const
				{
				return decompress_buffer.data() + integers;
				}

		public:
			/*
				DECODER_D0::DECODER_D0()
				------------------------
			*/
			/*!
				@brief Constructor
			*/
			decoder_d0() = delete;

			/*
				DECODER_D0::DECODER_D0()
				------------------------
			*/
			/*!
				@brief Constructor
				@param max_integers [in] The maximum number of integers that will ever need to be decoded using this object (i.e. the number of documents in the collection + overflow).
			*/
			explicit decoder_d0(size_t max_integers) :
				integers(0),
				decompress_buffer(max_integers, 0)
				{
				/* Nothing */
				}

			/*
				DECODER_D0::DECODE()
				--------------------
			*/
			/*!
				@brief Given the integer decoder, the number of integes to decode, and the compressed sequence, decompress (but do not process).
				@param decoder [in] The codex to use to decompress into the D0 sequence.
				@param integers [in] The number of integers that are compressed.
				@param compressed [in] The compressed sequence.
				@param compressed_size [in] The length of the compressed sequence.
			*/
			void decode(compress_integer &decoder, size_t integers, const void *compressed, size_t compressed_size)
				{
				decoder.decode(decompress_buffer.data(), integers, compressed, compressed_size);
				this->integers = integers;
				}

			/*
				DECODER_D0::PROCESS()
				---------------------
				We put the processing code here so that a decoder can work in parallel - if needed.
			*/
			/*!
				@brief Process the integer sequence as a D0 impact-ordered sequence into the accumulators
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
				DECODER_D0::UNITTEST()
				----------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				std::vector<uint32_t>integer_sequence = {2, 3, 5, 7, 11, 13, 17, 19};
				std::vector<std::string>primary_keys = {"zero" "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve", "thirteen", "fourteen", "fifteen", "sixteen", "seventeen", "eighteen", "nineteen", "twenty"};
				compress_integer_none identity;
				query<uint16_t, 100, 100> jass_query(primary_keys, 20, 5);
				std::ostringstream result;

				decoder_d0 decoder(20);
				decoder.decode(identity, integer_sequence.size(), integer_sequence.data(), sizeof(integer_sequence[0]) * integer_sequence.size());
				decoder.process(1, jass_query);
				for (const auto &answer : jass_query)
					result << answer.document_id << " ";

				JASS_assert(result.str() == "19 17 13 11 7 ");
				puts("decoder_d0::PASSED");
				}
		};
	}
