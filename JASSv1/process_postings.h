#ifndef PROCESS_POSTINGS_H_
#define PROCESS_POSTINGS_H_

#include <stdint.h>
#include "CI.h"
#include "compress_qmx.h"
#include "compress_qmx_d4.h"
#include "compress_simple8b.h"
#include "compress_integer_elias_gamma_simd.h"
#include "compress_integer_elias_delta_simd.h"

void CIt_process_list_compressed_elias_gamma_simd(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_elias_delta_simd(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_vbyte(uint8_t *doclist, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_decompress_then_process(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_not_compressed(uint8_t *doclist, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_qmx(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_qmx_d4(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_qmx_d0(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_simple8b_ATIRE(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);
void CIt_process_list_compressed_simple8b(uint8_t *source, uint8_t *end, uint16_t impact, uint32_t integers);

#define ALIGN_16 __attribute__ ((aligned (16)))
extern ALIGN_16 uint32_t *CI_decompressed_postings;
extern ALIGN_16 uint8_t *postings;					// the postings themselves

#endif

