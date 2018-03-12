/*
	PRM.CPP
	-------
*/
#include <stdint.h>
#include <strings.h>

#include <iostream>

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
