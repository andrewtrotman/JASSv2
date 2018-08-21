
/*
	COMPRESS_INTEGER_ELIAS_DELTA_SIMD.H
	------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdint.h>
#include <strings.h>
#include <immintrin.h>

#include <bitset>
#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_elias_delta_simd.h"

#define WORD_WIDTH 32
#define WORDS (512 / WORD_WIDTH)

namespace JASS
	{
	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::PUSH_SELECTOR()
		--------------------------------------------------
	*/
	void compress_integer_elias_delta_simd::push_selector(uint32_t *&destination, uint8_t raw)
		{
		uint32_t width = maths::floor_log2(raw);

		selector_bits_used += width;

//std::bitset<32> zero(0);
//std::cout << zero << ' ';

		uint64_t zig_zag = ((raw & ~(1 << width)) << 1) + 1;

		accumulated_selector |= zig_zag << selector_bits_used;

//std::bitset<32> zz(zig_zag);
//std::cout << zz << ' ' << std::bitset<32>(accumulated_selector) << '\n';

		selector_bits_used += width + 1;

		if (selector_bits_used > 32)
			{
std::cout << "WRITE->" << std::bitset<32>(accumulated_selector & 0xFFFFFFFFFFFFFFFF) << "\n\n";
			*destination++ = accumulated_selector & 0xFFFFFFFFFFFFFFFF;
			accumulated_selector >>= 32;
			selector_bits_used -= 32;
			}
		}
	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::FLUSH()
		------------------------------------------
	*/
	void compress_integer_elias_delta_simd::flush(uint32_t *&destination)
		{
		std::cout << "WRITE->" << std::bitset<32>(accumulated_selector & 0xFFFFFFFFFFFFFFFF) << "\n\n";
		*destination++ = accumulated_selector & 0xFFFFFFFFFFFFFFFF;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE_SELECTOR()
		----------------------------------------------------
		selector_bits_used = 0;
		accumulated_selector = 0;
	*/
	uint32_t compress_integer_elias_delta_simd::decode_selector(uint32_t *&selector_set)
		{
		if (selector_bits_used >= 32)
			{
			selector_bits_used -= 32;
			uint64_t next_word = *selector_set++;
			accumulated_selector = accumulated_selector | (next_word << (32 - selector_bits_used));
			}

		/*
			get the width
		*/
		uint64_t unary = _tzcnt_u64(accumulated_selector);

		/*
			get the zig-zag encoded binary, unzig-zag it and store it
		*/
		uint32_t decoded = (_bextr_u64(accumulated_selector, unary, unary + 1) >> 1) | (1UL << unary);

		/*
			Remember how much we've already used
		*/
		selector_bits_used += unary + unary + 1;
		accumulated_selector >>= unary + unary + 1;

		return decoded;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::ENCODE()
		-------------------------------------------
	*/
	size_t compress_integer_elias_delta_simd::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t elements)
		{
		return 0;
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::DECODE()
		-------------------------------------------
	*/
	void compress_integer_elias_delta_simd::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		}

	/*
		COMPRESS_INTEGER_ELIAS_DELTA_SIMD::UNITTEST()
		---------------------------------------------
	*/
	void compress_integer_elias_delta_simd::unittest(void)
		{
		const size_t end = 50;
		compress_integer_elias_delta_simd compressor;
		std::vector<uint8_t> mem(1024);

		uint32_t *into = (uint32_t *)mem.data();
		compressor.rewind();
		for (size_t x = 1; x <= end; x++)
			compressor.push_selector(into, x);

		compressor.flush(into);				// check the end case when we don't need to flush

		into = (uint32_t *)mem.data();
		compressor.rewind2();
		for (size_t x = 1; x <= end; x++)
			{
			uint32_t got = compressor.decode_selector(into);
			std::cout << x << "->" << got;
			if (x != got)
				std::cout << " WRONG";
			std::cout << '\n';
			}

exit(1);
		puts("compress_integer_prn_512_carryover::PASSED");
		}
	}
