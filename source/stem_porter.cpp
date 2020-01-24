/*
	STEM_PORTER.C
	-------------
	Generate the stem of a word using Porter's algorythm
	M.F. Porter, An algoritm for suffix stripping, Program, Vol 14, No 3, pp 130-137, July 1980
*/

#include "ascii.h"
#include "stem_porter.h"

namespace JASS
	{
	/*
		ANT_STEM_PORTER::LENGTH()
		-------------------------
	*/
	size_t stem_porter::length(const char *reversed)
		{
		const char *end;
		char was;
		long size;

		if (*reversed == '\0')
			return 0;

		size = 0;
		while (isvowely(reversed))
			reversed++;
		end = reversed + strlen(reversed) - 1;
		while (end >= reversed && !isvowely(end))
			end--;
		was = 'v';
		while (end >= reversed)
			{
			if (was == 'c' && isvowely(end))
				{
				was = 'v';
				size++;
				}
			else if (was == 'v' && !isvowely(end))
				{
				was = 'c';
				size++;
				}
			end--;
			}
		return size;
		}

	/*
		ANT_STEM_PORTER::HAS_VOWEL()
		----------------------------
	*/
	bool stem_porter::has_vowel(const char *what)
		{
		const char *pos;

		if (strpbrk("aeiou", what) != NULL)
			return true;
		else
			for (pos = strchr(what, 'y'); pos != NULL; pos = strchr(pos + 1, 'y'))
				if (!ascii::isvowel(*(pos + 1)))
					return true;

		return false;
		}

	/*
		ANT_STEM_PORTER::TOSTEM()
		-------------------------
		Generate the stem of a word using Porter's algorythm
		M.F. Porter, An algoritm for suffix stripping, Program, Vol 14, No 3, pp 130-137, July 1980

		parameter "destination" must not overlap parameter "what"
	*/
	size_t stem_porter::tostem(char *destination, const char *what)
		{
		long kill_dups = 0;

		workspace.resize(strlen(what + 1));
		std::reverse_copy(what, what + strlen(what) - 1, workspace.c_str());
		char *at = const_cast<char *>(workspace.c_str());

		/*
			Step 1a.
		*/
		if (*at == 's')
			{
			if (strncmp(at, "sess", 4) == 0)
				at += 2;
			else if (strncmp(at, "sei", 3) == 0)
				at += 2;
			else if (strncmp(at, "ss", 2) == 0)
				{ /* do nothing */ }
			else
				at++;
			}

		/*
			Step 1b.
		*/
		if (*at == 'd' || *at == 'g')
			{
			if (strncmp(at, "dee", 3) == 0 && length(at + 3) > 0)
				at++;
			else if (strncmp(at, "de", 2) == 0 && has_vowel(at + 2))
				{
				kill_dups = 1;
				at += 2;
				}
			else if (strncmp(at, "gni", 3) == 0 && has_vowel(at + 3))
				{
				kill_dups = 1;
				at += 3;
				}

			if (kill_dups)
				{
				if (*at == 't' && *(at + 1) == 'a')
					*--at = 'e';
				else if (*at == 'l' && *(at + 1) == 'b')
					*--at = 'e';
				else if (*at == 'z' && *(at + 1) == 'i')
					*--at = 'e';
				else if (*at == *(at + 1))
					{
					if (strchr("aeiouylsz", *at) == 0)
					at++;
					}
				else if (length(at) == 1 && cvc(at))
					*--at = 'e';
				}
			}

		/*
			Step 1c.
		*/
		if (*at == 'y')
			if (has_vowel(at + 1))
				*at = 'i';

		/*
			Step 2.
		*/
		if (strchr("lirnms", *at) != NULL)
			{
			if (strncmp(at, "lanoita", 7) == 0 && length(at + 7) > 0)
				{
				at += 4;
				*at = 'e';
				}
			else if (strncmp(at, "lanoit", 6) == 0 && length(at + 6) > 0)
				at += 2;
			else if (strncmp(at, "icne", 4) == 0 && length(at + 4) > 0)
				*at = 'e';
			else if (strncmp(at, "icna", 4) == 0 && length(at + 4) > 0)
				*at = 'e';
			else if (strncmp(at, "rezi", 4) == 0 && length(at + 4) > 0)
				at++;
			else if (strncmp(at, "ilba", 4) == 0 && length(at + 4) > 0)
				*at = 'e';
			else if (strncmp(at, "illa", 4) == 0 && length(at + 4) > 0)
				at += 2;
			else if (strncmp(at, "iltne", 5) == 0 && length(at + 5) > 0)
				at += 2;
			else if (strncmp(at, "ile", 3) == 0 && length(at + 3) > 0)
				at += 2;
			else if (strncmp(at, "ilsuo", 5) == 0 && length(at + 5) > 0)
				at += 2;
			else if (strncmp(at, "noitazi", 7) == 0 && length(at + 7) > 0)
				{
				at += 4;
				*at = 'e';
				}
			else if (strncmp(at, "noita", 5) == 0 && length(at + 5) > 0)
				{
				at += 2;
				*at = 'e';
				}
			else if (strncmp(at, "rota", 4) == 0 && length(at + 4) > 0)
				{
				at++;
				*at = 'e';
				}
			else if (strncmp(at, "msila", 5) == 0 && length(at + 5) > 0)
				at += 3;
			else if (strncmp(at, "ssenevi", 7) == 0 && length(at + 7) > 0)
				at += 4;
			else if (strncmp(at, "ssenluf", 7) == 0 && length(at + 7) > 0)
				at += 4;
			else if (strncmp(at, "ssensuo", 7) == 0 && length(at + 7) > 0)
				at += 4;
			else if (strncmp(at, "itila", 5) == 0 && length(at + 5) > 0)
				at += 3;
			else if (strncmp(at, "itivi", 5) == 0 && length(at + 5) > 0)
				{
				at += 2;
				*at = 'e';
				}
			else if (strncmp(at, "itilib", 6) == 0 && length(at + 6) > 0)
				{
				at += 4;
				*at-- = 'l';
				*at = 'e';
				}
			}

		/*
			Step 3.
		*/
		if (strchr("eils", *at) != NULL)
			{
			if (strncmp(at, "etaci", 5) == 0  && length(at + 5) > 0)
				at += 3;
			else if (strncmp(at, "evita", 5) == 0 && length(at + 5) > 0)
				at += 5;
			else if (strncmp(at, "ezila", 5) == 0 && length(at + 5) > 0)
				at += 3;
			else if (strncmp(at, "itici", 5) == 0 && length(at + 5) > 0)
				at += 3;
			else if (strncmp(at, "laci", 4) == 0 && length(at + 4) > 0)
				at += 2;
			else if (strncmp(at, "luf", 3) == 0 && length(at + 3) > 0)
				at += 3;
			else if (strncmp(at, "ssen", 4) == 0 && length(at + 4) > 0)
				at += 4;
			}

		/*
			Step 4.
		*/
		if (strchr("lerctumisn", *at) != NULL)
			{
			if (strncmp(at, "la", 2) == 0 && length(at + 2) > 1)
				at += 2;
			else if (strncmp(at, "ecna", 4) == 0 && length(at + 4) > 1)
				at += 4;
			else if (strncmp(at, "ecne", 4) == 0 && length(at + 4) > 1)
				at += 4;
			else if (strncmp(at, "re", 2) == 0 && length(at + 2) > 1)
				at += 2;
			else if (strncmp(at, "ci", 2) == 0 && length(at + 2) > 1)
				at += 2;
			else if (strncmp(at, "elba", 4) == 0 && length(at + 4) > 1)
				at += 4;
			else if (strncmp(at, "elbi", 4) == 0 && length(at + 4) > 1)
				at += 4;
			else if (strncmp(at, "tna", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "tneme", 5) == 0 && length(at + 5) > 1)
				at += 5;
			else if (strncmp(at, "tnem", 4) == 0 && length(at + 4) > 1)
				at += 4;
			else if (strncmp(at, "tne", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "uo", 2) == 0 && length(at + 2) > 1)
				at += 2;
			else if (strncmp(at, "msi", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "eta", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "iti", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "suo", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "evi", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "ezi", 3) == 0 && length(at + 3) > 1)
				at += 3;
			else if (strncmp(at, "nois", 4) == 0 && length(at + 3) > 1) // special case
				at += 3;
			else if (strncmp(at, "noit", 4) == 0 && length(at + 3) > 1) // special case
				at += 3;
			}

		/*
			Step 5a.
		*/
		if (*at == 'e')
			{
			if (length(at + 1) == 1 && !cvc(at + 1))
				at++;
			else if (length(at + 1) > 1)
				at++;
			}
		/*
			Step 5b.
		*/
		if (*at == 'l' && *(at + 1) == 'l' && length(at + 1) > 1)
			at++;

		strrev(destination, at);
		/*
			The Porter algorithms is only guaranteed not to change the first character of the word, but
			Porter's web site says that you can trim this to two because that will catch all cases except
			pathalogically stupid cases such as 'is' and 'as'.  As the length return value is only used
			by the search engine (because the stem is '\0' terminated), it is OK to return 2 here in order
			to make the search engine start at the first two characters, but it is not OK to use it for
			anything else.
		*/
		return 2;
		}
	}
