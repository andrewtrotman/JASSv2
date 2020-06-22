/*
	COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB.CPP
	----------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdint.h>
#include <immintrin.h>

#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_elias_gamma_simd_vb.h"

#define WORD_WIDTH 32
#define WORDS (512 / WORD_WIDTH)

namespace JASS
	{
	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::COMPUTE_SELECTOR()
		--------------------------------------------------------
	*/
	uint32_t compress_integer_elias_gamma_simd_vb::compute_selector(const uint8_t *encodings)
		{
		uint32_t value = 0;
		int current;

		for (current = 0; current < 32; current++)
			if (encodings[current] == 0)
				break;

		/*
			Compute the permutation number
		*/
		for (current--; current >=  0; current--)
			{
			size_t number_of_0s = encodings[current];
			value <<= number_of_0s;
			value |= 1 << (number_of_0s - 1);
			}

		return value;
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::ENCODE()
		----------------------------------------------
	*/
	size_t compress_integer_elias_gamma_simd_vb::encode(void *encoded, size_t encoded_buffer_length, const integer *array, size_t elements)
		{
		uint8_t encodings[33] = {0};
		uint32_t *destination = (uint32_t *)encoded;
		uint32_t *end_of_destination = (uint32_t *)((uint8_t *)encoded + encoded_buffer_length);

		/*
			Store the length of the variable byte part
		*/
		*destination++ = 0;			// store as a 4-byte integer to start with (variable byte encode it later)
		while (1)
			{
			/*
				Check for overflow of the output buffer
			*/
			if (destination + WORDS + 1 + 1 > end_of_destination)
				return 0;

			/*
				Zero the result memory before writing the bit paterns into it
			*/
			::memset(destination, 0, (WORDS + 1) * 4);

			/*
				Get the address of the selector and move on to the payload
			*/
			uint32_t *selector = destination;
			destination++;

			/*
				Encode the next integer
			*/
			uint32_t remaining = WORD_WIDTH;
			uint32_t cumulative_shift = 0;
			const integer *array_at_start_of_codeword = array;
			size_t elements_at_start_of_codeword = elements;

			/*
				Pack into the next word
			*/
			uint32_t slice;
			for (slice = 0; slice < 32; slice++)
				{
				/*
					Find the width of this column
				*/
				uint32_t max_width = 1;
				for (uint32_t word = 0; word < WORDS; word++)
					max_width |= word < elements ? array[word] : 1;

				max_width = maths::ceiling_log2(max_width);

				if (max_width > remaining)
					break;			// move on to the next code word
				else
					{
					/*
						Pack them in because there's room
					*/
					encodings[slice] = max_width;
					for (uint32_t word = 0; word < WORDS; word++)
						destination[word] |= (word < elements ? array[word] : 0) << cumulative_shift;
					cumulative_shift += max_width;
					remaining -= max_width;

					/*
						Check for end of input
					*/
					if (WORDS >= elements)
						{
						/*
							Pack the last selector
						*/
						encodings[slice] += remaining;
						encodings[slice + 1] = 0;
						*selector = compute_selector(encodings);

						if (WORDS == elements)
							{
							uint32_t *vb_length_locaton = (uint32_t *)encoded;
							*vb_length_locaton = 0;
							return (uint8_t *)(destination + WORDS) - (uint8_t *)encoded;						// we've finished the encoding
							}
						else
							{
							/*
								We're at end of input so either re-code the final column as variable byte or re-code the entire word.
							*/

							/*
								Check to see how many bits of the current word we're using.
							*/
							size_t elias_size = WORDS * (WORD_WIDTH / 8);		// size (in bytes) of the codeword up to the last column
							for (size_t current = 0; current < elements; current++)
								elias_size += bytes_needed_for(array[current]);

							/*
								Check to see how much space it would take with variable byte
							*/
							size_t vbyte_size = 0;
							for (size_t current = 0; current < elements_at_start_of_codeword; current++)
								vbyte_size += bytes_needed_for(array_at_start_of_codeword[current]);

							if (vbyte_size < elias_size)
								{
								// re-code the codeword
								vbyte_size = compress_integer_variable_byte::encode(selector, end_of_destination - selector, array_at_start_of_codeword, elements_at_start_of_codeword);
								destination = selector;
								}
							else
								{
								// re-code the final column
								for (uint32_t word = 0; word < WORDS; word++)
									destination[word] ^= (word < elements ? array[word] : 0) << (cumulative_shift - max_width);// becuase we've alread incremente cumulative_shift
								destination += WORDS;
								encodings[slice - 1] += encodings[slice];
								encodings[slice] = 0;
								*selector = compute_selector(encodings);
								vbyte_size = compress_integer_variable_byte::encode(destination, end_of_destination - selector, array, elements);
								}
							uint32_t *vb_length_locaton = (uint32_t *)encoded;
							*vb_length_locaton = vbyte_size;

							return (uint8_t *)destination + vbyte_size - (uint8_t *)encoded;						// we've finished the encoding
							}
						}
					elements -= WORDS;
					array += WORDS;
					}
				}
			/*
				Didn't fit!  Finish this codeword by padding the last width to the full width of the block (i.e. add the spare bits)
			*/
			encodings[slice - 1] += remaining;
			encodings[slice] = 0;

			*selector = compute_selector(encodings);
			destination += WORDS;
			}

		return 0;			// can't happen
		}

	alignas(64)  const uint32_t compress_integer_elias_gamma_simd_vb::mask_set[33][16]=
		{
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},								///< Sentinal as the selector cannot be 0.
		{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},								///< AND mask for 1-bit integers
		{0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03},								///< AND mask for 2-bit integers
		{0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07},								///< AND mask for 3-bit integers
		{0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f},								///< AND mask for 4-bit integers
		{0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f},								///< AND mask for 5-bit integers
		{0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f},								///< AND mask for 6-bit integers
		{0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f},								///< AND mask for 7-bit integers
		{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},								///< AND mask for 8-bit integers
		{0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff},						///< AND mask for 9-bit integers
		{0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff},						///< AND mask for 10-bit integers
		{0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff, 0x7ff},						///< AND mask for 11-bit integers
		{0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff, 0xfff},						///< AND mask for 12-bit integers
		{0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff, 0x1fff},			///< AND mask for 13-bit integers
		{0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff, 0x3fff},			///< AND mask for 14-bit integers
		{0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff, 0x7fff},			///< AND mask for 15-bit integers
		{0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff},			///< AND mask for 16-bit integers
		{0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff},								///< AND mask for 17-bit integers
		{0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff, 0x3ffff},								///< AND mask for 18-bit integers
		{0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff, 0x7ffff},								///< AND mask for 19-bit integers
		{0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff, 0xfffff},								///< AND mask for 20-bit integers
		{0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff, 0x1fffff},								///< AND mask for 21-bit integers
		{0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff, 0x3fffff},								///< AND mask for 22-bit integers
		{0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff},								///< AND mask for 23-bit integers
		{0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff, 0xffffff},								///< AND mask for 24-bit integers
		{0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff, 0x1ffffff},						///< AND mask for 25-bit integers
		{0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff, 0x3ffffff},						///< AND mask for 26-bit integers
		{0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff, 0x7ffffff},						///< AND mask for 27-bit integers
		{0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff, 0xfffffff},						///< AND mask for 28-bit integers
		{0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff, 0x1fffffff},			///< AND mask for 29-bit integers
		{0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff, 0x3fffffff},			///< AND mask for 30-bit integers
		{0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff},			///< AND mask for 31-bit integers
		{0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff},			///< AND mask for 32-bit integers
		};

