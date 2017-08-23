/*
	COMPRESS_INTEGER_QMX_ORIGINAL.H
	-------------------------------
	Copyright (c) 2014-2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

#include <stdint.h>
#include "compress_integer.h"

namespace JASS
	{
	/*
		class COMPRESS_INTEGER_QMX_ORIGINAL
		-----------------------------------
	*/
	class compress_integer_qmx_original : public compress_integer
	{
	private:
		uint8_t *length_buffer;
		uint64_t length_buffer_length;

	public:
		compress_integer_qmx_original();
		virtual ~compress_integer_qmx_original();

		void encodeArray(const uint32_t *in, uint64_t len, uint32_t *out, uint64_t *nvalue);
		void decodeArray(const uint32_t *in, uint64_t len, uint32_t *out, uint64_t nvalue);

		virtual uint64_t compress(uint8_t *destination, uint64_t destination_length, uint32_t *source, uint64_t source_integers)
			{
			uint64_t answer;
			encodeArray(source, source_integers, (uint32_t *)destination, &answer);
			return answer;
			}

		virtual void decompress(uint32_t *destination, uint64_t destinaton_integers, uint8_t *source, uint64_t source_length)
			{
			decodeArray((uint32_t *)source, source_length, destination, destinaton_integers);
			}
	} ;
}
