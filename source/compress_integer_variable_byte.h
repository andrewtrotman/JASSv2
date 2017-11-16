/*
	COMPRESS_INTEGER_VARIABLE_BYTE.H
	--------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Variable byte compression for integer sequences.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_VARIABLE_BYTE
		------------------------------------
	*/
	/*!
		@brief Variable byte compression for integer sequences.
		@details Variable byte compression is a whole suite of different techniques, for details see:
		A. Trotman (2014), Compression, SIMD, and Postings Lists. In Proceedings of the 2014 Australasian Document Computing Symposium (ADCS 2014), Pages 50-58. DOI=http://dx.doi.org/10.1145/2682862.2682870
		This particular version uses a stop-bit in the high bit of the last byte of the encoded integer, 
		stores the integer big-endian (high byte first), and uses loop unwinding for decoding efficiency.
		The encoding is straight forward.  An integer is broken into 7-bit chunks with the top bit of each
		chunk being 0, except the last byte which has a 1 in the top bit.  So, the integer 1905 (0x771)
		is the binary sequence 011101110001, which broken into 7-bit chunks is 0001110 1110001.  These then
		get the high bits added, 0 for all except the last byte, [0]0001110 [1]1110001, then write out
		the byte sequence high byte first 0x0E 0xF1.
		This implementation works with 32-bit and 64-bit integers.  To encode 64-bit integers ensure 
		\#define JASS_COMPRESS_INTEGER_BITS_PER_INTEGER 64
		is set at compile time.
	*/
	class compress_integer_variable_byte : public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::COMPRESS_INTEGER_VARIABLE_BYTE()
				----------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_variable_byte()
				{
				/* Nothing */
				}
			
			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::~COMPRESS_INTEGER_VARIABLE_BYTE()
				-----------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			virtual ~compress_integer_variable_byte()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::ENCODE()
				----------------------------------------
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
				COMPRESS_INTEGER_VARIABLE_BYTE::DECODE()
				----------------------------------------
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
				COMPRESS_INTEGER_VARIABLE_BYTE::BYTES_NEEDED_FOR()
				--------------------------------------------------
			*/
			/*!
				@brief Return the number of bytes necessary to encode the integer value.
				@param value [in] The value whose encoded size is being computed
				@return the numner of bytes needed to store the enoding of value.
			*/
			static inline size_t bytes_needed_for(integer value)
				{
				/*
					The size can be computed by compairing to a bunch of constants.
				*/
				if (value < ((uint64_t)1 << 7))
					return 1;
				else if (value < ((uint64_t)1 << 14))
					return 2;
				else if (value < ((uint64_t)1 << 21))
					return 3;
				else if (value < ((uint64_t)1 << 28))
					return 4;
#if JASS_COMPRESS_INTEGER_BITS_PER_INTEGER == 32
				else
					return 5;
#else
				else if (value < ((uint64_t)1 << 35))
					return 5;
				else if (value < ((uint64_t)1 << 42))
					return 6;
				else if (value < ((uint64_t)1 << 49))
					return 7;
				else if (value < ((uint64_t)1 << 56))
					return 8;
				else if (value < ((uint64_t)1 << 63))
					return 9;
				else
					return 10;
#endif
				}

			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::COMPRESS_INTO()
				-----------------------------------------------
			*/
			/*!
				@brief Encode the given integer placing the encoding into destination (whose size is not validated).
				@param destination [out] The buffer to write into.
				@param value [in] The value to encode.
			*/
			static inline void compress_into(uint8_t *destination, integer value)
				{
				/*
					Work out how many bytes it'll take to encode
				*/
				if (value < ((uint64_t)1 << 7))
					goto one;
				else if (value < ((uint64_t)1 << 14))
					goto two;
				else if (value < ((uint64_t)1 << 21))
					goto three;
				else if (value < ((uint64_t)1 << 28))
					goto four;
#if JASS_COMPRESS_INTEGER_BITS_PER_INTEGER == 32
				goto five;
#else
				else if (value < ((uint64_t)1 << 35))
					goto five;
				else if (value < ((uint64_t)1 << 42))
					goto six;
				else if (value < ((uint64_t)1 << 49))
					goto seven;
				else if (value < ((uint64_t)1 << 56))
					goto eight;
				else if (value < ((uint64_t)1 << 63))
					goto nine;
				else
					goto ten;
#endif

				/*
					Now encode byte at a time with fall-through
				*/
#if JASS_COMPRESS_INTEGER_BITS_PER_INTEGER == 64
				ten:
					*destination++ = (value >> 63) & 0x7F;
				nine:
					*destination++ = (value >> 56) & 0x7F;
				eight:
					*destination++ = (value >> 49) & 0x7F;
				seven:
					*destination++ = (value >> 42) & 0x7F;
				six:
					*destination++ = (value >> 35) & 0x7F;
#endif
				five:
					*destination++ = (value >> 28) & 0x7F;
				four:
					*destination++ = (value >> 21) & 0x7F;
				three:
					*destination++ = (value >> 14) & 0x7F;
				two:
					*destination++ = (value >> 7) & 0x7F;
				one:
					*destination++ = (value & 0x7F) | 0x80;
				}
				
			/*
				COMPRESS_INTEGER_VARIABLE_BYTE::UNITTEST()
				------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;


}