#ifdef __AVX512F__
	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::DECODE()
		----------------------------------------------
	*/
	void compress_integer_elias_gamma_simd_vb::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m512i mask;
		const uint8_t *source = (const uint8_t *)source_as_void;
		size_t vb_length = *(const uint32_t *)source_as_void;
		const uint8_t *end_of_source = (const uint8_t *)source_as_void + source_length - vb_length;
		source += sizeof(uint32_t);
		__m512i *into = (__m512i *)decoded;
		uint64_t selector = 0;
		__m512i payload;

		while (1)
			{
			if (selector == 0)
				{
				if (source >= end_of_source)
					break;
				selector = *(uint32_t *)source;
				payload = _mm512_loadu_si512((__m256i *)(source + 4));
				source += 68;
				}

			uint32_t width = (uint32_t)find_first_set_bit(selector);
			//coverity[OVERRUN]
			mask = _mm512_loadu_si512((__m512i *)mask_set[width]);
			_mm512_storeu_si512(into, _mm512_and_si512(payload, mask));
			payload = _mm512_srli_epi32(payload, width);

			into++;
			//coverity[BAD_SHIFT]
			selector >>= width;
			}

		if (vb_length != 0)
			compress_integer_variable_byte::decode((integer *)into, vb_length, source, vb_length);
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::DECODE_WITH_WRITER()
		----------------------------------------------------------
	*/
