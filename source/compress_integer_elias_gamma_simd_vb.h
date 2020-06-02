/*
	COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB.H
	--------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Pack 32-bit integers into 512-bit SIMD words using elias gamma encoding - with VByte on short lists
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#include "forceinline.h"
#include "compress_integer_variable_byte.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB
		------------------------------------------
	*/
	/*!
		@brief Pack 32-bit integers into 512-bit SIMD words using elias gamma with vbyte for short lists
		@details see:
		A. Trotman, K. Lilly (2018), Elias Revisited: Group Elias SIMD Coding,
		Proceedings of The 23rd Australasian Document Computing Symposium (ADCS 2018)
	*/
	class compress_integer_elias_gamma_simd_vb: public compress_integer_variable_byte
		{
		protected:
			static const uint32_t mask_set[33][16];

		protected:
			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::COMPUTE_SELECTOR()
				--------------------------------------------------------
			*/
			/*!
				@brief Computer the selector to use for this encoding.
				@param encodings [in] The width (in bits) of the (at most) 32 integers.  The first integer is at encodngs[0].  leading 0s are used to mark fewer than 32 integers.
				@return The selector encoded as an integer
			*/
			static uint32_t compute_selector(const uint8_t *encodings);

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::FIND_FIRST_SET_BIT()
				----------------------------------------------------------
			*/
			/*!
				@brief return the position of the least significant set bit (using a single machine code instruction)
				@param [in] value the integer to check.
				@return The position of the lowest set bit (or 0 if no bits are set)
			*/
			static forceinline uint64_t find_first_set_bit(uint64_t value)
				{
				return _tzcnt_u64(value) + 1;
				}

		public:
			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::ENCODE()
				----------------------------------------------
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
				COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::DECODE()
				----------------------------------------------
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
				COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::DECODE_WITH_WRITER()
				----------------------------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex, calling add_rsv for each SIMD register
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
#ifdef SIMD_JASS
			virtual void decode_with_writer(size_t integers_to_decode, const void *source_as_void, size_t source_length);
#endif

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::UNITTEST()
				------------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}


