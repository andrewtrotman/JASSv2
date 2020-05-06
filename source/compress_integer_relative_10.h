/*
	COMPRESS_INTEGER_RELATIVE_10.H
	------------------------------
	Copyright (c) 2007-2017 Vikram Subramanya, Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Relative-10 Compression.
	@author Vikram Subramanya, Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer_simple_9.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_RELATIVE_10
		----------------------------------
	*/
	/*!
		@brief Relative-10 integer compression
		@details Relative-10 is an encoding almost identical to Simple-9, exceot that the selector
		encodes realtive to the previous selector - with the first encoding being in Simple-9.  In this
		way the selector can be encoded in 2 bits rather than 4, and hence it is more effective than
		Simple-9.  The largest integet that can be encoded is 2^28.

		The encodings are:
			1 * 30-bit
			2 * 15-bit
			3 * 10-bit
			4 * 7-bit
			5 * 6-bit
			6 * 5-bit
			7 * 4-bit
			10 * 3-bit
			15 * 2-bit
			30 * 1-bit

		See:
			V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166
	*/
	class compress_integer_relative_10 : public compress_integer_simple_9
		{
		protected:
			/*
				CLASS RELATIVE_10_LOOKUP
				------------------------
			*/
			/*!
				@brief lookup table storing how many integers are encoded and how they are encoded,
			*/
			class relative_10_lookup
				{
				public:
					uint32_t numbers;					///< how many integers
					uint32_t bits;						///< how many bits they take
					uint32_t mask;						///< the mask to extract one
					uint32_t transfer_array[10];		///< which selector to use when encoding
					uint32_t relative_row[4];			///< given the previous row and a 2 bit selector, this is the new row to use
				};

		protected:
			static const relative_10_lookup relative10_table[];	///< The Relative-10 selector table explaining how the encoding works.
			static const uint32_t bits_to_use10[];						///< The number of bits that Relative-10 will be used to store an integer of the given the number of bits in length
			static const uint32_t table_row10[];							///< The row of the table to use given the number of integers to can pack into the word

		public:
			/*
				COMPRESS_INTEGER_RELATIVE_10::COMPRESS_INTEGER_RELATIVE_10()
				------------------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_integer_relative_10()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_RELATIVE_10::~COMPRESS_INTEGER_RELATIVE_10()
				-------------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_relative_10()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_RELATIVE_10::ENCODE()
				--------------------------------------
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
				COMPRESS_INTEGER_RELATIVE_10::DECODE()
				--------------------------------------
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
				COMPRESS_INTEGER_RELATIVE_10::UNITTEST()
				----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
