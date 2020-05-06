/*
	COMPRESS_INTEGER_SIMPLE_8B.H
	----------------------------
	Copyright (c) 2014-2017 Blake Burgess and Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	This code is from ATIRE, where it was written by Blake Burgess.
*/
/*!
	@file
	@brief Simple-8b Compression.
	@author Blake Burgess, Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_SIMPLE_8B
		--------------------------------
	*/
	/*!
		@brief Simple-8b integer compression
		@details Simple-8b is a 64-bit extension to Simple-9 compression.  It breaks a 64-bit integer
		into a 4-bit selector and 60-bi6 payload (and thus is not limited to 28-bit integers (see Simple-9)
		
		The encodings are listed below.  Note that the first 2 encodings are for valuw 1 not for 1-bit integers.
			240 * 1
			120 * 1
			60 * 1-bit
			30 * 2-bit
			20 * 3-bit
			15 * 4-bit
			12 * 5-bit
			10 * 6-bit
			8 * 7-bit
			7 * 8-bit
			6 * 10-bit
			5 * 12-bit
			4 * 15-bit
			3 * 16-bit
			2 * 30-bit
			1 * 60-bit

		See:
			V. Anh, A. Moffat (2010), Index compression using 64-bit words, Software Practive & Experience, pp 131-147
	*/
	class compress_integer_simple_8b : public compress_integer
		{
		protected:
			static const size_t ints_packed_table[];			///< Number of integers packed into a 64-bit word, given its mask type
			static const size_t can_pack_table[];				///< Bitmask map for valid masks at an offset (column) for some num_bits_needed (row)
			static const size_t row_for_bits_needed[];			///< Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table.
			static const size_t invalid_masks_for_offset[];		///< We AND out masks for offsets where we don't know if we can fully pack for that offset
			static const size_t simple8b_shift_table[];			///< Number of bits to shift across when packing - is sum of prior packed ints

		public:
			/*
				COMPRESS_INTEGER_SIMPLE_8B::COMPRESS_INTEGER_SIMPLE_8B()
				--------------------------------------------------------
			*/
			/*!
				@brief Consructor
			*/
			compress_integer_simple_8b()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_SIMPLE_8B::~COMPRESS_INTEGER_SIMPLE_8B()
				--------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_simple_8b()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_SIMPLE_8B::ENCODE()
				-----------------------------------
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
				COMPRESS_INTEGER_SIMPLE_8B::DECODE()
				-----------------------------------
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
				COMPRESS_INTEGER_SIMPLE_8B::UNITTEST()
				-------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
