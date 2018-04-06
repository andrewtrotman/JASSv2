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
		std::vector<uint32_t>compressed(sequence.size() * 4);
		std::vector<uint32_t>decompressed(sequence.size() + 256);

		auto size_once_compressed = compressor.encode(&compressed[0], compressed.size() * sizeof(compressed[0]), &sequence[0], sequence.size());
		compressor.decode(&decompressed[0], sequence.size(), &compressed[0], size_once_compressed);
		decompressed.resize(sequence.size());
		if (decompressed != sequence)
			{
			for (size_t index = 0; index < decompressed.size(); index++)
				std::cout << index << ":" << decompressed[index] << ' ' << sequence[index] << (decompressed[index] == sequence[index] ? "" : "   FAIL") << "\n";

			JASS_assert(false);
			}
		}
	
	/*
		COMPRESS_INTEGER::UNITTEST()
		----------------------------
	*/
	void compress_integer::unittest(compress_integer &&compressor)
		{
		std::vector<uint32_t> every_case;
		size_t instance;

		/*
			Check long strings of integers of different bit lengths (from 0..2^32-1)
		*/
		for (uint32_t bitness = 0; bitness <= 32; bitness++)
			{
			every_case.clear();
			for (instance = 0; instance < 1024; instance++)
				every_case.push_back((1LL << bitness) - 1);
			unittest_one(compressor, every_case);
			}
		}
	}
