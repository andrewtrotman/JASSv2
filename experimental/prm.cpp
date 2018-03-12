/*
	PRM.CPP
	-------
*/
#include <stdint.h>
#include <strings.h>

#include <iostream>

size_t power_of_2[] =
	{
	1,
	2,
	4,
	8,
	16,
	32,
	64,
	128,
	256,
	512,
	1024,
	2048,
	4096,
	8192,
	16384,
	32768,
	65536,
	131072,
	262144,
	524288,
	1048576,
	2097152,
	4194304,
	8388608,
	16777216,
	33554432,
	67108864,
	134217728,
	268435456,
	536870912,
	1073741824,
	2147483648,
	4294967296
	};


//n  z
//1->0
//2->1
//3->2

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
	check_encoder();
	check(4);
	check(5);
	check(6);
	check(7);
	}
