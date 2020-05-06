/*
	COMPRESS_INTEGER_CARRYOVER_12.H
	-------------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	This is not a port of the ATIRE code, as close inspection suggests that the ATIRE code was not Carryover-12.
	It is also not a port of the Anh & Moffat code, which also isn't Carryover-12.
*/
/*!
	@file
	@brief Carryover-12 Compression.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
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
		@details Simple-9 compression bin-packs as many integers as possible into a single 28-bit word with a 4-bit selector (to make 32-bits).
		Relative-10 uses 2-bit selector to encode the current word reltive to the previous word.  In Carryover-12 the 2-bit selector is stored
		in the previous word if there is room.  This means that sometimes its possible to use 30-bits in the integer and sometimes 32-bits (if the
		selector is in the previous word).

		See:
			V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Aligned Binary Codes, Information Retrieval, 8(1):151-166
	*/
	class compress_integer_carryover_12 : public compress_integer_simple_9
		{
		private:
			/*
		 		CLASS COMPRESS_INTEGER_CARRYOVER_12::SELECTOR
		 		---------------------------------------------
			*/
			/*!
				@brief Details of the 23 selectors used in Carryover-12.
			*/
			class selector
				{
				public:
					const char *name;						///< The name of this selector (not used)
					uint32_t bits;							///< The number of bits per integer
					uint32_t integers;						///< The numner of integers of the given width
					bool next_selector;						///< Is the next selector stored in this word?
					uint32_t new_selector[4];					///< The 2-bit selector take you to the table row stored here
				};

		private:
			static const compress_integer_carryover_12::selector transition_table[];			///< The selector table for Carryover-12

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
