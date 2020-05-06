/*
	COMPRESS_INTEGER_QMX_JASS_V1.H
	------------------------------
	Copyright (c) 2014-2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief QMX version compatible with JASS v1
	@author Andrew Trotman, Matt Crane
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include <vector>

#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_QMX_JASS_V1
		----------------------------------
	*/
	/*!
		@brief QMX compatible with JASS v1 which had its own "special" version that managed end of sequence
		differently from normal.
		@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
	*/
	class compress_integer_qmx_jass_v1 : public compress_integer
		{
		private:
			uint8_t *length_buffer;					///< Stores the number of bits needed to compress each integer
			uint64_t length_buffer_length;			///< The length of length_buffer

		public:
			/*
				COMPRESS_INTEGER_QMX_JASS_V1::COMPRESS_INTEGER_QMX_JASS_V1()
				------------------------------------------------------------
			*/
			/*!
				@brief Constructor
				@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
			*/
			compress_integer_qmx_jass_v1();

			/*
				COMPRESS_INTEGER_QMX_JASS_V1::~COMPRESS_INTEGER_QMX_JASS_V1()
				-------------------------------------------------------------
			*/
			/*!
				@brief Destructor
				@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
			*/
			virtual ~compress_integer_qmx_jass_v1();

			/*
				COMPRESS_INTEGER_QMX_JASS_V1::ENCODE()
				--------------------------------------
			*/
			/*!
				@brief Encode a sequence of integers returning the number of bytes used for the encoding, or 0 if the encoded sequence doesn't fit in the buffer.
				@param encoded [out] The sequence of bytes that is the encoded sequence.
				@param encoded_buffer_length [in] The length (in bytes) of the output buffer, encoded.
				@param source [in] The sequence of integers to encode.
				@param source_integers [in] The length (in integers) of the source buffer.
				@return The number of bytes used to encode the integer sequence, or 0 on error (i.e. overflow).
				@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers);

			/*
				COMPRESS_INTEGER_QMX_JASS_V1::DECODE()
				--------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param decoded [out] The sequence of decoded integers.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
				@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
			*/
			virtual void decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length);


			/*
				COMPRESS_INTEGER_QMX_JASS_V1::UNITTEST_ONE()
				--------------------------------------------
			*/
			/*!
				@brief Test one sequence to make sure it encodes and decodes to the same thing.  Assert if not.
				@param sequence [in] the sequernce to encode.
				@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
			*/
			static void unittest_one(const std::vector<uint32_t> &sequence);

			/*
				COMPRESS_INTEGER_QMX_JASS_V1::UNITTEST()
				----------------------------------------
			*/
			/*!
				@brief Unit test this class
				@deprecated DO NOT USE EXCEPT FOR JASS VERISON 1 COMPATIBILITY
			*/
			static void unittest(void);
		};
	}
