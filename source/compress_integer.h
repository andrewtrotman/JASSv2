/*
	COMPRESS_INTEGER.H
	------------------
*/
#pragma once

#include <stdint.h>

typedef uint32_t ANT_compressable_integer;

/*
	class ANT_COMPRESS
	------------------
*/
class ANT_compress
{
public:
	ANT_compress() {}
	virtual ~ANT_compress() {}

	/*
		destination_length is in bytes.  source_integers is in units of integers, returns the length in bytes
	*/
	virtual long long compress(unsigned char *destination, long long destination_length, ANT_compressable_integer *source, long long source_integers) = 0;
	virtual void decompress(ANT_compressable_integer *destination, unsigned char *source, long long destination_integers) = 0;
} ;
