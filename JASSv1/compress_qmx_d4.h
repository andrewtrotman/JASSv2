/*
	COMPRESS_QMX_D4.H
	-----------------
	A variant of QMX that uses the D4 approach to computing document deltas.  This code computes the deltas and the compresses.  On decompression it un-deltas back into Doc-IDs.
	That is: PASS THE DOC-IDs TO THIS CODE, DO NOT PASS DELTA ENCODED DOC-IDs.
	For details on D4, see: Daniel Lemire, Leonid Boytsov, Nathan Kurz, SIMD Compression and the Intersection of Sorted Integers, arXiv: 1401.6399, 2014 http://arxiv.org/abs/1401.6399

*/
#ifndef COMPRESS_QMX_D4_H_
#define COMPRESS_QMX_D4_H_

#include "compress.h"

/*
	class ANT_COMPRESS_QMX_D4
	-------------------------
*/
class ANT_compress_qmx_d4 : public ANT_compress
{
private:
	uint8_t *length_buffer;
	uint64_t length_buffer_length;
	uint32_t *deltas_buffer;

public:
	ANT_compress_qmx_d4();
	virtual ~ANT_compress_qmx_d4();

	virtual void encodeArray(const uint32_t *in, uint64_t len, uint32_t *out, uint64_t *nvalue);
	virtual void decodeArray(const uint32_t *in, uint64_t len, uint32_t *out, uint64_t nvalue);

	virtual long long compress(unsigned char *destination, long long destination_length, uint32_t *source, long long source_integers)
		{
		uint64_t answer;
		encodeArray(source, source_integers, (uint32_t *)destination, &answer);
		return answer;
		}

	virtual void decompress(uint32_t *destination, unsigned char *source, long long destination_integers)
		{}
} ;

#endif