#ifdef SIMD_JASS
	void compress_integer_elias_gamma_simd_vb::decode_with_writer(size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m512i mask;
		const uint8_t *source = (const uint8_t *)source_as_void;
		size_t vb_length = *(const uint32_t *)source_as_void;
		const uint8_t *end_of_source = (const uint8_t *)source_as_void + source_length - vb_length;
		source += sizeof(uint32_t);
		uint64_t selector = 0;
		__m512i payload;

		while (1)
			{
			if (selector == 0)
				{
				if (source >= end_of_source)
					break;
				selector = *(uint32_t *)source;
				payload = _mm512_loadu_si512((__m256i *)(source + 4));
				source += 68;
				}

			uint32_t width = (uint32_t)find_first_set_bit(selector);
			//coverity[OVERRUN]
			mask = _mm512_loadu_si512((__m512i *)mask_set[width]);
			add_rsv_d1(_mm512_and_si512(payload, mask));
			payload = _mm512_srli_epi32(payload, width);

			//coverity[BAD_SHIFT]
			selector >>= width;
			}

		if (vb_length != 0)
			compress_integer_variable_byte::decode_with_writer(vb_length, source, vb_length);
		}
#endif


#else
	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::DECODE()
		----------------------------------------------
	*/
	void compress_integer_elias_gamma_simd_vb::decode(integer *decoded, size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m256i mask;
		const uint8_t *source = (const uint8_t *)source_as_void;
		size_t vb_length = *(const uint32_t *)source_as_void;
		const uint8_t *end_of_source = (const uint8_t *)source_as_void + source_length - vb_length;
		source += sizeof(uint32_t);
		__m256i *into = (__m256i *)decoded;
		uint64_t selector = 0;
		__m256i payload1;
		__m256i payload2;

		while (1)
			{
			if (selector == 0)
				{
				if (source >= end_of_source)
					break;
				selector = *(uint32_t *)source;
				payload1 = _mm256_loadu_si256((__m256i *)(source + 4));
				payload2 = _mm256_loadu_si256((__m256i *)(source + 36));
				source += 68;
				}
			uint32_t width = (uint32_t)find_first_set_bit(selector);
			//coverity[OVERRUN]
			mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
			_mm256_storeu_si256(into, _mm256_and_si256(payload1, mask));
			_mm256_storeu_si256(into + 1, _mm256_and_si256(payload2, mask));
			payload1 = _mm256_srli_epi32(payload1, width);
			payload2 = _mm256_srli_epi32(payload2, width);

			into += 2;
			//coverity[BAD_SHIFT]
			selector >>= width;
			}

		if (vb_length != 0)
			compress_integer_variable_byte::decode((integer *)into, vb_length, source, vb_length);
		}

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::DECODE_WITH_WRITER()
		----------------------------------------------------------
	*/
#ifdef SIMD_JASS
	void compress_integer_elias_gamma_simd_vb::decode_with_writer(size_t integers_to_decode, const void *source_as_void, size_t source_length)
		{
		__m256i mask;
		const uint8_t *source = (const uint8_t *)source_as_void;
		size_t vb_length = *(const uint32_t *)source_as_void;
		const uint8_t *end_of_source = (const uint8_t *)source_as_void + source_length - vb_length;
		source += sizeof(uint32_t);
		uint64_t selector = 0;
		__m256i payload1;
		__m256i payload2;

		while (1)
			{
			if (selector == 0)
				{
				if (source >= end_of_source)
					break;
				selector = *(uint32_t *)source;
				payload1 = _mm256_loadu_si256((__m256i *)(source + 4));
				payload2 = _mm256_loadu_si256((__m256i *)(source + 36));
				source += 68;
				}

			uint32_t width = (uint32_t)find_first_set_bit(selector);
			//coverity[OVERRUN]
			mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
			add_rsv_d1(_mm256_and_si256(payload1, mask));
			add_rsv_d1(_mm256_and_si256(payload2, mask));
			payload1 = _mm256_srli_epi32(payload1, width);
			payload2 = _mm256_srli_epi32(payload2, width);

			//coverity[BAD_SHIFT]
			selector >>= width;
			}

		if (vb_length != 0)
			compress_integer_variable_byte::decode_with_writer(vb_length, source, vb_length);
		}
#endif

