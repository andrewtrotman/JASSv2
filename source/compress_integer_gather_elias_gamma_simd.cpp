/*
	COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD.H
	------------------------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdint.h>

#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_gather_elias_gamma_simd.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD_UNITTEST
		-------------------------------------------------------
	*/
	/*!
		@brief callback for compress_integer_gather_elias_gamma_simd::unittext()
	*/
	class compress_integer_gather_elias_gamma_simd_unittest
		{
		private:
			__m256i *into;						// pointer to where in the answer vector to put the next sequence of integers.
			uint16_t impact;					// the impact score to add to the array of accumulators

		public:
			std::vector<uint32_t> answer;	// this is where the sequence of document ids is put

		public:
			/*
				COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD_UNITTEST::COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD_UNITTEST()
				------------------------------------------------------------------------------------------------------
			*/
			/*!
				@brief Constructor
				@param size [in] The size of the decoded sequence.
			*/
			compress_integer_gather_elias_gamma_simd_unittest(uint32_t size)
				{
				answer.reserve(size + 1024);
				answer.resize(size);
				into = (__m256i *)&answer[0];
				}

			/*
				COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD_UNITTEST::SET_SCORE()
				--------------------------------------------------------------
			*/
			/*!
				@brief Remember the impact score to add to an array of documents on a push_back() call
				@param impact [in] The impact score passed by the decompressor.
			*/
		void set_score(uint16_t impact)
			{
			this->impact = impact;
			}


			/*
				COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD_UNITTEST::PUSH_BACK()
				--------------------------------------------------------------
			*/
			/*!
				@brief Add the impact score to the array of itegers
				@paraM integers [in] The sequence of document ids.
			*/
		void push_back(__m256i integers)
			{
			_mm256_storeu_si256(into, integers);
			into++;
			}
		} ;


	/*
		COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD::UNITTEST_ONE()
		--------------------------------------------------------
	*/
	void compress_integer_gather_elias_gamma_simd::unittest_one(compress_integer_gather_elias_gamma_simd &compressor, const std::vector<uint32_t> &sequence)
		{
		compress_integer_gather_elias_gamma_simd_unittest first_output(sequence.size());
		std::vector<uint32_t>compressed(sequence.size() * 4);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());

		first_output.set_score(12);
		compressor.decode(first_output, sequence.size(), &compressed[0], size_once_compressed);
		first_output.answer.resize(sequence.size());

		if (first_output.answer != sequence)
			{		//LCOV_EXCL_START		// diagnostics when compression fails
			for (size_t index = 0; index < first_output.answer.size(); index++)
				std::cout << index << ":" << first_output.answer[index] << ' ' << sequence[index] << (first_output.answer[index] == sequence[index] ? "" : "   FAIL") << "\n";

			JASS_assert(false);
			}		//LCOV_EXCL_STOP
		}

	/*
		COMPRESS_INTEGER_GATHER_ELIAS_GAMMA_SIMD::UNITTEST()
		----------------------------------------------------
	*/
	void compress_integer_gather_elias_gamma_simd::unittest(void)
		{
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

		std::vector<uint32_t> second_broken_sequence =
			{
			1, 1, 1, 793, 1, 1, 1, 1, 2, 1, 5, 3, 2, 1, 5, 63,		// 10 bits
			1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 5, 6, 2, 4, 1, 2,			// 3 bits
			1, 1, 1, 1, 4, 2, 1, 2, 2, 1, 1, 1, 3, 2, 2, 1,			// 3 bits
			1, 1, 2, 3, 1, 1, 8, 1, 1, 21, 2, 9, 15, 27, 7, 4,		// 5 bits
			2, 7, 1, 1, 2, 1, 1, 3, 2, 3, 1, 3, 3, 1, 2, 2,			// 3 bits
			3, 1, 3, 1, 2, 1, 2, 4, 1, 1, 3, 10, 1, 2, 1, 1,		// 4 bits
			6, 2, 1, 1, 3, 3, 7, 3, 2, 1, 2, 4, 3, 1, 2, 1,			// 3 bits <31 bits>, carryover 1 from next line
			6, 2, 2, 1															// 3 bits
			};

		compress_integer_gather_elias_gamma_simd compressor;
		unittest_one(compressor, broken_sequence);
		unittest_one(compressor, second_broken_sequence);

		puts("compress_integer_gather_elias_gamma_simd::PASSED");
		}
	}
