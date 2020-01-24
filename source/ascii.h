/*
	ASCII.H
	-------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief fast locale-ignoring version of the C runtime library ctype methods for plain 7-bit ASCII.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

#include "asserts.h"

namespace JASS
	{
	/*
		CLASS ASCII
		-----------
	*/
	/*!
		@class ascii
		@brief A fast, table-driven, implementation of the C runtime library ctype routines.
		@details This implementation assumes the plain 7-bit ASCII  as it is expected to be used
			alongside the Unicode equivelant.  These methods can be called with 8-bit values).  Because
			7-bit ASCII is used its not necessary to manage locals - which also makes it faster.
	*/
	class ascii
		{
		protected:
			/*!
				@brief The enum of characteristics of an ascii character.
			*/
			enum
				{
				UPPER = 1,											///< this character is an upper case character.
				LOWER = 2,											///< this character is a lower case character.
				DIGIT = 4,											///< this character is a digit.
				CONTROL = 8,										///< this character is a control character.
				PUNC = 16,											///< this characgter is punctuation.
				SPACE = 32,											///< this character is whitespace.
				HEX = 64,											///< this character is a hexadecimal digit.
				DNA = 128,											///< this character is a DNA base (i.e in: {ATCGatcg})
				VOWEL = 256,										///< this character is a vowel (i.e in: {aeiouAEIOU})
				VOWELY = 512,										///< this character is a vowel or Y (i.e in: {aeiouyAEIOUY})

				ALPHA = UPPER | LOWER,							///< this character is alphabetic
				ALNUM = ALPHA | DIGIT,							///< this character is alphanumeric
				GRAPH  = PUNC | UPPER | LOWER | DIGIT,		///< this character is a graphic character
				};

		protected:
			/*
				These two are defined in the CPP file.
			*/
			static const uint16_t ctype[];			///< table of characteristics about a character. ctype['x'] is "all about x".
			static const uint8_t upper_list[];		///< table of uppercase values.  upper_list['x'] is the uppercase of x (i.e. 'X')
			static const uint8_t lower_list[];		///< table of lowercase values.  lower_list['X'] is the lowercase of x (i.e. 'x')

		public:
			/*
				ASCII::ISASCII()
				----------------
			*/
			/*!
				@brief Is the given character a plain 7-bit ASCII character
				@param c [in] The character to check.
				@return true is the high bit is 0, false if the high bit is 1.
			*/
			#undef isascii
			inline static int isascii(uint8_t c)
				{
				return !(c & 0x80);
				}
			
			/*
				ASCII::ISALNUM()
				----------------
			*/
			/*!
				@brief Is this character alphanumeric?
				@param c [in] The character to check
				@return true if uppercase, lowercase, or a digit, else false.
			*/
			inline static int isalnum(uint8_t c)
				{
				return (ctype[c] & ALNUM);
				}

			/*
				ASCII::ISALPHA()
				----------------
			*/
			/*!
				@brief Is this character an uppercase or lowercase alphabetic?
				@param c [in] The character to check
				@return true if uppercase or lowercase, else false.
			*/
			inline static int isalpha(uint8_t c)
				{
				return (ctype[c] & ALPHA);
				}
				
			/*
				ASCII::ISCNTRL()
				----------------
			*/
			/*!
				@brief Is this character a control character (ASCII code 0x00-0x1F or 0x7F)?
				@param c [in] The character to check
				@return true if a control character, else false.
			*/
			inline static int iscntrl(uint8_t c)
				{
				return (ctype[c] & CONTROL);
				}
			
			
			/*
				ASCII::ISDIGIT()
				----------------
			*/
			/*!
				@brief Is this character a digit?
				@param c [in] The character to check
				@return true if a digit, else false.
			*/
			inline static int isdigit(uint8_t c)
				{
				return (ctype[c] & DIGIT);
				}
			
			/*
				ASCII::ISGRAPH()
				----------------
			*/
			/*!
				@brief Is this character a graphical character (defined as printable but not space).
				@param c [in] The character to check
				@return true if a graphic character, else false.
			*/
			inline static int isgraph(uint8_t c)
				{
				return (ctype[c] & GRAPH);
				}
			
			/*
				ASCII::ISLOWER()
				----------------
			*/
			/*!
				@brief Is this character a lowercase character.
				@param c [in] The character to check
				@return true if a lowercase character, else false.
			*/
			inline static int islower(uint8_t c)
				{
				return (ctype[c] & LOWER);
				}
				
			/*
				ASCII::ISPRINT()
				----------------
			*/
			/*!
				@brief Is this character a printable character (all characters greater than 0x1F, except 0x7F).
				@param c [in] The character to check
				@return true if a printable character, else false.
			*/
			inline static int isprint(uint8_t c)
				{
				return !iscntrl(c);
				}

			/*
				ASCII::ISPUNCT()
				----------------
			*/
			/*!
				@brief Is this character a punctuation character (a non-alphanumeric character that is printable).
				@param c [in] The character to check
				@return true if a punctuation character, else false.
			*/
			inline static int ispunct(uint8_t c)
				{
				return (ctype[c] & PUNC);
				}
			
			/*
				ASCII::ISSPACE()
				----------------
			*/
			/*!
				@brief Is this character a space character (space, tab, vertical tab, carrage return, newline, form feed)
				@param c [in] The character to check
				@return true if a space character, else false.
			*/
			inline static int isspace(uint8_t c)
				{
				return (ctype[c] & SPACE);
				}
				
			/*
				ASCII::ISUPPER()
				----------------
			*/
			/*!
				@brief Is this character a uppercase character.
				@param c [in] The character to check
				@return true if an uppercase character, else false.
			*/
			inline static int isupper(uint8_t c)
				{
				return (ctype[c] & UPPER);
				}
			
			/*
				ASCII::ISXDIGIT()
				----------------
			*/
			/*!
				@brief Is this character a hexadecimal digit.
				@param c [in] The character to check
				@return true if a hexadecimal, else false.
			*/
			inline static int isxdigit(uint8_t c)
				{
				return (ctype[c] & HEX);
				}

			/*
				ASCII::ISDNABASE()
				------------------
			*/
			/*!
				@brief Is this character a DNA base (i.e. in {a,t,c,g,A,T,C,G})
				@param c [in] The character to check
				@return true if a DNA base, else false.
			*/
			inline static int isDNAbase(uint8_t c)
				{
				return (ctype[c] & DNA);
				}

			/*
				ASCII::ISVOWEL()
				----------------
			*/
			/*!
				@brief Is this character a vowel (i.e. in {aeiouAEIOU})
				@param c [in] The character to check
				@return true if a vowel base, else false.
			*/
			inline static int isvowel(uint8_t c)
				{
				return (ctype[c] & VOWEL);
				}

			/*
				ASCII::ISVOWELY()
				----------------
			*/
			/*!
				@brief Is this character a vowel or Y (i.e. in {aeiouyAEIOUY})
				@param c [in] The character to check
				@return true if a vowel base, else false.
			*/
			inline static int isvowely(uint8_t c)
				{
				return (ctype[c] & VOWELY);
				}

			/*
				ASCII::TOLOWER()
				----------------
			*/
			/*!
				@brief Convert the uppercase character to lowercase (and leave all other characters are they are).
				@param c [in] The character to convert.
				@return if c is uppercase then the lowercase of c, else c.
			*/
			inline static uint8_t tolower(uint8_t c)
				{
				return lower_list[c];
				}
				
			/*
				ASCII::TOUPPER()
				----------------
			*/
			/*!
				@brief Convert the lowercase character to uppercase (and leave all other characters are they are).
				@param c [in] The character to convert.
				@return if c is lowercae then the uppercase of c, else c.
			*/
			inline static uint8_t toupper(uint8_t c)
				{
				return upper_list[c];
				}
				
			/*
				ASCII::UNITTEST()
				-----------------
			*/
			/*!
				@brief Unit test this class
				@details This simply compares this class's method with the C runtime library versions.  The test is exhaustive
			*/
			static void unittest(void)
				{
				for (uint8_t character = 0; character <= 0x7F; character++)
					{
					JASS_assert((::isalpha(character) && isalpha(character)) || ((!::isalpha(character) && !isalpha(character))));
					JASS_assert((::isalnum(character) && isalnum(character)) || ((!::isalnum(character) && !isalnum(character))));
					JASS_assert((::isupper(character) && isupper(character)) || ((!::isupper(character) && !isupper(character))));
					JASS_assert((::islower(character) && islower(character)) || ((!::islower(character) && !islower(character))));
					JASS_assert((::isdigit(character) && isdigit(character)) || ((!::isdigit(character) && !isdigit(character))));
					JASS_assert((::isspace(character) && isspace(character)) || ((!::isspace(character) && !isspace(character))));
					JASS_assert((::iscntrl(character) && iscntrl(character)) || ((!::iscntrl(character) && !iscntrl(character))));
					JASS_assert((::isgraph(character) && isgraph(character)) || ((!::isgraph(character) && !isgraph(character))));
					if (character != 9)
						{
						/*
							this is due to a windows bug, see: https://developercommunity.visualstudio.com/content/problem/297085/changing-result-of-isspacet.html
						*/
						JASS_assert((::isprint(character) && isprint(character)) || ((!::isprint(character) && !isprint(character))));
						}
					JASS_assert((::ispunct(character) && ispunct(character)) || ((!::ispunct(character) && !ispunct(character))));
					JASS_assert((::isxdigit(character) && isxdigit(character)) || ((!::isxdigit(character) && !isxdigit(character))));

					JASS_assert(::tolower(character) == tolower(character));
					JASS_assert(::toupper(character) == toupper(character));
					}
				puts("ascii::PASSED");
				}
	};
}