#endif

	/*
		COMPRESS_INTEGER_ELIAS_GAMMA_SIMD_VB::UNITTEST()
		------------------------------------------------
	*/
	void compress_integer_elias_gamma_simd_vb::unittest(void)
		{
		compress_integer_elias_gamma_simd_vb *compressor;

		compressor = new compress_integer_elias_gamma_simd_vb();
		std::vector<std::string>etc;
		compressor->init(etc);


		/*
			Variable byte only
		*/
		std::vector<uint32_t> short_sequence =
			{
			1, 2, 3
			};
		unittest_one(*compressor, short_sequence);

		/*
			SIMD + Variable byte
		*/
		std::vector<uint32_t> medium_short_sequence =
			{
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			// 1 bits
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			// 1 bits
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			// 1 bits
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			// 1 bits
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,			// 1 bits
			2, 3, 4																// 3 bits
			};
		unittest_one(*compressor, medium_short_sequence);

		/*
			SIMD only
		*/
		std::vector<uint32_t> long_short_sequence =
			{
			1, 1, 793,   1,   1, 1, 1, 1, 2, 1, 5, 3, 2, 1, 5, 63,		// 10 bits
			1, 1,   1, 793,   1, 1, 1, 1, 2, 1, 5, 3, 2, 1, 5, 63,		// 10 bits
			1, 1,   1,   1, 793, 1, 1, 1, 2, 1, 5, 3, 2, 1, 5, 63,		// 10 bits
			1, 1,   1,   1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1,		// 1 bits
			1, 1,   1,   1,   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1			// 1 bits
			};
		unittest_one(*compressor, long_short_sequence);

		/*
			All the usual tests
		*/
		compress_integer::unittest(*compressor);

		/*
			Cases that went wrong on Elias Gamma SIMD
		*/
		std::vector<uint32_t> broken_sequence =
			{
			6,10,2,1,2,1,1,1,1,2,2,1,1,14,1,1,		// 4 bits
			4,1,2,1,2,5,3,4,3,1,3,4,2,3,1,1,			// 3 bits
			6,13,5,1,2,8,4,2,5,1,1,1,2,1,1,2,		// 4 bits
			3,1,2,1,1,2,2,1,3,1,1,1,1,1,1,1,			// 2 bits
			1,2,1,1,1,1,1,1,2,1,1,1,1,1,2,3,			// 2 bits
			1,7,1,4,5,3,2,1,10,1,8,1,2,5,1,24,		// 5 bits
			1,1,1,1,1,1,1,5,5,2,2,1,3,4,5,5,			// 3 bits
			2,4,2,2,1,1,1,2,2,1,2,1,2,1,3,3,			// 3 bits
			3,7,3,2,1,1,4,5,4,1,4,8,6,1,2,1,			// 4 bits
			1,1,1,1,1,3,1,2,1,1,1,1,1,1,1,2,			// 2 bits						// 160 integers

			1,3,2,2,3,1,2,1,1,2,1,1,1,1,1,2,			// 2 bits
			9,1,1,4,5,6,1,4,2,5,4,6,7,1,1,2,			// 4 bits
			1,1,9,2,2,1,2,1,1,1,1,1,1,1,1,1,			// 4 bits
			1,1,1,1,1,1,1,6,4,1,5,7,1,1,1,1,			// 3 bits
			2,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits
			1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,			// 2 bits
			2,1,1,1,2,2,1,4,1,1,4,1,1,1,1,1,			// 3 bits
			1,1,1,1,1,2,5,3,1,3,1,1,4,1,2,1,			// 3 bits
			3,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,			// 2 bits						// 304 integers
			1,1,1,1,1,2,2,1,1,1,8,3,1,2,56,2,		// 6 bits (expand to 7)		// 320 integers

			12,1,6,70,68,25,13,44,36,22,4,95,19,5,39,8, // 7 bits
			25,14,9,8,27,6,1,1,8,11,8,3,4,1,2,8,			// 5 bits
			3,23,2,16,8,2,28,26,6,11,9,16,1,1,7,7,			// 5 bits
			45,2,33,39,20,14,2,1,8,26,1,10,12,3,16,3,		// 6 bits
			25,9,6,9,6,3,41,17,15,11,33,8,1,1,1,1			// 6 bits
			};
		unittest_one(*compressor, broken_sequence);

		std::vector<uint32_t> second_broken_sequence =
			{
			1, 1, 1, 793, 1, 1, 1, 1, 2, 1, 5, 3, 2, 1, 5, 63,		// 10 bits
			1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 5, 6, 2, 4, 1, 2,			// 3 bits
			1, 1, 1, 1, 4, 2, 1, 2, 2, 1, 1, 1, 3, 2, 2, 1,			// 3 bits
			1, 1, 2, 3, 1, 1, 8, 1, 1, 21, 2, 9, 15, 27, 7, 4,		// 5 bits
			2, 7, 1, 1, 2, 1, 1, 3, 2, 3, 1, 3, 3, 1, 2, 2,			// 3 bits
			3, 1, 3, 1, 2, 1, 2, 4, 1, 1, 3, 10, 1, 2, 1, 1,		// 4 bits
			6, 2, 1, 1, 3, 3, 7, 3, 2, 1, 2, 4, 3, 1, 2, 1,			// 3 bits <31 bits>, (expand to 4 bits)
			6, 2, 2, 1															// 3 bits
			};
		unittest_one(*compressor, second_broken_sequence);

		puts("compress_integer_elias_gamma_simd_vb::PASSED");
		}
	}
