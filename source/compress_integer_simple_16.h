/*
	COMPRESS_INTEGER_SIMPLE_16.H
	----------------------------
	Copyright (c) 2014-2017 Blake Burgess and Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	This code is from ATIRE, where it was written by Blake Burgess.
*/
/*!
	@file
	@brief Simple-16 Compression.
	@author Blake Burgess, Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_SIMPLE_16
		--------------------------------
	*/
	/*!
		@brief Simple-16 integer compression
		@details Simple-16 is an extension to Simple-9 that uses all 16 selectors (rather than just 9) for encoding
		the payloads.  This resulrs in a more effective encoding that performs faster than Simple-9.  This is because
		fewer reads ar eneeded and hence its faster.  Note that, because there are only 28 bits in a payload, the maximum 
		integer that can be encoded with simple-9 is (2^29) - 1 = 536,870,911.  This is less than the number of documens in
		a large collection (such as ClueWeb).
		
		The encodings are:
			28 * 1-bit
			7 * 2-bits and 14 * 1-bit
			7 * 1-bit and 7 * 2-bits and 7 * 1-bit
			14 * 1-bit and 7 * 2-bits
			14 * 2-bits
			1 * 4-bit and 8 * 3-bits
			1 * 3-bits and 4 * 4-bits and 3 * 3-bits
			7 * 4-bits
			4 * 5 bits and 2 * 4 bits
			2 * 4-bits and 4 * 5-bits
			3 * 6-bits and 2 * 5-bits
			2 * 5-bits and 3 * 6 bits
			4 * 7-bits
			1 * 10-bits and 2 * 9 bits
			2 * 14-bits
			1 * 28-bits

		See:
			Zhang J, Long X, Suel T. (2008) Performance of compressed inverted list caching in search engines. Proceeedings of 17th Conference on the World Wide Web, pp 387-396
			Yan H, Ding S, Suel T (2009) Inverted index compression and query processing with optimized document ordering. roceeedings of 18th Conference on the World Wide Web, 401-410
	*/
	class compress_integer_simple_16 : public compress_integer
		{
		protected:
			static const size_t ints_packed_table[];			///< Number of integers packed into a word, given its mask type
			static const size_t can_pack_table[];				///< Bitmask map for valid masks at an offset (column) for some num_bits_needed (row)
			static const size_t row_for_bits_needed[];			///< Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table
			static const size_t invalid_masks_for_offset[];		///< We AND out masks for offsets where we don't know if we can fully pack for that offset
			static const size_t simple16_shift_table[];			///< Number of bits to shift across when packing - is sum of prior packed ints (see above)

		public:
			/*
				COMPRESS_INTEGER_SIMPLE_16::COMPRESS_INTEGER_SIMPLE_16()
				--------------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_integer_simple_16()
				{
				}

			/*
				COMPRESS_INTEGER_SIMPLE_16::~COMPRESS_INTEGER_SIMPLE_16()
				--------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_simple_16()
				{
				}

			/*
				COMPRESS_INTEGER_SIMPLE_16::ENCODE()
				------------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);
			
			/*
				COMPRESS_INTEGER_SIMPLE_16::DECODE()
				------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length);
			
			/*
				COMPRESS_INTEGER_SIMPLE_16::UNITTEST()
				--------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);

		};
	}
