/*
	PARSER.CPP
	----------
*/
#include <string.h>

#include "ascii.h"
#include "parser.h"
#include "unicode.h"

namespace JASS
	{
	/*
		PARSER::GET_NEXT_TOKEN()
		------------------------
	*/
	class parser::token &parser::get_next_token(void)
		{
		size_t bytes;

		/*
			We skip over non-indexable characters before the start of the first token
			Start with the assumption that we're skipping over ASCII white space
		*/

		while (ascii::isspace(*current))
			current++;

		/*
			If we've hit a non-white space then either its an ASCII non-white space,
			else we have to skip over Unicode white space
		*/
		if (ascii::isascii(*current))
			bytes = 1;
		else
			{
			/*
				Skipping over Unicode whitespace
			*/
			uint32_t codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
			while (unicode::isalnum(codepoint))
				{
				current += bytes;
				codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
				}
			}

		/*
			Process based on the character type (ASCII or Unicode)
		*/
		if (bytes == 1)
			{
			/*
				Alphabetics
			*/
			if (ascii::isalpha(*current))
				{
				/*
					Normalise (casefold etc.) the token into the buffer in the token object
				*/
				uint8_t *buffer_pos = token.buffer;
				uint8_t *buffer_end = token.buffer + sizeof(token.buffer);

				/*
					As the first character is ASCII then assume the remainder will be.
				*/
				*buffer_pos++ = tolower(*current);
				current++;
			
				while (ascii::isalpha(*current))
					{
					if (buffer_pos < buffer_end)
						*buffer_pos++ = ascii::tolower(*current);				// won't write on overflow
					current++;
					}
					
				/*
					We might have terminated when we found a non-ASCII so keep processing from here on
					using Unicode (i.e. the slow way).  If we find more ASCII then it will keep going
					fine as ASCII is a subset of Unicode.
				*/
				if (!ascii::isascii(*current))
					{
					std::vector<uint32_t>normalized(15);
					uint32_t codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
					if (unicode::isalpha(codepoint))
						{
						for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
							buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow

						current += bytes;
						codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
						}
					}
				token.token = slice((void *)token.buffer, (void *)buffer_pos);
				token.type = token.alpha;
				}
			}
		else
			assert(/*"this is the unwritten Unicode path"*/ 0);

		return token;
		}
	}
