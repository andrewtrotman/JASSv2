/*
	PRM.CPP
	-------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>
#include <strings.h>
#include <immintrin.h>

#include <vector>
#include <iostream>

#include "maths.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_64.h"
#include "compress_integer_bitpack_32_reduced.h"
#include "compress_integer_lyck_16.h"
#include "compress_integer_nybble_8.h"

static const uint32_t WORDS = 2;
static const uint32_t WORD_WIDTH = 8;

/*
	COMPUTE_SELECTOR()
	------------------
*/
uint32_t compute_selector(uint8_t *encodings)
	{
	uint32_t value = 0;
	int current;

	for (current = 32; current >  0; current--)
		if (encodings[current] != 0)
			break;

	for (;current >=  0; current--)
		{
		size_t number_of_0s = encodings[current];
		value <<= number_of_0s;
		value |= 1 << (number_of_0s - 1);
		}

	return value;
	}

/*
	ESTIMATE()
	----------
*/
uint32_t estimate(uint32_t *array, size_t elements)
	{
	uint32_t *current;
	uint32_t width = 0;

	for (current = array; current < array + elements; current++)
		{
		uint32_t current_width = JASS::maths::ceiling_log2(*current);
		current_width = JASS::maths::maximum(current_width, static_cast<decltype(current_width)>(1));

		width += current_width;
		if (width > WORD_WIDTH)
			break;
		}
	return current - array;
	}

/*
	TEST_ENCODING()
	---------------
*/
bool test_encoding(uint8_t *encodings, uint32_t *array, size_t elements_in_array, size_t elements)
	{
	uint32_t width = 0;
	uint32_t *current;

	for (current = array; current < array + elements; current++)
		{
		uint32_t max_width = 0;
		for (uint32_t word = 0; word < WORDS; word++)
			{
			uint32_t current_width;
			if ((current + word * elements) >= array + elements_in_array)
				current_width = 0;
			else
				{
				current_width = JASS::maths::ceiling_log2(*(current + word * elements));
				current_width = JASS::maths::maximum(current_width, static_cast<decltype(current_width)>(1));
				}
			max_width = JASS::maths::maximum(max_width, current_width);
			}
		width += max_width;
		if (width > WORD_WIDTH)
			{
			encodings[current - array] = 0;
			return false;
			}
		encodings[current - array] = max_width;
		}
	encodings[current - array] = 0;
	return true;
	}

/*
	ENCODE()
	--------
*/
void encode(uint32_t *array, size_t elements)
	{
	uint8_t encodings[33];
	uint32_t *current;
	uint32_t integers_to_encode;

	/*
		Get the initial guess
	*/
	uint32_t initial_guess = estimate(array, elements);
	std::cout << "initial:" << initial_guess << "\n";

	/*
		Linear search for the best answer
	*/
	for (integers_to_encode = initial_guess; integers_to_encode > 1; integers_to_encode--)
		if (test_encoding(encodings, array, elements, integers_to_encode))
			break;

	/*
		Dump the answer
	*/
	for (size_t word = 0; word < WORDS; word++)
		{
		for (current = array + word * integers_to_encode; current < array + (word + 1) * integers_to_encode; current++)
			if (current < array + elements)
				printf("%08X ", *current);
			else
				printf("     (0) ");

		printf("\n");
		}

	printf("=\n");

	for (current = array; current < array + integers_to_encode; current++)
		printf("%08u ", encodings[current - array]);
	printf("\n");

	compute_selector(encodings);
	}

/*
	FIND_FIRST_SET_BIT()
	--------------------
*/
uint32_t find_first_set_bit(uint32_t value)
	{
	#ifdef _MSC_VER
		unsigned long result;
		_BitScanForward(&result, value);
		return result;
	#else
		return ffs(value);
	#endif
	}

/*
	DECODE()
	--------
*/
void decode(uint32_t value)
	{
	std::cout << value << "->";
	do
		{
		int shift = find_first_set_bit(value);
		std::cout << shift << "\n   ";
		value >>= shift;
		}
	while (value != 0);
	std::cout << "\n";
	}

uint32_t packable_data[] = {1,2,3,4,5,6,7,300,1,2,3,4,5,6,7,300,1,2,3,4,5,6,7,300,1,2,3,4,5,6,7,300,1,2,3,4,5,6,7,300,};
size_t packable_data_size = sizeof(packable_data) / sizeof(*packable_data);


//uint32_t test_set[] = {0x01, 0x0F, 0x01, 0xFFFF, 0x01, 0xFF, 0xFF, 0xFF, 0x0FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, };
//uint32_t test_set[] = {0x01, 0xFFFF, 0x01, 0xFFFF, 0x01, 0xFFF, 0xFF, 0xFF, 0x0FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, };
//uint32_t test_set[] = {1,0xf,1,1,1,1,0x0f,1,1,1,1,1,1,1,1,1,1,0x01, 0x0F, 0x01, 0xFFFF, 0x01, 0xFF, 0xFF, 0xFF, 0x0FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, };
uint32_t test_set[] = {1,0xf,1,1,1,1};
//uint32_t test_set[] = {1,0xf};
uint32_t test_set_size = sizeof(test_set) / sizeof(*test_set);


int main(void)
	{
	JASS::compress_integer_bitpack_256 scheme_256;
	scheme_256.unittest();

	JASS::compress_integer_bitpack_128 scheme_128;
	scheme_128.unittest();

	JASS::compress_integer_bitpack_64 scheme_64;
	scheme_64.unittest();

	JASS::compress_integer_bitpack_32_reduced scheme_32;
	scheme_32.unittest();

	JASS::compress_integer_lyck_16 scheme_16;
	scheme_16.unittest();

	JASS::compress_integer_nybble_8 scheme_8;
	scheme_8.unittest();
	}
