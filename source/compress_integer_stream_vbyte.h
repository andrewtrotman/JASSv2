/*
	COMPRESS_INTEGER_STREAM_VBYTE.H
	-------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Interface to Lemire's Stream VByte source code.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_STREAM_VBYTE
		-----------------------------------
	*/
	/*!
		@brief Stream VByte is an SIMD byte-aligned encoding scheme by Lemire et al.
		@details A byte-aligned encoding scheme that places the selectors before the payloads and uses SIMD instructions to decode.  See:
			D. Lemire, N. Kurz, C. Rupp (2018), Stream VByte: Faster byte-oriented integer compression, Information Processing Letters 130:(1-6)
	*/
	class compress_integer_stream_vbyte : public compress_integer
		{
		public:
			/*
				COMPRESS_INTEGER_STREAM_VBYTE::COMPRESS_INTEGER_VARIABLE_BYTE()
				---------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_stream_vbyte()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_STREAM_VBYTE::~COMPRESS_INTEGER_VARIABLE_BYTE()
				----------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			virtual ~compress_integer_stream_vbyte()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_STREAM_VBYTE::ENCODE()
				---------------------------------------
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
				COMPRESS_INTEGER_STREAM_VBYTE::DECODE()
				---------------------------------------
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
				COMPRESS_INTEGER_STREAM_VBYTE::UNITTEST()
				-----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;


}
