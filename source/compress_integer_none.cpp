/*
	COMPRESS_INTEGER_NONE.CPP
	-------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdio.h>

#include <array>
#include <random>

#include "asserts.h"
#include "compress_integer_none.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_NONE::ENCODE()
		-------------------------------
	*/
	size_t compress_integer_none::encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		size_t used = sizeof(*source) * source_integers;
		if (used > encoded_buffer_length)
			return 0;

		::memcpy(encoded, source, used);

		return used;
		}

	/*
		COMPRESS_INTEGER_NONE::DECODE()
		-------------------------------
	*/
	void compress_integer_none::decode(integer *decoded, size_t integers_to_decode, const void *source, size_t source_length)
		{
		::memcpy(decoded, source, source_length);
		}

	/*
		COMPRESS_INTEGER_NONE::UNITTEST()
		---------------------------------
	*/
	void compress_integer_none::unittest(void)
		{
		compress_integer_none *codex = new compress_integer_none;											// so that encode() and decode() can be called
		std::array<integer, 2048> encoded_buffer = {};					// sequences are encoded into this buffer
		std::array<integer, 2048> decoded_buffer = {};					// sequences are decoded into this buffer
		
		/*
			Generate a sequence of random integers and check they encode and decode correctly.  Yes, this is favouring large integers
			because the probability of the high bit being set is 50%.
		*/
		std::random_device device;
		std::mt19937 generator(device());
		std::uniform_int_distribution<integer> distribution;
		std::array<integer, 128> raw_buffer;
		
		for (auto &element : raw_buffer)
			element = distribution(generator);

		size_t bytes_used = codex->encode(&encoded_buffer[0], encoded_buffer.size() * sizeof(encoded_buffer[0]), &raw_buffer[0], raw_buffer.size());
		codex->decode(&decoded_buffer[0], raw_buffer.size(), &encoded_buffer[0], bytes_used);
		JASS_assert(memcmp(&decoded_buffer[0], &raw_buffer[0], raw_buffer.size() * sizeof(raw_buffer[0])) == 0);

		/*
			Check the overflow case
		*/
		bytes_used = codex->encode(&encoded_buffer[0], 1, &raw_buffer[0], raw_buffer.size());
		JASS_assert(bytes_used == 0);
		
		/*
			The tests have passed
		*/
		delete codex;
		puts("compress_integer_none::PASSED");
		}
	}
