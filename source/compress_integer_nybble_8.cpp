/*
	COMPRESS_INTEGER_NYBBLE_8.CPP
	-----------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <immintrin.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_nybble_8.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_NYBBLE_8::ENCODE()
		-----------------------------------
	*/
	size_t compress_integer_nybble_8::encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		uint8_t *destination = (uint8_t *)encoded;
		uint8_t *end_of_destination = destination + encoded_buffer_length;

		while (source_integers > 0)
			{
			/*
				Check that we'll fit
			*/
			if (destination + 2 > end_of_destination)
				return 0;

			if (source_integers == 1 || *source >= 16 || *(source + 1) >= 16)
				{
				/*
					Check for integer too large
				*/
				if (*source > 255)
					return 0;

				/*
					Pack that integer into the byte
				*/
				*destination++ = 1;
				*destination++ = (uint8_t)*source;
				source++;
				source_integers--;
				}
			else
				{
				/*
					Pack 2 integers into the byte
				*/
				*destination++ = 0;
				*destination++ = (*source << 4) | *(source + 1);
				source += 2;
				source_integers -= 2;
				}
			}
		return destination - (uint8_t *)encoded;
		}

	/*
		COMPRESS_INTEGER_NYBBLE_8::DECODE()
		-----------------------------------
	*/
	void compress_integer_nybble_8::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		uint32_t *into = (uint32_t *)decoded;
		const uint8_t *source = (uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;

		while (source < end_of_source)
			{
			uint8_t width = *source++;
			uint8_t data = *source++;
			switch (width)
				{
				case 0:
					*(into + 0) = data >> 4;
					*(into + 1) = data & 0x0F;

					into += 2;
					break;
				case 1:
					*into = data;
					into++;
					break;
				}
			}
		}

	/*
		COMPRESS_INTEGER_NYBBLE_8::UNITTEST_ONE()
		-----------------------------------------
	*/
	void compress_integer_nybble_8::unittest_one(const std::vector<uint32_t> &sequence)
		{
		compress_integer_nybble_8 compressor;
		std::vector<uint32_t>compressed(sequence.size() * 5);
		std::vector<uint32_t>decompressed(sequence.size() + 256);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor.decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		JASS_assert(decompressed == sequence);
		}

	/*
		COMPRESS_INTEGER_NYBBLE_8::UNITTEST()
		-------------------------------------
	*/
	void compress_integer_nybble_8::unittest(void)
		{
		std::vector<uint32_t> every_case;
		uint32_t instance;

		/*
			1-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 32 * 8; instance++)
			every_case.push_back(0x01);
		unittest_one(every_case);

		/*
			2-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 16 * 8; instance++)
			every_case.push_back(0x03);
		unittest_one(every_case);

		/*
			3-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 10 * 8; instance++)
			every_case.push_back(0x07);
		unittest_one(every_case);

		/*
			4-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 8 * 8; instance++)
			every_case.push_back(0x0F);
		unittest_one(every_case);

		/*
			5-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 8 * 8; instance++)
			every_case.push_back(0x1F);
		unittest_one(every_case);

		/*
			8-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 8 * 8; instance++)
			every_case.push_back(0xFF);
		unittest_one(every_case);

		/*
			all the compressable integers (0..255)
		*/
		every_case.clear();
		for (instance = 0; instance < 256; instance++)
			every_case.push_back(instance);
		unittest_one(every_case);

		puts("compress_integer_nybble_8::PASSED");
		}
	}
