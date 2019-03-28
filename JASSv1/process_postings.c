#include "process_postings.h"

#define ALIGN_16 __attribute__ ((aligned (16)))
ALIGN_16 uint32_t *CI_decompressed_postings;
ALIGN_16 uint8_t *postings;					// the postings themselves


/*
	CIT_PROCESS_LIST_COMPRESSED_ELIAS_GAMMA_SIMD()
	----------------------------------------------
*/
JASS::compress_integer_elias_gamma_simd egs_decoder;
void CIt_process_list_compressed_elias_gamma_simd(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t sum, *finish, *current;

egs_decoder.decode(CI_decompressed_postings, integers, (uint32_t *)source, end - source);

sum = 0;
current = CI_decompressed_postings;
finish = current + integers;
while (current < finish)
	{
	sum += *current++;
	add_rsv(sum, impact);
	}
}
/*
	CIT_PROCESS_LIST_COMPRESSED_ELIAS_DELTA_SIMD()
	----------------------------------------------
*/
JASS::compress_integer_elias_delta_simd eds_decoder;
void CIt_process_list_compressed_elias_delta_simd(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t sum, *finish, *current;

eds_decoder.decode(CI_decompressed_postings, integers, (uint32_t *)source, end - source);

sum = 0;
current = CI_decompressed_postings;
finish = current + integers;
while (current < finish)
	{
	sum += *current++;
	add_rsv(sum, impact);
	}
}



/*
	CIT_PROCESS_LIST_COMPRESSED_VBYTE()
	-----------------------------------
*/
void CIt_process_list_compressed_vbyte(uint8_t *doclist, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t doc, sum;

sum = 0;
for (uint8_t *i = doclist; i < end;)
	{
	if (*i & 0x80)
		doc = *i++ & 0x7F;
	else
		{
		doc = *i++;
		while (!(*i & 0x80))
		   doc = (doc << 7) | *i++;
		doc = (doc << 7) | (*i++ & 0x7F);
		}
	sum += doc;
	
	add_rsv(sum, impact);
	}
}

/*
	CIT_PROCESS_LIST_DECOMPRESS_THEN_PROCESS()
	------------------------------------------
*/
void CIt_process_list_decompress_then_process(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t doc, sum;
uint32_t *integer, *destination = CI_decompressed_postings;

while (source < end)
	if (*source & 0x80)
		*destination++ = *source++ & 0x7F;
	else
		{
		*destination = *source++;
		while (!(*source & 0x80))
		   *destination = (*destination << 7) | *source++;
		*destination = (*destination << 7) | (*source++ & 0x7F);
		destination++;
		}

sum = 0;
integer = CI_decompressed_postings;
while (integer < destination)
	{
	sum += *integer++;
	add_rsv(sum, impact);
	}
}

/*
	CIT_PROCESS_LIST_NOT_COMPRESSED()
	---------------------------------
*/
void CIt_process_list_not_compressed(uint8_t *doclist, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t *i;

for (i = (uint32_t *)doclist; i < (uint32_t *)end; i++)
	add_rsv(*i, impact);
}

/*
	CIT_PROCESS_LIST_COMPRESSED_QMX()
	---------------------------------
*/
ANT_compress_qmx qmx_decoder;
void CIt_process_list_compressed_qmx(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t sum, *finish, *current;

qmx_decoder.decodeArray((uint32_t *)source, end - source, CI_decompressed_postings, integers);

sum = 0;
current = CI_decompressed_postings;
finish = current + integers;
while (current < finish)
	{
	sum += *current++;
	add_rsv(sum, impact);
	}
}

/*
	CIT_PROCESS_LIST_COMPRESSED_QMX_D4()
	------------------------------------
*/
ANT_compress_qmx_d4 qmx_d4_decoder;
void CIt_process_list_compressed_qmx_d4(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t sum, *finish, *current;

qmx_d4_decoder.decodeArray((uint32_t *)source, end - source, CI_decompressed_postings, integers);

current = CI_decompressed_postings;
finish = current + integers;
while (current < finish)
	add_rsv(*current++, impact);
}

/*
	CIT_PROCESS_LIST_COMPRESSED_QMX_D0()
	------------------------------------
*/
void CIt_process_list_compressed_qmx_d0(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t sum, *finish, *current;

qmx_decoder.decodeArray((uint32_t *)source, end - source, CI_decompressed_postings, integers);

current = CI_decompressed_postings;
finish = current + integers;
while (current < finish)
	add_rsv(*current++, impact);
}

