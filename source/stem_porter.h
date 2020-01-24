/*
	STEM_PORTER.H
	-------------
*/
#pragma once

#include <stdio.h>
#include "stem.h"

#define MAX_TERM_LENGTH 1024

namespace JASS
	{
	/*
		class STEM_PORTER
		-----------------
	*/
	class stem_porter : public stem
		{
		private:
			std::string workspace;
			
		private:
			/*
				Is the character a vowel or a y not folowed by a vowel
			*/
			bool isvowely(const char *c)
				{
				return *c != '\0' && (ascii::isvowel(*c) || (*c == 'y' && !ascii::isvowel(*(c + 1))));
				}

			/*
				Is the chacter a cvc sequence?
			*/
			bool cvc(const char *what)
				{
				return ((strchr("aeiouwxy", *what) == NULL) && (isvowely(what + 1)) && (!isvowely(what + 2))) ? true : false;
				}

			/*
				return M, the porter length value
			*/
			size_t length(const char *reversed);

			bool has_vowel(const char *what);

		public:
			virtual ~stem_porter()
				{
				/* Nothing */
				}

			virtual size_t tostem(char *destination, const char *term);

			virtual std::string name(void)
				{
				return "Porter";
				}
		} ;

	}
