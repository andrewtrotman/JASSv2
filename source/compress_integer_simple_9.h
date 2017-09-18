/*
	COMPRESS_SIMPLE9.H
	------------------
	Copyright (c) 2014-2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Anh and Moffat's Simple-9 Compression scheme from:
	V. Anh, A. Moffat (2005), Inverted Index Compression Using Word-Alligned Binary Codes, Information Retrieval, 8(1):151-166

	This code was originally written by Vikram Subramanya while working on:
	A. Trotman, V. Subramanya (2007), Sigma encoded inverted files, Proceedings of CIKM 2007, pp 983-986

	Substantially re-written and converted for use in ATIRE by Andrew Trotman (2009)
	but then adapted for Simple-8b and back ported by Blake Burgess.  This version has been
	refactored for JASS by Andrew Trotman.
*/
#pragma once
#include "compress_integer.h"

namespace JASS
	{
	/*
		CLASS COMPRESS_INTEGER_SIMPLE_9
		-------------------------------
	*/
	class compress_integer_simple_9 : public compress_integer
		{
		protected:
			/*
				CLASS LOOKUP
				------------
				internal class for compatibility with derived schemes
			*/
			class lookup
				{
				public:
					size_t numbers;
					size_t bits;
					size_t mask;
				} ;
		protected:
			/*
				internal tables for comptability with derived schemes
			*/
			static lookup simple9_table[];
			static size_t bits_to_use[];
			static size_t table_row[];

			static size_t ints_packed_table[];
			static size_t can_pack_table[];
			static size_t row_for_bits_needed[];
			static size_t invalid_masks_for_offset[];
			static size_t simple9_shift_table[];

		public:
			compress_integer_simple_9()
				{
				/* Nothing */
				}
			virtual ~compress_integer_simple_9()
				{
				/* Nothing */
				}

			virtual size_t compress(uint8_t *destination, size_t destination_length, integer *source, size_t source_integers);
			
			virtual void decompress(integer *destination, uint8_t *source, size_t destination_integers);
		};
	}
