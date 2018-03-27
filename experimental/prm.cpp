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
#include "compress_integer_prn_512.h"
#include "compress_integer_bitpack_256.h"
#include "compress_integer_bitpack_128.h"
#include "compress_integer_bitpack_64.h"
#include "compress_integer_bitpack_32_reduced.h"
#include "compress_integer_lyck_16.h"
#include "compress_integer_nybble_8.h"

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
	JASS::compress_integer_bitpack_128 scheme;
	scheme.unittest();	
	}
