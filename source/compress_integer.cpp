/*
	COMPRESS_INTEGER.CPP
	--------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>

#include <vector>
#include <iostream>

#include "asserts.h"
#include "compress_integer.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER::UNITTEST_ONE()
		--------------------------------
	*/
	void compress_integer::unittest_one(compress_integer &compressor, const std::vector<uint32_t> &sequence)
		{
		std::vector<uint32_t>compressed(sequence.size() * 4 + 1024);
		std::vector<uint32_t>decompressed(sequence.size() + 1024);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor.decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		if (decompressed != sequence)
			{		//LCOV_EXCL_START		// diagnostics when compression fails
			for (size_t index = 0; index < decompressed.size(); index++)
				std::cout << index << ":" << decompressed[index] << ' ' << sequence[index] << (decompressed[index] == sequence[index] ? "" : "   FAIL") << "\n";

			JASS_assert(false);
			}		//LCOV_EXCL_STOP
		}
	
	/*
		COMPRESS_INTEGER::UNITTEST()
		----------------------------
	*/
	void compress_integer::unittest(compress_integer &compressor, uint32_t starting_at)
		{
		std::vector<integer> every_case;
		size_t instance;

		/*
			Check long strings of integers of different bit lengths (from 0..2^32-1)
		*/
		for (uint32_t bitness = starting_at; bitness <= 32; bitness++)
			{
			every_case.clear();
			for (instance = 0; instance < 1024; instance++)
				every_case.push_back(static_cast<integer>((1LL << bitness) - 1));
			unittest_one(compressor, every_case);
			}

		/*
			Test the delta (d-gap) encoder.
		*/
		every_case = {4, 5, 7, 9, 12};
		std::vector<integer> d1_answer = {4, 1, 2, 2, 3};

		std::vector<integer> every_encoded;
		std::vector<integer> every_decoded;

		every_encoded.resize(every_case.size());
		auto got = d1_encode(&every_encoded[0], &every_case[0], every_case.size());
		JASS_assert(got == every_case.size());
		JASS_assert(every_encoded == d1_answer);

		every_decoded.resize(every_case.size());
		got = d1_decode(&every_decoded[0], &every_encoded[0], every_encoded.size());
		JASS_assert(got == every_case.size());
		JASS_assert(every_decoded == every_case);

		/*
			Check Dn-encoding (where n == 2)
		*/
		std::vector<integer> d2_answer = {4, 5, 3, 4, 5};
		every_encoded.resize(0);
		every_encoded.resize(every_case.size());
		got = dn_encode(&every_encoded[0], &every_case[0], every_case.size(), 2);
		JASS_assert(got == every_case.size());
		JASS_assert(every_encoded == d2_answer);

		every_decoded.resize(0);
		every_decoded.resize(every_case.size());
		got = dn_decode(&every_decoded[0], &every_encoded[0], every_encoded.size(), 2);
		JASS_assert(got == every_case.size());
		JASS_assert(every_decoded == every_case);
		}
	}
