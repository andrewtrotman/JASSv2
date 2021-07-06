/*
	STEM_PORTER.CPP
	---------------
*/
#include <vector>
#include <iostream>
#include <algorithm>

#include "asserts.h"
#include "ascii.h"
#include "stem_porter.h"

namespace JASS
	{
	/*
		STEM_PORTER::LENGTH()
		---------------------
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
		STEM_PORTER::HAS_VOWEL()
		------------------------
	*/
	bool stem_porter::has_vowel(const char *what)
		{
		if (strpbrk("aeiou", what) != NULL)
			return true;
		else
			for (const char *pos = strchr(what, 'y'); pos != NULL; pos = strchr(pos + 1, 'y'))
				if (!ascii::isvowel(*(pos + 1)))
					return true;

		return false;
		}

	/*
		STEM_PORTER::TOSTEM()
		---------------------
	*/
	size_t stem_porter::tostem(char *destination, const char *source, size_t source_length)
		{
		/*
			Reverse the string.
		*/
		workspace.resize(source_length + 1);
		char *at = const_cast<char *>(workspace.c_str());
		std::reverse_copy(source, source + source_length, at);
//		workspace[source_length] = '\0';
		at[source_length] = '\0';

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
			long kill_dups = 0;

			if (strncmp(at, "dee", 3) == 0)
				{
				if (length(at + 3) > 0)
					at++;
				}
			else if (strncmp(at, "de", 2) == 0)
				{
				if (has_vowel(at + 2))
					{
					kill_dups = 1;
					at += 2;
					}
				}
			else if (strncmp(at, "gni", 3) == 0)
				{
				if (has_vowel(at + 3))
					{
					kill_dups = 1;
					at += 3;
					}
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
			else if (strncmp(at, "noitazi", 7) == 0)
				{
				if (length(at + 7) > 0)
					{
					at += 4;
					*at = 'e';
					}
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
			else if (strncmp(at, "tneme", 5) == 0)
				{
				if (length(at + 5) > 1)
					at += 5;
				}
			else if (strncmp(at, "tnem", 4) == 0)
				{
				if (length(at + 4) > 1)
					at += 4;
				}
			else if (strncmp(at, "tne", 3) == 0)
				{
				if (length(at + 3) > 1)
					at += 3;
				}
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
			if (length(at + 1) > 1)
				at++;
			else if (length(at + 1) == 1 && !cvc(at + 1))
				at++;
			}

		/*
			Step 5b.
		*/
		if (*at == 'l' && *(at + 1) == 'l' && length(at + 1) > 1)
			at++;

		size_t stem_length = strlen(at);
		std::reverse_copy(at, at + stem_length, destination);
		destination[stem_length] = '\0';

		return stem_length;
		}

	/*
		STEM_PORTER::UNITTEST()
		-----------------------
	*/
	void stem_porter::unittest(void)
		{
		std::vector<std::pair<std::string, std::string>> test_data =
			{
			{"caresses", "caress"},
			{"ponies", "poni"},
			{"ties", "ti"},
			{"caress", "caress"},
			{"cats", "cat"},
			{"feed", "feed"},
			{"agreed", "agre"},
			{"plastered", "plaster"},
			{"bled", "bled"},
			{"motoring", "motor"},
			{"sing", "sing"},
			{"conflated", "conflat"},
			{"troubled", "troubl"},
			{"sized", "size"},
			{"hopping", "hop"},
			{"tanned", "tan"},
			{"falling", "fall"},
			{"hissing", "hiss"},
			{"fizzed", "fizz"},
			{"failing", "fail"},
			{"filing", "file"},
			{"happy", "happi"},
			{"sky", "sky"},
			{"relational", "relat"},
			{"conditional", "condit"},
			{"rational", "ration"},
			{"valenci", "valenc"},
			{"hesitanci", "hesit"},
			{"digitizer", "digit"},
			{"conformabli", "conform"},
			{"radicalli", "radic"},
			{"differentli", "differ"},
			{"vileli", "vile"},
			{"analogousli", "analog"},
			{"vietnamization", "vietnam"},
			{"predication", "predic"},
			{"operator", "oper"},
			{"feudalism", "feudal"},
			{"decisiveness", "decis"},
			{"hopefulness", "hope"},
			{"callousness", "callous"},
			{"formaliti", "formal"},
			{"sensitiviti", "sensit"},
			{"sensibiliti", "sensibl"},
			{"triplicate", "triplic"},
			{"formative", "form"},
			{"formalize", "formal"},
			{"electriciti", "electr"},
			{"electrical", "electr"},
			{"hopeful", "hope"},
			{"goodness", "good"},
			{"revival", "reviv"},
			{"allowance", "allow"},
			{"inference", "infer"},
			{"airliner", "airlin"},
			{"gyroscopic", "gyroscop"},
			{"adjustable", "adjust"},
			{"defensible", "defens"},
			{"irritant", "irrit"},
			{"replacement", "replac"},
			{"adjustment", "adjust"},
			{"dependent", "depend"},
			{"adoption", "adopt"},
			{"homologou", "homolog"},
			{"communism", "commun"},
			{"activate", "activ"},
			{"angulariti", "angular"},
			{"homologous", "homolog"},
			{"effective", "effect"},
			{"bowdlerize", "bowdler"},
			{"probate", "probat"},
			{"rate", "rate"},
			{"cease", "ceas"},
			{"controll", "control"},
			{"roll", "roll"}
		};

		stem_porter stemmer;
		char result[1024];

		for (const auto &example : test_data)
			{
			stemmer.tostem(result, example.first.c_str(), example.first.size());
			JASS_assert(result == example.second);
			}

		puts("stem_porter::PASSED");
		}
	}
