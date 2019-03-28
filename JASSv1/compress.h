/*
	COMPRESS.H
	----------
*/
#ifndef COMPRESS_H_
#define COMPRESS_H_

#include <stdint.h>
#define ANT_compressable_integer uint32_t
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
	virtual long long compress(unsigned char *destination, long long destination_length, uint32_t *source, long long source_integers) = 0;
	virtual void decompress(uint32_t *destination, unsigned char *source, long long destination_integers) = 0;
} ;

#endif  /* COMPRESS_H_ */

