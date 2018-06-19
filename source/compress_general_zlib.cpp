/*
	COMPRESS_GENERAL_ZLIB.CPP
	-------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdlib.h>

#include <limits>
#include <stdexcept>

#include "asserts.h"
#include "unittest_data.h"
#include "compress_general_zlib.h"

namespace JASS
	{
	/*
		COMPRESS_GENERAL_ZLIB::COMPRESS_GENERAL_ZLIB()
		----------------------------------------------
	*/
	compress_general_zlib::compress_general_zlib()  :
		compress_general()
		{
		stream.zalloc = instream.zalloc = Z_NULL;
		stream.zfree = instream.zfree = Z_NULL;
		stream.opaque = instream.opaque = Z_NULL;

		if (deflateInit(&stream, Z_DEFAULT_COMPRESSION) != Z_OK)
			throw std::runtime_error("Zlib deflateInit() failed in compress_general_zlib::compress_general_zlib()");	// LCOV_EXCL_LINE  // we can't make this happen
		if (inflateInit(&instream) != Z_OK)
			throw std::runtime_error("Zlib inflateInit() failed in compress_general_zlib::compress_general_zlib()");	// LCOV_EXCL_LINE  // we can't make this happen
		}

	/*
		COMPRESS_GENERAL_ZLIB::~COMPRESS_GENERAL_ZLIB()
		-----------------------------------------------
	*/
	compress_general_zlib::~compress_general_zlib()
		{
		deflateEnd(&stream);
		inflateEnd(&instream);
		}

	/*
		COMPRESS_GENERAL_ZLIB::ENCODE()
		-------------------------------
	*/
	size_t compress_general_zlib::encode(void *encoded, size_t encoded_buffer_length, const void *source, size_t source_bytes)
		{
		/*
			Reset the zlib stream.
		*/
		if (deflateReset(&stream) != Z_OK)
			return 0;								// LCOV_EXCL_LINE  // we can't make this happen

		/*
			Check for overflow before we go-ahead.
		*/
		if (source_bytes > std::numeric_limits<decltype(stream.avail_in)>::max())
			return 0; 
		if (encoded_buffer_length > std::numeric_limits<decltype(stream.avail_out)>::max())
			return 0; 

		/*
			Setup the parameter block
		*/
		stream.avail_in  = static_cast<decltype(stream.avail_in)>(source_bytes);
		stream.next_in   = reinterpret_cast<decltype(stream.next_in)>(const_cast<void *>(source));
		stream.avail_out = static_cast<decltype(stream.avail_out)>(encoded_buffer_length);
		stream.next_out  = static_cast<decltype(stream.next_out)>(encoded);

		/*
			Compress
		*/
		auto success = deflate(&stream, Z_FINISH);    /* no bad return value */
		return success == Z_STREAM_END ? (encoded_buffer_length - stream.avail_out) : 0;
		}

	/*
		COMPRESS_GENERAL_ZLIB::DECODE()
		-------------------------------
	*/
	size_t compress_general_zlib::decode(void *decoded, size_t destination_length, const void *source, size_t source_bytes)
		{
		/*
			Reset the zlib stream.
		*/
		if (inflateReset(&instream) != Z_OK)
			return 0;								// LCOV_EXCL_LINE  // we can't make this happen

		/*
			Check for overflow before we go-ahead.
		*/
		if (source_bytes > std::numeric_limits<decltype(stream.avail_in)>::max())
			return 0; 
		if (destination_length > std::numeric_limits<decltype(stream.avail_out)>::max())
			return 0; 

		/*
			Setup the parameter block
		*/
		instream.avail_in  = static_cast<decltype(stream.avail_in)>(source_bytes);
		instream.next_in   = reinterpret_cast<decltype(stream.next_in)>(const_cast<void *>(source));
		instream.avail_out = static_cast<decltype(stream.avail_out)>(destination_length);
		instream.next_out  = static_cast<decltype(stream.next_out)>(decoded);

		/*
			Decompress
		*/
		auto status = inflate(&instream, Z_FINISH);

		return status == Z_STREAM_END ? (destination_length - instream.avail_out) : 0;
		}
		
	/*
		COMPRESS_GENERAL_ZLIB::UNITTEST()
		---------------------------------
	*/
	void compress_general_zlib::unittest(void)
		{
		/*
			Check that a decoded encoded string is the same as the source string
		*/
		compress_general_zlib codex;
		std::string encoded;
		std::string decoded;

		encoded.resize(unittest_data::ten_documents.size());
		auto took = codex.encode(const_cast<char *>(encoded.data()), encoded.size(), unittest_data::ten_documents.c_str(), unittest_data::ten_documents.size());
		JASS_assert(took < unittest_data::ten_documents.size());

		decoded.resize(unittest_data::ten_documents.size());
		auto became = codex.decode(const_cast<char *>(decoded.data()), decoded.size(), encoded.data(), took);
		JASS_assert(became == unittest_data::ten_documents.size());
		JASS_assert(decoded == unittest_data::ten_documents);

		auto zero = codex.encode(const_cast<char *>(encoded.data()), std::numeric_limits<size_t>::max(), unittest_data::ten_documents.c_str(), unittest_data::ten_documents.size());
		JASS_assert(zero == 0);
		zero = codex.encode(const_cast<char *>(encoded.data()), encoded.size(), unittest_data::ten_documents.c_str(), std::numeric_limits<size_t>::max());
		JASS_assert(zero == 0);
		zero = codex.decode(const_cast<char *>(decoded.data()), std::numeric_limits<size_t>::max(), encoded.data(), took);
		JASS_assert(zero == 0);
		zero = codex.decode(const_cast<char *>(decoded.data()), decoded.size(), encoded.data(),  std::numeric_limits<size_t>::max());
		JASS_assert(zero == 0);

		/*
			The tests have passed
		*/
		puts("compress_general_zlib::PASSED");
		}
	}
