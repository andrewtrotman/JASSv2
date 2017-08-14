/*
	CHECKSUM.CPP
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>

#include <ios>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <iterator>

#include "asserts.h"
#include "checksum.h"
#include "unittest_data.h"

namespace JASS
	{
	/*
		CHECKSUM::FLETCHER_16()
		-----------------------
	*/
	template <typename TYPE>
	uint16_t checksum::fletcher_16(TYPE &start, TYPE &end)
		{
		/*
			This is a re-written version of the Wikipedia example (https://en.wikipedia.org/wiki/Fletcher%27s_checksum).
			Re-written to avoid the array dereference. This is the "slow" version, but its a lot clearer than the fast version,
			in this case clarity wins over speed.
		*/
		uint16_t sum_1 = 0;
		uint16_t sum_2 = 0;

		while (start != end)
			{
			sum_1 = (sum_1 + *start) % 255;
			sum_2 = (sum_2 + sum_1) % 255;

			++start;
			}

		return (sum_2 << 8) | sum_1;
		}

	/*
		CHECKSUM::FLETCHER_16()
		-----------------------
	*/
	uint16_t checksum::fletcher_16(const void *address, size_t length)
		{
		const uint8_t *start = (const uint8_t *)address;
		const uint8_t *end = (const uint8_t *)address + length;
		return fletcher_16(start, end);
		}

	/*
		CHECKSUM::FLETCHER_16()
		-----------------------
	*/
	uint16_t checksum::fletcher_16(std::istream &stream)
		{
		/*
			Get the stream's flags
		*/
		auto flags = stream.flags(); 

		/*
			set the noskipws so that it also checksums the whitespaces
		*/
		stream >> std::noskipws;

		/*
			Compute the checksum
		*/
		std::istream_iterator<uint8_t> byte(stream);
		std::istream_iterator<uint8_t> end;
		auto checksum = fletcher_16(byte, end);

		/*
			Reset the stream's flags.
		*/
		stream.flags(flags); 

		return checksum;
		}

	/*
		CHECKSUM::FLETCHER_16_FILE()
		----------------------------
	*/
	uint16_t checksum::fletcher_16_file(const std::string &filename)
		{
		std::ifstream file(filename, std::ios::binary);
		return fletcher_16(file);
		}

	/*
		CHECKSUM::UNITTEST()
		--------------------
	*/
	void checksum::unittest(void)
		{
		/*
			Check the Fletcher 16-bit checksum of 8-bit data against the online calculator here: http://www.nitrxgen.net/hashgen/
		*/
		uint16_t checksum;

		/*
			Zero length string should produce 0
		*/
		checksum = checksum::fletcher_16("", 0);
		JASS_assert(checksum == 0x0000);


		/*
			String of length 1
		*/
		checksum = checksum::fletcher_16("a", 1);
		JASS_assert(checksum == 0x6161);

		checksum = checksum::fletcher_16("z", 1);
		JASS_assert(checksum == 0x7A7A);

		/*
			Long string
		*/
		checksum = checksum::fletcher_16((const uint8_t *)(unittest_data::ten_documents.c_str()), (int)unittest_data::ten_documents.size());
		JASS_assert(checksum == 0xF7DE);

		/*
			C++ Strings.  Since this just calls the C version, checking only one string suffices.
		*/
		checksum = checksum::fletcher_16(unittest_data::ten_documents);
		JASS_assert(checksum == 0xF7DE);

		/*
			Check the istream version
		*/
		std::istringstream stream(unittest_data::ten_documents);
		checksum = checksum::fletcher_16(stream);
		JASS_assert(checksum == 0xF7DE);
		
		/*
			Passed!
		*/
		puts("checksum::PASSED");
		}
	}
