/*
	COMPRESS_ELIAS_GAMMA.H
	----------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
*/
/*!
	@file
	@brief Elias gamma encoding using bit-by-bit encoding and decoding (slow)
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include "maths.h"
#include "compress_integer.h"
#include "bitstream.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE
		------------------------------------------
	*/
	/*!
		@brief Elias gamma encoding using bit-by-bit encoding and decoding (slow)
	*/
	class compress_integer_elias_gamma_bitwise : public compress_integer
		{
		protected:
			JASS::bitstream bitstream;			///< The bit patern that is the Elias gamma encoding

		protected:
			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::ENCODE()
				----------------------------------------------
			*/
			/*!
				@brief encode (and push) one integer
				@param val [in] The integer to encode
			*/
			inline void encode(integer val)
				{
				uint32_t exp = maths::floor_log2(val);

				bitstream.push_zeros(exp);
				bitstream.push_bits(val, exp + 1);
				}

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::DECODE()
				----------------------------------------------
			*/
			/*!
				@brief Decode (and pull) one integer from the stream
				@return The next integer in the stream
			*/
			inline integer decode(void)
				{
				uint32_t exp = 0;

				while (bitstream.get_bit() == 0)
					exp++;

				return (integer)((1ULL << exp) | bitstream.get_bits(exp));
				}

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::EOF()
				-------------------------------------------
			*/
			/*!
				@brief Mark the stream as complete
				@return The length of the stream (in bytes)
			*/
			uint64_t eof(void)
				{
				return bitstream.eof();
				}

		public:
			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE()
				----------------------------------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_integer_elias_gamma_bitwise()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::~COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE()
				-----------------------------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_elias_gamma_bitwise()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::ENCODE()
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
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers)
				{
				bitstream.rewind(encoded, encoded_buffer_length);
				while (source_integers-- > 0)
					encode(*source++);

				return eof();
				}

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::DECODE()
				----------------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length)
				{
				bitstream.rewind(const_cast<void *>(source));
				while (integers_to_decode-- > 0)
					*decoded++ = decode();
				}

			/*
				COMPRESS_INTEGER_ELIAS_GAMMA_BITWISE::UNITTEST()
				------------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				compress_integer_elias_gamma_bitwise *compressor = new compress_integer_elias_gamma_bitwise;
				compress_integer::unittest(*compressor, 1);
				delete compressor;
				puts("compress_integer_elias_gamma_bitwise::PASSED");
				}
		};
	}
