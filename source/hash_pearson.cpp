/*
	HASH_PEARSON.CPP
	----------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Pearson's random hash

	Pearson, Peter K., Fast hashing of variable-length text strings,
	CACM Volume 33, No. 6 (June 1990),  pp. 677-680
*/

#include "slice.h"
#include "asserts.h"
#include "hash_pearson.h"

namespace JASS
	{
	
	/*
		Table of random numbers (each unique) for Pearson's random hash function.
	*/
	uint8_t hash_pearson::random_walk[] =
		{
		0xE0, 0x16, 0xC0, 0x9E, 0xC2, 0xBD, 0x48, 0xB9, 0x6B, 0x43, 0x93, 0x28, 0x06, 0xBC, 0x70, 0x88,
		0xCA, 0xB7, 0xF1, 0x1D, 0x51, 0x7D, 0x96, 0x5C, 0x53, 0x34, 0xC6, 0x67, 0x61, 0x69, 0x95, 0x4C,
		0x07, 0x8E, 0x8C, 0x52, 0xB4, 0xB6, 0x63, 0x42, 0x40, 0x68, 0xC7, 0x11, 0x2F, 0x99, 0xF4, 0xA0,
		0x2C, 0x6F, 0xEC, 0x89, 0x77, 0x81, 0x3E, 0xE3, 0xD3, 0xF9, 0xF2, 0xCF, 0xC8, 0x85, 0xDF, 0x90,
		0x94, 0x9D, 0x65, 0x1A, 0x30, 0x5A, 0xD1, 0x26, 0x50, 0x01, 0xA5, 0x23, 0x3F, 0x29, 0x4E, 0xDD,
		0x7F, 0x58, 0x8A, 0xEF, 0xDB, 0x1B, 0x83, 0xE1, 0x4B, 0x75, 0x39, 0x32, 0x82, 0x1E, 0x24, 0xE9,
		0x0A, 0xDA, 0x62, 0x12, 0xF3, 0x9F, 0x6D, 0x3A, 0x7E, 0x0E, 0xE4, 0xCE, 0xA4, 0x57, 0xA9, 0x78,
		0x2E, 0x27, 0x8F, 0xD2, 0x41, 0x18, 0x4F, 0xEE, 0x7C, 0xAA, 0x92, 0xFA, 0xB3, 0xEB, 0x8B, 0xAD,
		0x86, 0x20, 0x15, 0x31, 0x46, 0x38, 0xED, 0xD5, 0x47, 0x14, 0x35, 0x84, 0x5D, 0x04, 0xAB, 0x79,
		0xFC, 0xFD, 0x33, 0x3D, 0xD0, 0x59, 0x56, 0x22, 0x00, 0xD6, 0x66, 0x55, 0xF0, 0xC1, 0xBA, 0xF8,
		0x9B, 0xA6, 0xC5, 0xBE, 0x74, 0x4A, 0x0C, 0x80, 0x05, 0x1C, 0x5B, 0x6E, 0xDC, 0x2B, 0x91, 0x0B,
		0x4D, 0x6C, 0xCD, 0xA3, 0xCB, 0x7A, 0xAE, 0x72, 0x9A, 0x3B, 0xD8, 0xA2, 0xBB, 0x73, 0x0D, 0xA1,
		0xB8, 0x02, 0x3C, 0x98, 0x9C, 0xE2, 0xFE, 0x7B, 0x64, 0xAC, 0x2D, 0xFB, 0xCC, 0x19, 0xD7, 0xF7,
		0xB5, 0xC4, 0x17, 0xEA, 0xB1, 0x5F, 0xC9, 0x8D, 0x60, 0xE7, 0x10, 0x54, 0x25, 0xDE, 0x1F, 0x36,
		0xA8, 0xE5, 0x87, 0x97, 0xAF, 0xD4, 0xD9, 0x45, 0x13, 0x08, 0x76, 0xE6, 0xF6, 0x5E, 0x21, 0xB2,
		0x49, 0xBF, 0xB0, 0xC3, 0x71, 0xFF, 0x2A, 0x6A, 0x37, 0x44, 0x0F, 0x03, 0xF5, 0xA7, 0x09, 0xE8
		};

	/*
		HASH_PEARSON::UNITTEST()
		------------------------
	*/
	void hash_pearson::unittest(void)
		{
		/*
			Check success states
		*/
		size_t eight = hash_pearson::hash_8("here", 4);
		size_t sixteen = hash_pearson::hash_16("here there", 10);
		size_t twentyfour = hash_pearson::hash_24("here there and", 14);
		size_t thirtytwo = hash_pearson::hash_32("here there and everywhere", 25);
		
		JASS_assert(eight == 83);
		JASS_assert(sixteen == 49465);
		JASS_assert(twentyfour == 13710892);
		JASS_assert(thirtytwo == 1803130861);

		size_t eight_a = hash_pearson::hash<8>("here", 4);
		size_t sixteen_a = hash_pearson::hash<16>("here there", 10);
		size_t twentyfour_a = hash_pearson::hash<24>("here there and", 14);
		size_t thirtytwo_a = hash_pearson::hash<32>("here there and everywhere", 25);

		JASS_assert(eight_a == 83);
		JASS_assert(sixteen_a == 49465);
		JASS_assert(twentyfour_a == 13710892);
		JASS_assert(thirtytwo_a == 1803130861);

		/*
			Check odd lengths
		*/
		eight = hash_pearson::hash_8("here", 4);
		JASS_assert(eight == 83);
		sixteen = hash_pearson::hash_16("here there", 10);
		JASS_assert(sixteen == 49465);
		sixteen = hash_pearson::hash_16("here there", 9);
		JASS_assert(sixteen == 63545);
		twentyfour = hash_pearson::hash_24("here there and", 12);
		JASS_assert(twentyfour == 13744990);
		twentyfour = hash_pearson::hash_24("here there and", 11);
		JASS_assert(twentyfour == 2603870);
		twentyfour = hash_pearson::hash_24("here there and", 10);
		JASS_assert(twentyfour == 2595934);
		thirtytwo = hash_pearson::hash_32("here there and everywhere", 24);
		JASS_assert(thirtytwo == 1803130851);
		thirtytwo = hash_pearson::hash_32("here there and everywhere", 23);
		JASS_assert(thirtytwo == 2054789091);
		thirtytwo = hash_pearson::hash_32("here there and everywhere", 22);
		JASS_assert(thirtytwo == 2062194659);
		thirtytwo = hash_pearson::hash_32("here there and everywhere", 21);
		JASS_assert(thirtytwo == 2062191587);

		slice string = "here";
		eight = hash_pearson::hash<8>(string);
		sixteen = hash_pearson::hash<16>(string);
		twentyfour = hash_pearson::hash<24>(string);
		thirtytwo = hash_pearson::hash<32>(string);

		JASS_assert(eight == 83);
		JASS_assert(sixteen == 3846);
		JASS_assert(twentyfour == 9412455);
		JASS_assert(thirtytwo == 2676989822);

		puts("hash_pearson::PASSED");
		}
	}

#ifdef NEVER
	/*
		Use this program to generate the random table above.  If it generates a different random
		table then the results of a hash will be different, so it is not recommended to be run.
	*/
	#include <stdio.h>
	#include <stdlib.h>
	#include <time.h>

	unsigned char table[0x100];

	int main(void)
	{
	int x;

	srand((unsigned)time(NULL));

	for (x = 0; x < 0x100; x++)
		table[x] = x;

	for (x = 0; x < 0x100; x++)
		{
		int val = (int)((double)rand() / (double)RAND_MAX * (double)(0x100 - x));
		if (x + val > 0x100)
			printf("oops\n");
		int tmp = table[x];
		table[x] = table[x + val];
		table[x + val] = tmp;
		}

	printf("uint8_t hash_pearson::random_walk[] = \n{\n");
	for (x = 0; x < 16; x++)
		{
		for (int y = 0; y < 16; y++)
			printf("0x%02X, ", table[x * 16 + y]);
		printf("\n");
		}
	printf("\n};\n");

	return 0;
	}

#endif
