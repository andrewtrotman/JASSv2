/*
    COMPRESS_GENERAL.H
    ------------------
    Copyright (c) 2017 Andrew Trotman
    Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief General purpose compression
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

namespace JASS
	{
	/*
		CLASS COMPRESS_GENERAL
		----------------------
	*/
	/*!
		@brief Base class for encoding and decoding general purpose byte sequences
	*/
	class compress_general
		{
		public:
			/*
				COMPRESS_GENERAL::COMPRESS_GENERAL()
				-------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_general()
				{
				/* Nothing. */
				}

			/*
				COMPRESS_GENERAL::~COMPRESS_GENERAL()
				-------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_general()
				{
				/* Nothing. */
				}

			/*
				COMPRESS_GENERAL::ENCODE()
				--------------------------
			*/
			/*!
				@brief Encode a byte sequence
				@param encoded [out] The encoded sequence
				@param encoded_buffer_length [in] The length of encoded (used to prevent overflow)
				@param source [in] The byte sequence to encode
				@param source_bytes [in] The number of bytes to encode (starting at source)
				@return The number of bytes used to encode the input sequence
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const void *source, size_t source_bytes) = 0;
			
			/*
				COMPRESS_GENERAL::DECODE()
				--------------------------
			*/
			/*!
				@brief Encode a byte sequence
				@param decoded [out] The decoded sequence
				@param destination_length [in] The length of decoded (used to prevent overflow)
				@param source [in] The byte sequence to decode
				@param source_bytes [in] Length of the encoded sequence
			*/
			virtual size_t decode(void *decoded, size_t destination_length, const void *source, size_t source_bytes) = 0;
			};
	}

