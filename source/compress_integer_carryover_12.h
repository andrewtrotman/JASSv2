/*
	COMPRESS_INTEGER_CARRYOVER_12.H
	-------------------------------

*/
#pragma once

#include "compress_integer_simple_9.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_CARRYOVER_12
		-----------------------------------
	*/
	/*!
		@brief Carryover-12 integer compression
		@details Similar to Simple-9, except that it uses the wasted bits between words.

		See:
			V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166
	*/
	class compress_integer_carryover_12 : public compress_integer_simple_9
		{
		public:
			/*
				COMPRESS_INTEGER_CARRYOVER_12::COMPRESS_INTEGER_CARRYOVER_12
				------------------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_carryover_12()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_CARRYOVER_12::~COMPRESS_INTEGER_CARRYOVER_12
				------------------------------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~compress_integer_carryover_12()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_CARRYOVER_12::ENCODE()
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
				COMPRESS_INTEGER_CARRYOVER_12::DECODE()
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
				COMPRESS_INTEGER_CARRYOVER_12::UNITTEST()
				-----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
