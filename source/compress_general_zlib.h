/*
    COMPRESS_GENERAL_ZLIB.H
    -----------------------
    Copyright (c) 2017 Andrew Trotman
    Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief General purpose compression using zlib (deflate)
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "zlib.h"
#include "compress_general.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_GENERAL_ZLIB
		---------------------------
	*/
	/*!
		@brief Encoding and decoding general purpose byte sequences using zlib
		@details see: http://zlib.net for details
	*/
	class compress_general_zlib : public compress_general
		{
		private:
			z_stream stream;			///< compression context for compression (deflation)
			z_stream instream;		///< compression context for decompression (inflation)

		public:
			/*
				COMPRESS_GENERAL_ZLIB::COMPRESS_GENERAL_ZLIB()
				-----------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_general_zlib();
			
			/*
				COMPRESS_GENERAL_ZLIB::~COMPRESS_GENERAL_ZLIB()
				-----------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_general_zlib();

			/*
				COMPRESS_GENERAL_ZLIB::ENCODE()
				-------------------------------
			*/
			/*!
				@brief Encode a byte sequence
				@param encoded [out] The encoded sequence
				@param encoded_buffer_length [in] The length of encoded (used to prevent overflow)
				@param source [in] The byte sequence to encode
				@param source_bytes [in] The number of bytes to encode (starting at source)
				@return The number of bytes used to encode the input sequence, or 0 on error
			*/
			virtual size_t encode(void *encoded, size_t encoded_buffer_length, const void *source, size_t source_bytes);
			
			/*
				COMPRESS_GENERAL_ZLIB::DECODE()
				-------------------------------
			*/
			/*!
				@brief Encode a byte sequence
				@param decoded [out] The decoded sequence
				@param destination_length [in] The length of decoded (used to prevent overflow)
				@param source [in] The byte sequence to decode
				@param source_bytes [in] Length of the encoded sequence
				@return The number of bytes decoded, or 0 on error.
			*/
			virtual size_t decode(void *decoded, size_t destination_length, const void *source, size_t source_bytes);
			
			/*
				COMPRESS_GENERAL_ZLIB::UNITTEST()
				---------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

