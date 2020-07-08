/*
	COMPRESS_INTEGER_STREAM_VBYTE.CPP
	---------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdio.h>

#include <random>

#include "asserts.h"
#include "compress_integer_stream_vbyte.h"

namespace streamvbyte
	{
	#ifdef _MSC_VER
		#define __restrict__ __restrict
	#endif

	#include "../external/streamvbyte/src/streamvbyte_shuffle_tables.h"
	#include "../external/streamvbyte/src/streamvbyte.c"
	}

namespace JASS
	{
	/*
		COMPRESS_INTEGER_STREAM_VBYTE::ENCODE()
		---------------------------------------
	*/
	size_t compress_integer_stream_vbyte::encode(void *encoded_as_void, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		return streamvbyte::streamvbyte_encode(const_cast<integer *>(source), static_cast<uint32_t>(source_integers), static_cast<uint8_t *>(encoded_as_void));
		}

	/*
		COMPRESS_INTEGER_STREAM_VBYTE::DECODE()
		---------------------------------------
	*/
	void compress_integer_stream_vbyte::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		streamvbyte::streamvbyte_decode(reinterpret_cast<uint8_t *>(const_cast<void *>(source_as_void)), decoded, static_cast<uint32_t>(integers_to_decode));
		}

	/*
		COMPRESS_INTEGER_STREAM_VBYTE::UNITTEST()
		-----------------------------------------
	*/
	void compress_integer_stream_vbyte::unittest(void)
		{
		compress_integer_stream_vbyte *compressor = new compress_integer_stream_vbyte;
		std::vector<uint32_t> sequence;
		size_t instance;

		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFFF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFFFF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFFFFF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFFFFFF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFFFFFFF);
		for (instance = 0; instance < 128; instance++)
			sequence.push_back(0xFFFFFFFF);

		std::vector<uint32_t>compressed(sequence.size() * 2);
		std::vector<uint32_t>decompressed(sequence.size() + 256);

		auto size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor->decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		JASS_assert(decompressed == sequence);

		auto will_take = streamvbyte::streamvbyte_max_compressedbytes(128);
		JASS_assert(will_take == 544);

		sequence.clear();
		uint32_t empty = 0;
		size_once_compressed = compressor->encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &empty, sequence.size());
		JASS_assert(size_once_compressed == 0);

		/*
			Try decompressing 0 bytes - streamvbyte::streamvbyte_decode() should return 0 which is then ignored (but the coverage tool notices that we've done the check).
		*/
		compressor->decode(&decompressed[0], 0, &compressed[0], size_once_compressed);

		/*
			The tests have passed
		*/
		delete compressor;
		puts("compress_integer_stream_vbyte::PASSED");
		}
	}
