/*
	PRM.CPP
	-------
*/
#include <stdint.h>
#include <strings.h>
#include <immintrin.h>

#include <vector>
#include <iostream>

#include "maths.h"

static const uint32_t WORDS = 2;
static const uint32_t WORD_WIDTH = 8;

uint32_t encodings[33];

/*
	BITS_NEEDED()
	-------------
*/
inline uint32_t bits_needed(uint32_t integer)
	{
	uint32_t current_width = JASS::maths::ceiling_log2(integer);
	return JASS::maths::maximum(current_width, static_cast<decltype(current_width)>(1));
	}

uint32_t bits_to_use[] =			// bits to use (column 1 ) for bits in integer (right column)
	{
	1, //0
	1, //1
	2, //2
	3, //3
	4, //4
	5, //5
	6, //6
	8, //7
	8, //8
	10, //9
	10, //10
	16, //11
	16, //12
	16, //13
	16, //14
	16, //15
	16, //16
	32, //17
	32, //18
	32, //19
	32, //20
	32, //21
	32, //22
	32, //23
	32, //24
	32, //25
	32, //26
	32, //27
	32, //28
	32, //29
	32, //30
	32, //31
	32 //32
	};

/*
	BIN_PACK()
	----------
*/
uint32_t bin_pack(__m256i &result, const uint32_t *array, size_t elements)
	{
	uint32_t buffer[8];

	const uint32_t *end = array + elements;
	uint32_t widest = 0;

	/*
		Find the widest integer in the list
	*/
	for (const uint32_t *current = array; current < end; current++)
		widest = JASS::maths::maximum(widest, bits_needed(*current));

	uint32_t words_needed = (widest * elements + 31) / 32;

	widest = bits_to_use[widest];
	uint32_t current_word = 0;
	for (const uint32_t *current = array; current < end; current++)
		{
		buffer[current_word] = (buffer[current_word] << widest) | *current;
		current_word++;
		}

	result = _mm256_loadu_si256((__m256i *)buffer);

	return widest;
	}

alignas(32) static uint32_t static_mask_16[] = {0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff};			///< AND mask for 10-bit integers
alignas(32) static uint32_t static_mask_10[] = {0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff, 0x3ff};						///< AND mask for 10-bit integers
alignas(32) static uint32_t static_mask_8[] =  {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};								///< AND mask for 8-bit integers
alignas(32) static uint32_t static_mask_6[]  = {0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f};								///< AND mask for 6-bit integers
alignas(32) static uint32_t static_mask_5[]  = {0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f};								///< AND mask for 5-bit integers
alignas(32) static uint32_t static_mask_4[]  = {0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};								///< AND mask for 4-bit integers
alignas(32) static uint32_t static_mask_3[]  = {0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07};								///< AND mask for 3-bit integers
alignas(32) static uint32_t static_mask_2[]  = {0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};								///< AND mask for 2-bit integers
alignas(32) static uint32_t static_mask_1[]  = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};								///< AND mask for 1-bit integers

/*
	UNBIN_PACK()
	------------
*/
void unbin_pack(uint32_t *destination, __m256i const *source, uint32_t width)
	{
	const __m256i mask_16 = _mm256_loadu_si256((__m256i *)static_mask_16);
	const __m256i mask_10 = _mm256_loadu_si256((__m256i *)static_mask_10);
	const __m256i mask_8 = _mm256_loadu_si256((__m256i *)static_mask_8);
	const __m256i mask_6 = _mm256_loadu_si256((__m256i *)static_mask_6);
	const __m256i mask_5 = _mm256_loadu_si256((__m256i *)static_mask_5);
	const __m256i mask_4 = _mm256_loadu_si256((__m256i *)static_mask_4);
	const __m256i mask_3 = _mm256_loadu_si256((__m256i *)static_mask_3);
	const __m256i mask_2 = _mm256_loadu_si256((__m256i *)static_mask_2);
	const __m256i mask_1 = _mm256_loadu_si256((__m256i *)static_mask_1);

	__m256i *into = (__m256i *)destination;
	__m256i data;

	switch (width)
		{
		case 1:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 8, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 9, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 10, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 11, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 12, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 13, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 14, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 15, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 16, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 17, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 18, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 19, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 20, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 21, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 22, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 23, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 24, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 25, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 26, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 27, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 28, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 29, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 30, _mm256_and_si256(data, mask_1));
			_mm256_store_si256(into + 31, _mm256_and_si256(data, mask_1));
			into += 32;
			source++;
			break;
		case 2:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 8, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 9, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 10, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 11, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 12, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 13, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 14, _mm256_and_si256(data, mask_2));
			_mm256_store_si256(into + 15, _mm256_and_si256(data, mask_2));
			into += 16;
			source++;
			break;
		case 3:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 8, _mm256_and_si256(data, mask_3));
			_mm256_store_si256(into + 9, _mm256_and_si256(data, mask_3));
			into += 10;
			source++;
			break;
		case 4:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 6, _mm256_and_si256(data, mask_4));
			_mm256_store_si256(into + 7, _mm256_and_si256(data, mask_4));
			into += 8;
			source++;
			break;
		case 5:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_5));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_5));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_5));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_5));
			_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_5));
			_mm256_store_si256(into + 5, _mm256_and_si256(data, mask_5));
			into += 6;
			source++;
			break;
		case 6:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_6));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_6));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_6));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_6));
			_mm256_store_si256(into + 4, _mm256_and_si256(data, mask_6));
			into += 5;
			source++;
			break;
		case 7:
			break;
		case 8:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_8));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_8));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_8));
			_mm256_store_si256(into + 3, _mm256_and_si256(data, mask_8));
			into += 4;
			source++;
			break;
		case 9:
			break;
		case 10:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_10));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_10));
			_mm256_store_si256(into + 2, _mm256_and_si256(data, mask_10));
			into += 3;
			source++;
			break;
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			break;
		case 16:
			data = _mm256_loadu_si256(source);
			_mm256_store_si256(into + 0, _mm256_and_si256(data, mask_16));
			_mm256_store_si256(into + 1, _mm256_and_si256(data, mask_16));
			into += 2;
			source++;
			break;
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			break;
		case 32:
			_mm256_store_si256(into, _mm256_loadu_si256(source));
			into++;
			source++;
			break;
		}
	}



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

uint32_t packable_data[] = {1,2,2,3,4,5, 300};
size_t packable_data_size = sizeof(packable_data) / sizeof(*packable_data);

int main(void)
	{
//	word_lengths(test_set, test_set_size);

	__m256i packed;
	uint32_t width = bin_pack(packed, packable_data, packable_data_size);

	std::vector<uint32_t>into;
	into.resize(packable_data_size);

	unbin_pack(&into[0], &packed, width);

	for (size_t element = 0; element < packable_data_size; element++)
		std::cout << packable_data[element] << "->"  << into[element] << (packable_data[element] == into[element] ? "" : " WRONG") << "\n";

//	check_encoder();
//	check(4);
//	check(5);
//	check(6);
//	check(7);
	}
