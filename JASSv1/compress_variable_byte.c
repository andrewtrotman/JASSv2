/*
	COMPRESS_VARIABLE_BYTE.C
	------------------------
*/
#include "compress_variable_byte.h"

/*
	ANT_COMPRESS_VARIABLE_BYTE::COMPRESS()
	--------------------------------------
*/
long long ANT_compress_variable_byte::compress(unsigned char *destination, long long destination_length, uint32_t *source, long long source_integers)
{
uint32_t *current, *end;
long needed;
long long used = 0;

end = source + source_integers;
for (current = source; current < end; current++)
	{
	needed = compress_bytes_needed(*current);
	if (used + needed > destination_length)
		return 0;
	compress_into(destination + used, *current);
	used += needed;
	}

return used;
}

/*
	ANT_COMPRESS_VARIABLE_BYTE::DECOMPRESS()
	----------------------------------------
*/
void ANT_compress_variable_byte::decompress(uint32_t *destination, unsigned char *source, long long destination_integers)
{
uint32_t *end;

end = destination + destination_integers;

while (destination < end)
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
}
