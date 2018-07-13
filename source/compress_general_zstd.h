/*
    COMPRESS_GENERAL_ZSTD.H
    -----------------------
    Copyright (c) 2017 Andrew Trotman
    Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief General purpose compression using the zstd version of deflate
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "zstd.h"
#include "compress_general.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_GENERAL_ZSTD
		---------------------------
	*/
	/*!
		@brief Encoding and decoding general purpose byte sequences using zstd
		@details see: https://github.com/facebook/zstd for details
	*/
	class compress_general_zstd : public compress_general
		{
		private:
			class resources
				{
				public:
					size_t buffInSize;		///< size of the input buffer
					size_t buffOutSize;		///< size of the output buffer
					ZSTD_CStream *cstream;	///< the ZSTD stream
				};
		private:
			resources ress;				///< The compression handle
		
		public:
			/*
				COMPRESS_GENERAL_ZSTD::COMPRESS_GENERAL_ZSTD()
				-----------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			compress_general_zstd();
			
			/*
				COMPRESS_GENERAL_ZSTD::~COMPRESS_GENERAL_ZSTD()
				-----------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~compress_general_zstd();

			/*
				COMPRESS_GENERAL_ZSTD::ENCODE()
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
				COMPRESS_GENERAL_ZSTD::DECODE()
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
				COMPRESS_GENERAL_ZSTD::UNITTEST()
				---------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}