/*
	CIT_PROCESS_LIST_COMPRESSED_SIMPLE8B_ATIRE()
	--------------------------------------------
*/
ANT_compress_simple8b simple8b_decoder;
void CIt_process_list_compressed_simple8b_ATIRE(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint32_t sum, *finish, *current;

simple8b_decoder.decompress(CI_decompressed_postings, source, integers);

sum = 0;
current = CI_decompressed_postings;
finish = current + integers;
while (current < finish)
	{
	sum += *current++;
	add_rsv(sum, impact);
	}
}

/*
	CIT_PROCESS_LIST_COMPRESSED_SIMPLE8B()
	--------------------------------------
*/
void CIt_process_list_compressed_simple8b(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers)
{
uint64_t *compressed_sequence = (uint64_t *)source;
uint32_t mask_type, sum;
uint64_t value;

sum = 0;
while (compressed_sequence < (uint64_t *)end)
	{
	// Load next compressed int, pull out the mask type used, shift to the values
	value = *compressed_sequence++;
	mask_type = value & 0xF;
	value >>= 4;

	// Unrolled loop to enable pipelining
	switch (mask_type)
		{
		case 0x0:
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			break;
		case 0x1:
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			sum += 1; add_rsv(sum, impact);
			break;
		case 0x2:
			sum += value & 0x1; add_rsv(sum, impact);
			sum += (value >> 1) & 0x1; add_rsv(sum, impact);
			sum += (value >> 2) & 0x1; add_rsv(sum, impact);
			sum += (value >> 3) & 0x1; add_rsv(sum, impact);
			sum += (value >> 4) & 0x1; add_rsv(sum, impact);
			sum += (value >> 5) & 0x1; add_rsv(sum, impact);
			sum += (value >> 6) & 0x1; add_rsv(sum, impact);
			sum += (value >> 7) & 0x1; add_rsv(sum, impact);
			sum += (value >> 8) & 0x1; add_rsv(sum, impact);
			sum += (value >> 9) & 0x1; add_rsv(sum, impact);
			sum += (value >> 10) & 0x1; add_rsv(sum, impact);
			sum += (value >> 11) & 0x1; add_rsv(sum, impact);
			sum += (value >> 12) & 0x1; add_rsv(sum, impact);
			sum += (value >> 13) & 0x1; add_rsv(sum, impact);
			sum += (value >> 14) & 0x1; add_rsv(sum, impact);
			sum += (value >> 15) & 0x1; add_rsv(sum, impact);
			sum += (value >> 16) & 0x1; add_rsv(sum, impact);
			sum += (value >> 17) & 0x1; add_rsv(sum, impact);
			sum += (value >> 18) & 0x1; add_rsv(sum, impact);
			sum += (value >> 19) & 0x1; add_rsv(sum, impact);
			sum += (value >> 20) & 0x1; add_rsv(sum, impact);
			sum += (value >> 21) & 0x1; add_rsv(sum, impact);
			sum += (value >> 22) & 0x1; add_rsv(sum, impact);
			sum += (value >> 23) & 0x1; add_rsv(sum, impact);
			sum += (value >> 24) & 0x1; add_rsv(sum, impact);
			sum += (value >> 25) & 0x1; add_rsv(sum, impact);
			sum += (value >> 26) & 0x1; add_rsv(sum, impact);
			sum += (value >> 27) & 0x1; add_rsv(sum, impact);
			sum += (value >> 28) & 0x1; add_rsv(sum, impact);
			sum += (value >> 29) & 0x1; add_rsv(sum, impact);
			sum += (value >> 30) & 0x1; add_rsv(sum, impact);
			sum += (value >> 31) & 0x1; add_rsv(sum, impact);
			sum += (value >> 32) & 0x1; add_rsv(sum, impact);
			sum += (value >> 33) & 0x1; add_rsv(sum, impact);
			sum += (value >> 34) & 0x1; add_rsv(sum, impact);
			sum += (value >> 35) & 0x1; add_rsv(sum, impact);
			sum += (value >> 36) & 0x1; add_rsv(sum, impact);
			sum += (value >> 37) & 0x1; add_rsv(sum, impact);
			sum += (value >> 38) & 0x1; add_rsv(sum, impact);
			sum += (value >> 39) & 0x1; add_rsv(sum, impact);
			sum += (value >> 40) & 0x1; add_rsv(sum, impact);
			sum += (value >> 41) & 0x1; add_rsv(sum, impact);
			sum += (value >> 42) & 0x1; add_rsv(sum, impact);
			sum += (value >> 43) & 0x1; add_rsv(sum, impact);
			sum += (value >> 44) & 0x1; add_rsv(sum, impact);
			sum += (value >> 45) & 0x1; add_rsv(sum, impact);
			sum += (value >> 46) & 0x1; add_rsv(sum, impact);
			sum += (value >> 47) & 0x1; add_rsv(sum, impact);
			sum += (value >> 48) & 0x1; add_rsv(sum, impact);
			sum += (value >> 49) & 0x1; add_rsv(sum, impact);
			sum += (value >> 50) & 0x1; add_rsv(sum, impact);
			sum += (value >> 51) & 0x1; add_rsv(sum, impact);
			sum += (value >> 52) & 0x1; add_rsv(sum, impact);
			sum += (value >> 53) & 0x1; add_rsv(sum, impact);
			sum += (value >> 54) & 0x1; add_rsv(sum, impact);
			sum += (value >> 55) & 0x1; add_rsv(sum, impact);
			sum += (value >> 56) & 0x1; add_rsv(sum, impact);
			sum += (value >> 57) & 0x1; add_rsv(sum, impact);
			sum += (value >> 58) & 0x1; add_rsv(sum, impact);
			sum += (value >> 59) & 0x1; add_rsv(sum, impact);
			break;
		case 0x3:
			sum += value & 0x3; add_rsv(sum, impact);
			sum += (value >> 2) & 0x3; add_rsv(sum, impact);
			sum += (value >> 4) & 0x3; add_rsv(sum, impact);
			sum += (value >> 6) & 0x3; add_rsv(sum, impact);
			sum += (value >> 8) & 0x3; add_rsv(sum, impact);
			sum += (value >> 10) & 0x3; add_rsv(sum, impact);
			sum += (value >> 12) & 0x3; add_rsv(sum, impact);
			sum += (value >> 14) & 0x3; add_rsv(sum, impact);
			sum += (value >> 16) & 0x3; add_rsv(sum, impact);
			sum += (value >> 18) & 0x3; add_rsv(sum, impact);
			sum += (value >> 20) & 0x3; add_rsv(sum, impact);
			sum += (value >> 22) & 0x3; add_rsv(sum, impact);
			sum += (value >> 24) & 0x3; add_rsv(sum, impact);
			sum += (value >> 26) & 0x3; add_rsv(sum, impact);
			sum += (value >> 28) & 0x3; add_rsv(sum, impact);
			sum += (value >> 30) & 0x3; add_rsv(sum, impact);
			sum += (value >> 32) & 0x3; add_rsv(sum, impact);
			sum += (value >> 34) & 0x3; add_rsv(sum, impact);
			sum += (value >> 36) & 0x3; add_rsv(sum, impact);
			sum += (value >> 38) & 0x3; add_rsv(sum, impact);
			sum += (value >> 40) & 0x3; add_rsv(sum, impact);
			sum += (value >> 42) & 0x3; add_rsv(sum, impact);
			sum += (value >> 44) & 0x3; add_rsv(sum, impact);
			sum += (value >> 46) & 0x3; add_rsv(sum, impact);
			sum += (value >> 48) & 0x3; add_rsv(sum, impact);
			sum += (value >> 50) & 0x3; add_rsv(sum, impact);
			sum += (value >> 52) & 0x3; add_rsv(sum, impact);
			sum += (value >> 54) & 0x3; add_rsv(sum, impact);
			sum += (value >> 56) & 0x3; add_rsv(sum, impact);
			sum += (value >> 58) & 0x3; add_rsv(sum, impact);
			break;
		case 0x4:
			sum += value & 0x7; add_rsv(sum, impact);
			sum += (value >> 3) & 0x7; add_rsv(sum, impact);
			sum += (value >> 6) & 0x7; add_rsv(sum, impact);
			sum += (value >> 9) & 0x7; add_rsv(sum, impact);
			sum += (value >> 12) & 0x7; add_rsv(sum, impact);
			sum += (value >> 15) & 0x7; add_rsv(sum, impact);
			sum += (value >> 18) & 0x7; add_rsv(sum, impact);
			sum += (value >> 21) & 0x7; add_rsv(sum, impact);
			sum += (value >> 24) & 0x7; add_rsv(sum, impact);
			sum += (value >> 27) & 0x7; add_rsv(sum, impact);
			sum += (value >> 30) & 0x7; add_rsv(sum, impact);
			sum += (value >> 33) & 0x7; add_rsv(sum, impact);
			sum += (value >> 36) & 0x7; add_rsv(sum, impact);
			sum += (value >> 39) & 0x7; add_rsv(sum, impact);
			sum += (value >> 42) & 0x7; add_rsv(sum, impact);
			sum += (value >> 45) & 0x7; add_rsv(sum, impact);
			sum += (value >> 48) & 0x7; add_rsv(sum, impact);
			sum += (value >> 51) & 0x7; add_rsv(sum, impact);
			sum += (value >> 54) & 0x7; add_rsv(sum, impact);
			sum += (value >> 57) & 0x7; add_rsv(sum, impact);
			break;
		case 0x5:
			sum += value & 0xF; add_rsv(sum, impact);
			sum += (value >> 4) & 0xF; add_rsv(sum, impact);
			sum += (value >> 8) & 0xF; add_rsv(sum, impact);
			sum += (value >> 12) & 0xF; add_rsv(sum, impact);
			sum += (value >> 16) & 0xF; add_rsv(sum, impact);
			sum += (value >> 20) & 0xF; add_rsv(sum, impact);
			sum += (value >> 24) & 0xF; add_rsv(sum, impact);
			sum += (value >> 28) & 0xF; add_rsv(sum, impact);
			sum += (value >> 32) & 0xF; add_rsv(sum, impact);
			sum += (value >> 36) & 0xF; add_rsv(sum, impact);
			sum += (value >> 40) & 0xF; add_rsv(sum, impact);
			sum += (value >> 44) & 0xF; add_rsv(sum, impact);
			sum += (value >> 48) & 0xF; add_rsv(sum, impact);
			sum += (value >> 52) & 0xF; add_rsv(sum, impact);
			sum += (value >> 56) & 0xF; add_rsv(sum, impact);
			break;
		case 0x6:
			sum += value & 0x1F; add_rsv(sum, impact);
			sum += (value >> 5) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 10) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 15) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 20) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 25) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 30) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 35) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 40) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 45) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 50) & 0x1F; add_rsv(sum, impact);
			sum += (value >> 55) & 0x1F; add_rsv(sum, impact);
			break;
		case 0x7:
			sum += value & 0x3F; add_rsv(sum, impact);
			sum += (value >> 6) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 12) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 18) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 24) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 30) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 36) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 42) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 48) & 0x3F; add_rsv(sum, impact);
			sum += (value >> 54) & 0x3F; add_rsv(sum, impact);
			break;
		case 0x8:
			sum += value & 0x7F; add_rsv(sum, impact);
			sum += (value >> 7) & 0x7F; add_rsv(sum, impact);
			sum += (value >> 14) & 0x7F; add_rsv(sum, impact);
			sum += (value >> 21) & 0x7F; add_rsv(sum, impact);
			sum += (value >> 28) & 0x7F; add_rsv(sum, impact);
			sum += (value >> 35) & 0x7F; add_rsv(sum, impact);
			sum += (value >> 42) & 0x7F; add_rsv(sum, impact);
			sum += (value >> 49) & 0x7F; add_rsv(sum, impact);
			break;
		case 0x9:
			sum += value & 0xFF; add_rsv(sum, impact);
			sum += (value >> 8) & 0xFF; add_rsv(sum, impact);
			sum += (value >> 16) & 0xFF; add_rsv(sum, impact);
			sum += (value >> 24) & 0xFF; add_rsv(sum, impact);
			sum += (value >> 32) & 0xFF; add_rsv(sum, impact);
			sum += (value >> 40) & 0xFF; add_rsv(sum, impact);
			sum += (value >> 48) & 0xFF; add_rsv(sum, impact);
			break;
		case 0xA:
			sum += value & 0x3FF; add_rsv(sum, impact);
			sum += (value >> 10) & 0x3FF; add_rsv(sum, impact);
			sum += (value >> 20) & 0x3FF; add_rsv(sum, impact);
			sum += (value >> 30) & 0x3FF; add_rsv(sum, impact);
			sum += (value >> 40) & 0x3FF; add_rsv(sum, impact);
			sum += (value >> 50) & 0x3FF; add_rsv(sum, impact);
			break;
		case 0xB:
			sum += value & 0xFFF; add_rsv(sum, impact);
			sum += (value >> 12) & 0xFFF; add_rsv(sum, impact);
			sum += (value >> 24) & 0xFFF; add_rsv(sum, impact);
			sum += (value >> 36) & 0xFFF; add_rsv(sum, impact);
			sum += (value >> 48) & 0xFFF; add_rsv(sum, impact);
			break;
		case 0xC:
			sum += value & 0x7FFF; add_rsv(sum, impact);
			sum += (value >> 15) & 0x7FFF; add_rsv(sum, impact);
			sum += (value >> 30) & 0x7FFF; add_rsv(sum, impact);
			sum += (value >> 45) & 0x7FFF; add_rsv(sum, impact);
			break;
		case 0xD:
			sum += value & 0xFFFFF; add_rsv(sum, impact);
			sum += (value >> 20) & 0xFFFFF; add_rsv(sum, impact);
			sum += (value >> 40) & 0xFFFFF; add_rsv(sum, impact);
			break;
		case 0xE:
			sum += value & 0x3FFFFFFF; add_rsv(sum, impact);
			sum += (value >> 30) & 0x3FFFFFFF; add_rsv(sum, impact);
			break;
		case 0xF:
			sum += value & 0xFFFFFFFFFFFFFFFL; add_rsv(sum, impact);
			break;
		}
	}
}
