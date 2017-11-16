
/*
	COMPRESS_INTEGER_CARRY_8B.H
	---------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	The Anh and Moffat code proved too awkward to remove from their test harness so this is a re-implementation
*/
/*!
	@file
	@brief Carry-8b Compression.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "compress_integer_simple_9.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_CARRY_8B
		-------------------------------
	*/
	/*!
		@brief Carry-8b integer compression
		@details Simple-9 compression bin-packs as many integers as possible into a single 28-bit word with a 4-bit selector (to make 32-bits).
		Relative-10 uses 2-bit selector to encode the current word reltive to the previous word.  In Carryover-12 the 2-bit selector is stored
		in the previous word if there is room.  In Carry-8b the selector is 4 bits and carried in the previous word if it fits.  Note, however,
		that there are 22 possible selectors for 60 or 64-bits and which ones are used is carried in the first word - it uses 16 consequitive
		selectors from the table starting at that base value.

		See:
			V. Anh, A. Moffat (2010), Index compression using 64-bit words, Software Practive & Experience, pp 131-147
	*/
	class compress_integer_carry_8b : public compress_integer_simple_9
		{
		private:
			/*
		 		CLASS COMPRESS_INTEGER_CARRY_8B::SELECTOR
		 		-----------------------------------------
			*/
			/*!
				@brief Details of the selectors used in Carry-8b
			*/
			class selector
				{
				public:
					const char *name;						///< The name of this selector (not used)
					size_t bits;							///< The number of bits per integer
					size_t integers;						///< The numner of integers of the given width
					size_t next_selector;				///< Is the next selector stored in this word - if so then at which bit position?
				};

		private:
			static const compress_integer_carry_8b::selector selector_table[];			///< The selector table for Carry-8b
			static const size_t base_table[];														///< The starting point in the selector table.

		private:
			/*
				COMPRESS_INTEGER_CARRY_8B::PACK_ONE_WORD
				----------------------------------------
			*/
			/*!
				@brief encode some number of integers into the given destination using the encoding table.
				@param base [in] where in the selector_table to start from
				@param highest [in] how many selectors to use starting at base
				@param destination [in] where to put the encoded codeword
				@param source [in] the integer sequence to encode
				@param source_integers [in] the maximum number of integers to encode
				@param next_selector_in_previous_word [in] Should the compressor pack the selector in the previous word?
				@return the number of integers that were encoded (or 0 on failure to encode)
			*/
			size_t pack_one_word(size_t base, size_t highest, uint64_t *destination, const integer *source, size_t source_integers, size_t &next_selector_in_previous_word);

		public:
			/*
				COMPRESS_INTEGER_CARRY_8B::COMPRESS_INTEGER_CARRY_8B
				----------------------------------------------------
			*/
			/*!
				@brief Constructor.
			*/
			compress_integer_carry_8b()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_CARRY_8B::~COMPRESS_INTEGER_CARRY_8B
				-----------------------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~compress_integer_carry_8b()
				{
				/* Nothing */
				}

			/*
				COMPRESS_INTEGER_CARRY_8B::ENCODE()
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
				COMPRESS_INTEGER_CARRY_8B::DECODE()
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
				COMPRESS_INTEGER_CARRY_8B::UNITTEST_THIS()
				------------------------------------------
			*/
			/*!
				@brief Unit test one sequence of integers - assert on failure.
				@param every_case [in] the sequence to test
			*/
			static void unittest_this(std::vector<integer> every_case);

			/*
				COMPRESS_INTEGER_CARRY_8B::UNITTEST()
				-------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
