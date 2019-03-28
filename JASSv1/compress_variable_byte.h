/*
	COMPRESS_VARIABLE_BYTE.H
	------------------------
*/
#ifndef COMPRESS_VARIABLE_BYTE_H_
#define COMPRESS_VARIABLE_BYTE_H_

#include "compress.h"

/*
	class ANT_COMPRESS_VARIABLE_BYTE
	--------------------------------
*/
class ANT_compress_variable_byte : public ANT_compress
{
public:
	ANT_compress_variable_byte() {}
	virtual ~ANT_compress_variable_byte() {}

	virtual long long compress(unsigned char *destination, long long destination_length, uint32_t *source, long long source_integers);
	virtual void decompress(uint32_t *destination, unsigned char *source, long long destination_integers);

	static inline long compress_bytes_needed(long long docno);
	static inline void compress_into(unsigned char *dest, long long docno);
} ;

/*
	ANT_COMPRESS_VARIABLE_BYTE::COMPRESS_BYTES_NEEDED()
	---------------------------------------------------
*/
inline long ANT_compress_variable_byte::compress_bytes_needed(long long docno)
{
if (docno < ((long long)1 << 7))
	return 1;
else if (docno < ((long long)1 << 14))
	return 2;
else if (docno < ((long long)1 << 21))
	return 3;
else if (docno < ((long long)1 << 28))
	return 4;
else if (docno < ((long long)1 << 35))
	return 5;
else if (docno < ((long long)1 << 42))
	return 6;
else if (docno < ((long long)1 << 49))
	return 7;
else if (docno < ((long long)1 << 56))
	return 8;
else
	return 9;
}

/*
	ANT_COMPRESS_VARIABLE_BYTE::COMPRESS_INTO()
	-------------------------------------------
*/
inline void ANT_compress_variable_byte::compress_into(unsigned char *dest, long long docno)
{
if (docno < ((long long)1 << 7))
	goto one;
else if (docno < ((long long)1 << 14))
	goto two;
else if (docno < ((long long)1 << 21))
	goto three;
else if (docno < ((long long)1 << 28))
	goto four;
else if (docno < ((long long)1 << 35))
	goto five;
else if (docno < ((long long)1 << 42))
	goto six;
else if (docno < ((long long)1 << 49))
	goto seven;
else if (docno < ((long long)1 << 56))
	goto eight;

/*
	Else we are a nine byte compressed integer
*/
	*dest++ = (docno >> 56) & 0x7F;
eight:
	*dest++ = (docno >> 49) & 0x7F;
seven:
	*dest++ = (docno >> 42) & 0x7F;
six:
	*dest++ = (docno >> 35) & 0x7F;
five:
	*dest++ = (docno >> 28) & 0x7F;
four:
	*dest++ = (docno >> 21) & 0x7F;
three:
	*dest++ = (docno >> 14) & 0x7F;
two:
	*dest++ = (docno >> 7) & 0x7F;
one:
	*dest++ = (docno & 0x7F) | 0x80;
}

#endif  /* COMPRESS_VARIABLE_BYTE_H_ */
