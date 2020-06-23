/*
	COMPRESS_INTEGER_VARIABLE_BYTE.CPP
	----------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdio.h>

#include <random>

#include "assert.h"
#include "compress_integer_variable_byte.h"


	/*
		COMPRESS_INTEGER_VARIABLE_BYTE::ENCODE()
		----------------------------------------
	*/
	size_t compress_integer_variable_byte::encode(void *encoded_as_void, size_t encoded_buffer_length, const integer *source, size_t source_integers)
		{
		uint8_t *encoded = static_cast<uint8_t *>(encoded_as_void);
		size_t used = 0;						// the number of bytes of storage used so far

		const integer *end = source + source_integers;			// the end of the input sequence
		
		/*
			Iterate over each integer in the input sequence
		*/
		for (const integer *current = source; current < end; current++)
			{
			/*
				find out how much space it'll take
			*/
			size_t needed = bytes_needed_for(*current);
			
			/*
				make sure it'll fit in the output buffer
			*/
			if (used + needed > encoded_buffer_length)
				return 0;				// didn't fit so return failure state.
			
			/*
				it fits so encode and add to the size used
			*/
			compress_into(encoded, *current);
			used += needed;
			}

		return used;
		}

