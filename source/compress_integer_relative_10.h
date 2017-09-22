/*
	COMPRESS_RELATIVE10.H
	---------------------
*/
#ifndef __COMPRESS_RELATIVE10_H__
#define __COMPRESS_RELATIVE10_H__

#include "compress_simple9.h"

/*
	class ANT_COMPRESS_RELATIVE10
	-----------------------------
*/
class ANT_compress_relative10 : public ANT_compress_simple9
{
protected:
	class ANT_compress_relative10_lookup
	{
	public:
		long long numbers;
		long bits;
		long mask;
		long transfer_array[10];
		long relative_row[4];
	} ;
protected:
	static ANT_compress_relative10_lookup relative10_table[];
	static long bits_to_use10[];
	static long table_row10[];

public:
	ANT_compress_relative10() {}
	virtual ~ANT_compress_relative10() {}

	virtual long long compress(unsigned char *destination, long long destination_length, ANT_compressable_integer *source, long long source_integers);
	virtual void decompress(ANT_compressable_integer *destination, unsigned char *source, long long destination_integers);
} ;

#endif  /* __COMPRESS_RELATIVE10_H__ */
