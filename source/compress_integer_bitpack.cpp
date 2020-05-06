/*
	COMPRESS_INTEGER_BITPACK.CPP
	----------------------------
	Copyright (c) 2018 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdint.h>

#include "compress_integer_bitpack.h"

namespace JASS
	{
	/*
		COMPRESS_INTEGER_BITPACK::BITS_TO_USE_COMPLETE[]
		------------------------------------------------
		Given the number of bits needed to store the integers, return the actual width to use.  This
		happens when, for example, you can get away with 9 bits, but since 9 * 3 = 27 and 10 * 3 = 30, you
		may as well use 10 bits.
	*/
	const uint32_t compress_integer_bitpack::bits_to_use_complete[] =			///< bits to use (column 1 ) for bits in integer (right column)
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
		32  //32
		};

	/*
		COMPRESS_INTEGER_BITPACK::SELECTOR_TO_USE_COMPLETE[]
		----------------------------------------------------
		Given the width in bits, which selector should be used?  This is used to ensure
		a switch() statement has all the entries 0..n with no gaps.
	*/
	const uint32_t compress_integer_bitpack::selector_to_use_complete[] =			///< selector to use (column 1 ) for bits in integer (right column)
		{
		0, //0
		0, //1
		1, //2
		2, //3
		3, //4
		4, //5
		5, //6
		6, //7
		6, //8
		7, //9
		7, //10
		8, //11
		8, //12
		8, //13
		8, //14
		8, //15
		8, //16
		9, //17
		9, //18
		9, //19
		9, //20
		9, //21
		9, //22
		9, //23
		9, //24
		9, //25
		9, //26
		9, //27
		9, //28
		9, //29
		9, //30
		9, //31
		9  //32
		};
	}

