/*
	PRM.CPP
	-------
*/
#include <stdint.h>
#include <strings.h>

#include <iostream>

#include "maths.h"

static const uint32_t WORDS = 2;
static const uint32_t WORD_WIDTH = 8;

uint32_t encodings[33];

uint32_t encode(void)
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
		std::cout << number_of_0s << "\n";
		printf(" %X\n", value);
		}

	std::cout << "RESULT:" << value << "\n";
	return value;
	}

//uint32_t test_set[] = {0x01, 0x0F, 0x01, 0xFFFF, 0x01, 0xFF, 0xFF, 0xFF, 0x0FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, };
//uint32_t test_set[] = {0x01, 0xFFFF, 0x01, 0xFFFF, 0x01, 0xFFF, 0xFF, 0xFF, 0x0FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, };
//uint32_t test_set[] = {1,0xf,1,1,1,1,0x0f,1,1,1,1,1,1,1,1,1,1,0x01, 0x0F, 0x01, 0xFFFF, 0x01, 0xFF, 0xFF, 0xFF, 0x0FFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, };
uint32_t test_set[] = {1,0xf,1,1,1,1};
//uint32_t test_set[] = {1,0xf};
uint32_t test_set_size = sizeof(test_set) / sizeof(*test_set);

uint32_t best_one(uint32_t *array, size_t elements)
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

bool check_one(uint32_t *encodings, uint32_t *array, size_t elements_in_array, size_t elements)
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

void word_lengths(uint32_t *array, size_t elements)
	{
	uint32_t *current;
	uint32_t integers_to_encode;

	/*
		Get the initial guess
	*/
	uint32_t initial_guess = best_one(array, elements);
	std::cout << "initial:" << initial_guess << "\n";

	/*
		Linear search for the best answer
	*/
	for (integers_to_encode = initial_guess; integers_to_encode > 1; integers_to_encode--)
		if (check_one(encodings, array, elements, integers_to_encode))
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

	encode();
	}

void check_encoder(void)
	{
	std::fill(encodings, encodings + 33, 0);
	encodings[0] = 2;
	encodings[1] = 1;
	encodings[2] = 0;
	encodings[3] = 0;
	encode();
	std::cout << "--\n";
	}

int decode(uint32_t value)
	{
	return ffs(value);
	}

void check(uint32_t value)
	{
	std::cout << value << "->";
	do
		{
		int shift = ffs(value);
		std::cout << shift << "\n   ";
		value >>= shift;
		}
	while (value != 0);
	std::cout << "\n";
	}

int main(void)
	{
	word_lengths(test_set, test_set_size);

//	check_encoder();
//	check(4);
//	check(5);
//	check(6);
//	check(7);
	}
