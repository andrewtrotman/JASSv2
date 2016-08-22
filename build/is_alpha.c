#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ers.cpp"

uint32_t unicode_isalpha(uint32_t codepoint)
{
size_t byte = codepoint >> 3;
size_t bit = 1 << (codepoint & 0x07);

return unicode_isalpha_data[byte] & bit;
}
