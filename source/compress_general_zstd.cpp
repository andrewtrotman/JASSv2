/*
	COMPRESS_GENERAL_ZSTD.CPP
	-------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdlib.h>

#include <limits>

#include "asserts.h"
#include "unittest_data.h"
#include "compress_general_zstd.h"

namespace JASS
	{
	/*
		COMPRESS_GENERAL_ZSTD::COMPRESS_GENERAL_ZSTD()
		----------------------------------------------
	*/
	compress_general_zstd::compress_general_zstd()  :
		compress_general()
		{
		ress.buffInSize = ZSTD_CStreamInSize();
		ress.buffOutSize = ZSTD_CStreamOutSize();
		ress.cstream = ZSTD_createCStream();
		}

	/*
		COMPRESS_GENERAL_ZSTD::~COMPRESS_GENERAL_ZSTD()
		-----------------------------------------------
	*/
	compress_general_zstd::~compress_general_zstd()
		{
		ZSTD_freeCStream(ress.cstream);
		}

	/*
		COMPRESS_GENERAL_ZSTD::ENCODE()
		-------------------------------
	*/
	size_t compress_general_zstd::encode(void *encoded, size_t encoded_buffer_length, const void *source, size_t source_bytes)
		{
		size_t initResult = ZSTD_initCStream(ress.cstream, 7);
		if (ZSTD_isError(initResult))
			return 0;

		ZSTD_inBuffer input = {source, source_bytes, 0};
		ZSTD_outBuffer output = {encoded, encoded_buffer_length, 0};

		while (input.pos < input.size)
			{
			auto toRead = ZSTD_compressStream(ress.cstream, &output , &input);   /* toRead is guaranteed to be <= ZSTD_CStreamInSize() */
			if (ZSTD_isError(toRead))
				return 0;
			}

		size_t const remainingToFlush = ZSTD_endStream(ress.cstream, &output);   /* close frame */
		if (remainingToFlush != 0)
			return 0;
			
		return output.pos;
		}

	/*
		COMPRESS_GENERAL_ZSTD::DECODE()
		-------------------------------
	*/
	size_t compress_general_zstd::decode(void *decoded, size_t destination_length, const void *source, size_t source_bytes)
		{
		/*
		if (inflateReset(&instream) != Z_OK)
			return 0;

		instream.avail_in = source_bytes;
		instream.next_in = (Bytef *)source;
		instream.avail_out = destination_length;
		instream.next_out = (Bytef *)decoded;

		auto status = inflate(&instream, Z_FINISH);

		return status == Z_STREAM_END ? (destination_length - instream.avail_out) : 0;
		*/
		return 0;
		}
		
	/*
		COMPRESS_GENERAL_ZSTD::UNITTEST()
		---------------------------------
	*/
	void compress_general_zstd::unittest(void)
		{
		/*
			Check that a decoded encoded string is the same as the source string
		*/
		compress_general_zstd codex;
		std::string encoded;
		std::string decoded;

		encoded.resize(unittest_data::ten_documents.size());
		auto took = codex.encode(const_cast<char *>(encoded.data()), encoded.size(), unittest_data::ten_documents.c_str(), unittest_data::ten_documents.size());
		JASS_assert(took < unittest_data::ten_documents.size());

		decoded.resize(unittest_data::ten_documents.size());
		auto became = codex.decode(const_cast<char *>(decoded.data()), decoded.size(), encoded.data(), took);
		JASS_assert(became == unittest_data::ten_documents.size());
		JASS_assert(decoded == unittest_data::ten_documents);

		/*
			The tests have passed
		*/
		puts("compress_general_zstd::PASSED");
		}
	}
