/*
	COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD.H
	------------------------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Pack 32-bit integers into 512-bit SIMD words using elias gamma encoding and scatter/garther decoding
	@author Andrew Trotman
	@copyright 2018 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include <string.h>
#include <immintrin.h>

#include "forceinline.h"
#include "compress_integer_elias_gamma_simd.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD
		----------------------------------------------
	*/
	/*!
		@brief Pack 32-bit integers into 512-bit SIMD words using prn and carryover
	*/
	class compress_integer_gather_elias_gamma_simd: public compress_integer_elias_gamma_simd
		{
		public:
			/*
				COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD::DECODE()
				--------------------------------------------------
			*/
			/*!
				@brief Decode a sequence of integers encoded with this codex.
				@param into [out] The object to recieve the decoded integes via push_back(doc, impact).
				@param impact [in] The impact score passed to into.
				@param integers_to_decode [in] The minimum number of integers to decode (it may decode more).
				@param source [in] The encoded integers.
				@param source_length [in] The length (in bytes) of the source buffer.
			*/
			template <typename INTO>
			void decode(INTO &decoded, uint16_t impact, size_t integers_to_decode, const void *source_as_void, size_t source_length)
				{
				__m256i mask;
				const uint8_t *source = (const uint8_t *)source_as_void;
				const uint8_t *end_of_source = source + source_length;

				uint64_t selector = *(uint32_t *)source;
				__m256i payload1 = _mm256_loadu_si256((__m256i *)(source + 4));
				__m256i payload2 = _mm256_loadu_si256((__m256i *)(source + 36));
				source += 68;

				while (1)
					{
					uint32_t width = (uint32_t)find_first_set_bit(selector);
					mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
					decoded.push_back(_mm256_and_si256(payload1, mask), impact);
					decoded.push_back(_mm256_and_si256(payload2, mask), impact);
					payload1 = _mm256_srli_epi32(payload1, width);
					payload2 = _mm256_srli_epi32(payload2, width);

					selector >>= width;

					while (selector == 0)
						{
						if (source >= end_of_source)
							return;

						/*
							Save the remaining bits
						*/
						__m256i high_bits1 = payload1;
						__m256i high_bits2 = payload2;

						/*
							move on to the next word
						*/
						selector = *(uint32_t *)source;
						payload1 = _mm256_loadu_si256((__m256i *)(source + 4));
						payload2 = _mm256_loadu_si256((__m256i *)(source + 36));
						source += 68;

						/*
							get the low bits and write to memory
						*/
						width = (uint32_t)find_first_set_bit(selector);

						high_bits1 = _mm256_slli_epi32(high_bits1, width);
						high_bits2 = _mm256_slli_epi32(high_bits2, width);

						mask = _mm256_loadu_si256((__m256i *)mask_set[width]);
						decoded.push_back(_mm256_or_si256(_mm256_and_si256(payload1, mask), high_bits1), impact);
						decoded.push_back(_mm256_or_si256(_mm256_and_si256(payload2, mask), high_bits2), impact);

						payload1 = _mm256_srli_epi32(payload1, width);
						payload2 = _mm256_srli_epi32(payload2, width);

						/*
							move on to the next slector
						*/
						selector >>= width;
						}
					}
				}

			/*
				COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD::UNITTEST_ONE()
				--------------------------------------------------------
			*/
			static void unittest_one(compress_integer_gather_elias_gamma_simd &compressor, const std::vector<uint32_t> &sequence);

			/*
				COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD::UNITTEST()
				----------------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
