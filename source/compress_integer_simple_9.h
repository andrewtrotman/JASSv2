/*
	COMPRESS_INTEGER_SIMPLE9.H
	--------------------------
	Copyright (c) 2007-2017 Vikram Subramanya, Andrew Trotman, Blake Burgess
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Anh and Moffat's Simple-9 Compression scheme from:
	V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166

	This code was originally written by Vikram Subramanya while working on:
	A. Trotman, V. Subramanya (2007), Sigma encoded inverted files, Proceedings of CIKM 2007, pp 983-986

	Substantially re-written and converted for use in ATIRE by Andrew Trotman (2009)
	but then adapted for Simple-8b and back ported by Blake Burgess.  This version has been
	refactored for JASS by Andrew Trotman.
*/
/*!
	@file
	@brief Simple-9 Compression.
	@author Vikram Subramanya, Andrew Trotman, Blake Burgess
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_SIMPLE_9
		-------------------------------
	*/
	/*!
		@brief Simple-9 integer compression
		@details Simple-9 compression bit-packs as many integers as possible into a 32-bit word.  All integers
		are packed into the same number of bits.  The encoding is stored in a selector stored in the top 4 bits of the 
		32-bit word and 28-bits for the payload.  Note that, because there are only 28 bits in a payload, the maximum 
		integer that can be encoded with simple-9 is (2^29) - 1 = 536,870,911.  This is less than the number of documens in
		a large collection (such as ClueWeb).
		
		In essence, it encodes into a 32-bit word: 28 * 1-bit integers, or 14 * 2-bit integers, 9 * 3-bit integers,
		7 * 4-bit integers, 5 * 5-bit integers 4 * 7 bit integers, 3 * 9-bit integers, 2 * 14-bit integers, or 1 * 28-bit integer
		
		See:
			V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166
	*/
	class compress_integer_simple_9 : public compress_integer
		{
		protected:
			/*
				CLASS LOOKUP
				------------
				A lookup table encoding how many integers are stored in how many bits and the bitmask to extract an integer
			*/
			/*!
				@brief lookup table storing how many integers are encoded and how they are encoded,
			*/
			class lookup
				{
				public:
					uint32_t numbers;							///< Number of integers encoded using this selector.
					uint32_t bits;								///< NUmber of vits used to encode each integer.
					uint32_t mask;								///< A bitmast to select an integer.
				};

		protected:
			/*
				internal tables for comptability with derived schemes
			*/
			static const lookup simple9_table[];				///< The table mapping bits to slectors and masks
			static const uint32_t bits_to_use[];					///< The number of bits used to store an integer of the given the number of bits in length
			static const uint32_t table_row[];						///< Given the number of bits, which row of simple9_table should be used?

			static const uint32_t ints_packed_table[];			///< Number of integers packed into a 32-bit word, given its mask type
			static const uint32_t can_pack_table[];				///< Bitmask map for valid masks at an offset (column) for some num_bits_needed (row).
			static const uint32_t row_for_bits_needed[];		///< Translates the 'bits_needed' to the appropriate 'row' offset for use with can_pack table.
			static const uint32_t invalid_masks_for_offset[];	///< AND out masks for offsets where we don't know if we can fully pack for that offset
			static const uint32_t simple9_shift_table[];		///< Number of bits to shift when packing - 9 rows for simple-9


		public:
			/*
				COMPRESS_INTEGER_SIMPLE_9::COMPRESS_INTEGER_SIMPLE_9()
				------------------------------------------------------
			*/
			/*!
				@brief Consructor
			*/
			compress_integer_simple_9()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_SIMPLE_9::~COMPRESS_INTEGER_SIMPLE_9()
				-------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_simple_9()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_SIMPLE_9::ENCODE()
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
				COMPRESS_INTEGER_SIMPLE_9::DECODE()
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
				COMPRESS_INTEGER_SIMPLE_9::UNITTEST()
				-------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
