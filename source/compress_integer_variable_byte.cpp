/*
	COMPRESS_INTEGER_VARIABLE_BYTE.CPP
	----------------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>
#include <stdio.h>

#include <random>

#include "asserts.h"
#include "compress_integer_variable_byte.h"

namespace JASS
	{
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

	/*
		COMPRESS_INTEGER_VARIABLE_BYTE::UNITTEST()
		------------------------------------------
	*/
	void compress_integer_variable_byte::unittest(void)
		{
		compress_integer_variable_byte *codex = new compress_integer_variable_byte;								// so that encode() and decode() can be called
		size_t bytes_used;														// the number of bytes used to encode the integer sequence
		uint8_t encoded_buffer[2048];											// sequences are encoded into this buffer
		integer decoded_buffer[2048];											// sequences are decoded into this buffer

		/*
			Check what happens if it won't fit
		*/
		const integer too_big[] = {1 << 21,  (1 << 28) - 1};		// the bounds on 4-byte encodings
		bytes_used = codex->encode(encoded_buffer, 1, too_big, sizeof(too_big) / sizeof(*too_big));
		JASS_assert(bytes_used == 0);
		
		/*
			Check the upper and lower bounds of 1-byte encodings
		*/
		const integer one_byte[] = {0, (1 << 7) - 1};					// the bounds on 1-byte encodings
		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), one_byte, sizeof(one_byte) / sizeof(*one_byte));
		codex->decode(decoded_buffer, sizeof(one_byte) / sizeof(*one_byte), encoded_buffer, bytes_used);
		JASS_assert(bytes_used == 2);
		JASS_assert(memcmp(decoded_buffer, one_byte, sizeof(one_byte)) == 0);
		
		/*
			Check the upper and lower bounds of 2-byte encodings
		*/
		const integer two_byte[] = {1 << 7,  (1 << 14) - 1};			// the bounds on 2-byte encodings
		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), two_byte, sizeof(two_byte) / sizeof(*two_byte));
		codex->decode(decoded_buffer, sizeof(two_byte) / sizeof(*two_byte), encoded_buffer, bytes_used);
		JASS_assert(bytes_used == 4);
		JASS_assert(memcmp(decoded_buffer, two_byte, sizeof(two_byte)) == 0);
		
		/*
			Check the upper and lower bounds of 3-byte encodings
		*/
		const integer three_byte[] = {1 << 14,  (1 << 21) - 1};		// the bounds on 3-byte encodings
		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), three_byte, sizeof(three_byte) / sizeof(*three_byte));
		codex->decode(decoded_buffer, sizeof(three_byte) / sizeof(*three_byte), encoded_buffer, bytes_used);
		JASS_assert(bytes_used == 6);
		JASS_assert(memcmp(decoded_buffer, three_byte, sizeof(three_byte)) == 0);

		/*
			Check the upper and lower bounds of 4-byte encodings
		*/
		const integer four_byte[] = {1 << 21,  (1 << 28) - 1};		// the bounds on 4-byte encodings
		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), four_byte, sizeof(four_byte) / sizeof(*four_byte));
		codex->decode(decoded_buffer, sizeof(four_byte) / sizeof(*four_byte), encoded_buffer, bytes_used);
		JASS_assert(bytes_used == 8);
		JASS_assert(memcmp(decoded_buffer, four_byte, sizeof(four_byte)) == 0);
		
		/*
			Check the upper and lower bounds of 5-byte encodings
		*/
		const integer five_byte[] = {1 << 28,  0xFFFFFFFF};			// the bounds on 5-byte encodings
		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), five_byte, sizeof(five_byte) / sizeof(*five_byte));
		codex->decode(decoded_buffer, sizeof(five_byte) / sizeof(*five_byte), encoded_buffer, bytes_used);
		JASS_assert(bytes_used == 10);
		JASS_assert(memcmp(decoded_buffer, five_byte, sizeof(five_byte)) == 0);

		/*
			Check the top range of the 64-bit encodings
		*/
		uint64_t ten_byte[] = {(uint64_t)1 << (uint64_t)63,  0xFFFFFFFFFFFFFFFF};
		uint64_t answer_64 = 0;
		uint8_t *into_64 = encoded_buffer;
		compress_integer_variable_byte::compress_into(into_64, ten_byte[0]);
		into_64 = encoded_buffer;
		compress_integer_variable_byte::decompress_into(&answer_64, into_64);
		JASS_assert(answer_64 == ten_byte[0]);
		answer_64 = 0;
		into_64 = encoded_buffer;
		compress_integer_variable_byte::compress_into(into_64, ten_byte[1]);
		into_64 = encoded_buffer;
		compress_integer_variable_byte::decompress_into(&answer_64, into_64);
		JASS_assert(answer_64 == ten_byte[1]);

		/*
			Generate a sequence of random integers and check they encode and decode correctly.  Yes, this is favouring large integers
			because the probability of the high bit being set is 50%.
		*/
		std::random_device device;
		std::mt19937 generator(device());
		std::uniform_int_distribution<integer> distribution;
		integer raw_buffer[128];
		
		for (size_t count = 0; count < sizeof(raw_buffer) / sizeof(*raw_buffer); count++)
			raw_buffer[count] = distribution(generator);

		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), raw_buffer, sizeof(raw_buffer) / sizeof(*raw_buffer));
		codex->decode(decoded_buffer, sizeof(raw_buffer) / sizeof(*raw_buffer), encoded_buffer, bytes_used);
		JASS_assert(memcmp(decoded_buffer, raw_buffer, sizeof(raw_buffer)) == 0);
		
		/*
			Now check that the example in the documentation is correct, and that the encoding is big-endian
		*/
		auto pointer = &encoded_buffer[0];
		compress_into(pointer, (uint32_t)1905);
		const uint8_t answer[] = {0x0E, 0xF1};
		JASS_assert(memcmp(answer, encoded_buffer, 2) == 0);
		
#if JASS_COMPRESS_INTEGER_BITS_PER_INTEGER == 64
		/*
			If we're encoding 64-bit integers then check the highest boundaries (i.e. 10 byte encodings).  Note that
			to get to this point it is highly probably that a load of 64-bit integers with the high bit set have
			already been tested in the random test.  The same is true of 64-bit integers without the high bit set
			(i.e. 63-bit integers).
		*/
		const integer ten_byte[] = {(uint64_t)1 << 63,  0xFFFFFFFFFFFFFFFF};
		memset(encoded_buffer, 0, sizeof(encoded_buffer));
		memset(decoded_buffer, 0, sizeof(decoded_buffer));
		bytes_used = codex->encode(encoded_buffer, sizeof(encoded_buffer), ten_byte, sizeof(ten_byte) / sizeof(*ten_byte));
		codex->decode(decoded_buffer, sizeof(ten_byte) / sizeof(*five_byte), encoded_buffer, bytes_used);
		JASS_assert(bytes_used == 20);
		JASS_assert(memcmp(decoded_buffer, ten_byte, sizeof(five_byte)) == 0);
#endif
		
		/*
			The tests have passed
		*/
		delete codex;
		puts("compress_integer_variable_byte::PASSED");
		}
	}
