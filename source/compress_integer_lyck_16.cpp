/*
	COMPRESS_INTEGER_LYCK_16.CPP
	----------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <immintrin.h>

#include <vector>

#include "maths.h"
#include "asserts.h"
#include "compress_integer_bitpack.h"
#include "compress_integer_lyck_16.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_LYCK_16::ENCODE()
		-----------------------------------
	*/
	size_t compress_integer_lyck_16::encode(void *encoded, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		uint8_t *destination = (uint8_t *)encoded;
		uint8_t *end_of_destination = destination + encoded_buffer_length;

		while (source_integers > 0)
			{
			/*
				Check that we'll fit
			*/
			if (destination + 3 > end_of_destination)
				return 0;

			uint32_t integers_to_encode;
			size_t cumulative = 0;
			uint32_t ending = source_integers > 16 ? 16 : (uint32_t)source_integers;
			for (integers_to_encode = 0; integers_to_encode < ending; integers_to_encode++)
				{
				auto bits = compress_integer_bitpack::bits_needed(source[integers_to_encode]);
				/*
					Check for overflow
				*/
				if (bits > 16)
					return 0;

				/*
					Check to see if we still fit in the current word
				*/
				cumulative += bits;
				if (cumulative > 16)
					break;
				}

			/*
				integers_to_encode is the number of integers that will fit
			*/
			switch (integers_to_encode)
				{
				case 0:						// Cannot happen
					JASS_assert(integers_to_encode == 0);
					break;
				case 1:						// 1 * 16-bit integer
					*destination++ = 3;
					*(uint16_t *)destination = *source;
					source++;
					destination += 2;
					source_integers -= 1;
					break;
				case 2:						// 2 * 8-bit integer
				case 3:
					*destination++ = 2;
					*(uint16_t *)destination = (*source << 8) | *(source + 1);
					source += 2;
					destination += 2;
					source_integers -= 2;
					break;
				case 4:						// 4 * 4-bit integers
				case 5:
				case 6:
				case 7:
					*destination++ = 1;
					*(uint16_t *)destination = (*source << 12) | (*(source + 1) << 8) | (*(source + 2) << 4) | *(source + 3);
					source += 4;
					destination += 2;
					source_integers -= 4;
					break;
				default:						// 8 * 2-bit integers
					*destination++ = 0;
					*(uint16_t *)destination = (*source << 14) | (*(source + 1) << 12) | (*(source + 2) << 10) | (*(source + 3) << 8) | (*(source + 4) << 6) | (*(source + 5) << 4) | (*(source + 6) << 2) | *(source + 7);
					source += 8;
					destination += 2;
					source_integers -= 8;
					break;
				}
			}
		return destination - (uint8_t *)encoded;
		}

	/*
		COMPRESS_INTEGER_LYCK_16::DECODE()
		----------------------------------
	*/
	void compress_integer_lyck_16::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		uint32_t *into = (uint32_t *)decoded;
		const uint8_t *source = (const uint8_t *)source_as_void;
		const uint8_t *end_of_source = source + source_length;

		while (source < end_of_source)
			{
			uint8_t width = *source++;
			uint16_t data = *(uint16_t *)source;
			source += 2;
			switch (width)
				{
				case 0:
						*(into + 0) = data >> 14;
						*(into + 1) = (data >> 12) & 0x03;
						*(into + 2) = (data >> 10) & 0x03;
						*(into + 3) = (data >> 8) & 0x03;
						*(into + 4) = (data >> 6) & 0x03;
						*(into + 5) = (data >> 4) & 0x03;
						*(into + 6) = (data >> 2) & 0x03;
						*(into + 7) = data & 0x03;

						into += 8;
					break;
				case 1:
						*(into + 0) = data >> 12;
						*(into + 1) = (data >> 8) & 0x0F;
						*(into + 2) = (data >> 4 )& 0x0F;
						*(into + 3) = data & 0x0F;

						into += 4;
					break;
				case 2:
						*(into + 0) = data >> 8;
						*(into + 1) = data & 0xFF;

						into += 2;
					break;
				case 3:
						*into = data;
						into++;
					break;
				}
			}
		}

	/*
		COMPRESS_INTEGER_LYCK_16::UNITTEST_ONE()
		----------------------------------------
	*/
	void compress_integer_lyck_16::unittest_one(const std::vector<uint32_t> &sequence)
		{
		compress_integer_lyck_16 compressor;
		std::vector<uint32_t>compressed(sequence.size() * 5);
		std::vector<uint32_t>decompressed(sequence.size() + 256);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor.decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		JASS_assert(decompressed == sequence);
		}

	/*
		COMPRESS_INTEGER_LYCK_16::UNITTEST()
		------------------------------------
	*/
	void compress_integer_lyck_16::unittest(void)
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
			16-bit integers
		*/
		every_case.clear();
		for (instance = 0; instance < 8 * 8; instance++)
			every_case.push_back(0xFFFF);
		unittest_one(every_case);

		/*
			All the compressable integers (0..65535)
		*/
		every_case.clear();
		for (instance = 0; instance <= 0xFFFF; instance++)
			every_case.push_back(instance);
		unittest_one(every_case);

		puts("compress_integer_lyck_16::PASSED");
		}
	}
