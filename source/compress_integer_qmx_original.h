/*
	COMPRESS_INTEGER_QMX_ORIGINAL.H
	-------------------------------
	Copyright (c) 2014-2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief QMX Compression.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include <vector>

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_QMX_ORIGINAL
		-----------------------------------
	*/
	/*!
		@brief QMX as orignally published (with bug fixes and a a few method name changes)
		@details The original QMX source code with a bug fix to do with short string encoding
		and with the interface changed to fit the JASS requirements.  For details see:

		A. Trotman (2014), Compression, SIMD, and Postings Lists, Proceedings of the 19th Australasian Document Computing Symposium (ADCS 2014)
		
		QMX is a version of BinPacking where we pack into a 128-bit SSE register the following:
		256  0-bit words
		128  1-bit words 
		 64	 2-bit words
		 40  3-bit words
		 32  4-bit words
		 24  5-bit words
		 20  6-bit words
		 16  8-bit words
		 12 10-bit words
		  8 16-bit words
		  4 32-bit words
		or pack into two 128-bit words (i.e. 256 bits) the following:
		 36  7-bit words
		 28  9-bit words
		 20 12-bit words
		 12 21-bit words
		
	This gives us 15 possible combinations.  The combinaton is stored in the top 4 bits of a selector byte.  The
	bottom 4-bits of the selector store a run-length (the number of such sequences seen in a row.

	The 128-bit (or 256-bit) packed binary values are stored first.  Then we store the selectors,  Finally,
	stored variable byte encoded, is a pointer to the start of the selector (from the end of the sequence).

	This way, all reads and writes are 128-bit word aligned, except addressing the selector (and the pointer
	the selector).  These reads are byte aligned.

	Note:  There is currently 1 unused encoding (i.e. 16 unused selector values).  These might in the future be
	used for encoding exceptions, much as PForDelta does.
	
	This code differes from the original as published in two ways.  First, two bugs are fixed (an overflow on reading 
	the buffer to be encoded, and an edge case at end of encoded string), and it has been changed to remove SIMD-word
	alignment requirement.
	*/
	class compress_integer_qmx_original : public compress_integer
		{
		private:
			uint8_t *length_buffer;					///< Stores the number of bits needed to compress each integer
			uint64_t length_buffer_length;			///< The length of length_buffer
			uint32_t *full_length_buffer;			///< If the run_length is too short then 0-pad into this buffer

		private:
			/*
				COMPRESS_INTEGER_QMX_ORIGINAL::WRITE_OUT()
				------------------------------------------
			*/
			/*!
				@brief Encode and write out the sequence into the buffer
				@param buffer [in] where to write the encoded sequence
				@param source [in] the integer sequence to encode
				@param raw_count [in] the numnber of integers to encode
				@param size_in_bits [in] the size, in bits, of the largest integer
				@param length_buffer [in] the length of buffer, in bytes
			*/
			void write_out(uint8_t **buffer, uint32_t *source, uint32_t raw_count, uint32_t size_in_bits, uint8_t **length_buffer);

			/*
				COMPRESS_INTEGER_QMX_ORIGINAL::COMPRESS_INTEGER_QMX_IMPROVED()
				--------------------------------------------------------------
			*/
			/*!
				@brief Not permitted to copy an object of this type due to memory use
			*/
			compress_integer_qmx_original(const compress_integer_qmx_original &obj)
				{
				JASS_assert(false);				// Added to keep Codacy from producing a warning.
				}

		public:
			/*
				COMPRESS_INTEGER_QMX_ORIGINAL::COMPRESS_INTEGER_QMX_IMPROVED()
				--------------------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_integer_qmx_original();

			/*
				COMPRESS_INTEGER_QMX_ORIGINAL::~COMPRESS_INTEGER_QMX_IMPROVED()
				---------------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_integer_qmx_original();

			/*
				COMPRESS_INTEGER_QMX_ORIGINAL::ENCODE()
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
				COMPRESS_INTEGER_QMX_ORIGINAL::DECODE()
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
				COMPRESS_INTEGER_QMX_ORIGINAL::UNITTEST_ONE()
				---------------------------------------------
			*/
			/*!
				@brief Test one sequence to make sure it encodes and decodes to the same thing.  Assert if not.
				@param sequence [in] the sequernce to encode.
			*/
			static void unittest_one(const std::vector<uint32_t> &sequence);

			/*
				COMPRESS_INTEGER_QMX_ORIGINAL::UNITTEST()
				-----------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
	} ;
}
