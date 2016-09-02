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
#include <assert.h>

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
		private:
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

				ALPHA = UPPER | LOWER,							///< this character is alphabetic
				ALNUM = ALPHA | DIGIT,							///< this character is alphanumeric
				GRAPH  = PUNC | UPPER | LOWER | DIGIT,		///< this character is a graphic character
				};

		private:
			/*
				These two are defined in the CPP file.
			*/
			static const uint16_t ctype[];			///< table of characteristics about a character. ctype['x'] is "all about x".
			static const uint8_t upper_list[];		///< table of uppercase values.  upper_list['x'] is the uppercase of x (i.e. 'X')
			static const uint8_t lower_list[];		///< table of lowercase values.  lower_list['X'] is the lowercase of x (i.e. 'x')

		public:
			/*
				ASCII::ISALNUM()
				----------------
			*/
			/*!
				@brief Is this character alphanumeric?
				@param c [in] The character to check
				@return true if uppercase, lowercase, or a digit, else false.
			*/
			inline static bool isalnum(uint8_t c)
				{
				return (ctype[c] & ALNUM) != 0;
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
			inline static bool isalpha(uint8_t c)
				{
				return (ctype[c] & ALPHA) != 0;
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
			inline static bool iscntrl(uint8_t c)
				{
				return (ctype[c] & CONTROL) != 0;
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
			inline static bool isdigit(uint8_t c)
				{
				return (ctype[c] & DIGIT) != 0;
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
			inline static bool isgraph(uint8_t c)
				{
				return (ctype[c] & GRAPH) != 0;
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
			inline static bool islower(uint8_t c)
				{
				return (ctype[c] & LOWER) != 0;
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
			inline static bool isprint(uint8_t c)
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
			inline static bool ispunct(uint8_t c)
				{
				return (ctype[c] & PUNC) != 0;
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
			inline static bool isspace(uint8_t c)
				{
				return (ctype[c] & SPACE) != 0;
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
			inline static bool isupper(uint8_t c)
				{
				return (ctype[c] & UPPER) != 0;
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
			inline static bool isxdigit(uint8_t c)
				{
				return (ctype[c] & HEX) != 0;
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
				@brief Static method that performs a unittest.
				@details This simply compares this class's method with the C runtime library versions.  The test is exhaustive
			*/
			static void unittest(void)
				{
				for (uint8_t character = 0; character <= 0x7F; character++)
					{
					assert(::isalpha(character) == isalpha(character));
					assert(::isalnum(character) == isalnum(character));
					assert(::isupper(character) == isupper(character));
					assert(::islower(character) == islower(character));
					assert(::isdigit(character) == isdigit(character));
					assert(::isspace(character) == isspace(character));
					assert(::iscntrl(character) == iscntrl(character));
					assert(::isgraph(character) == isgraph(character));
					assert(::isprint(character) == isprint(character));
					assert(::ispunct(character) == ispunct(character));
					assert(::isxdigit(character) == isxdigit(character));
					assert(::tolower(character) == tolower(character));
					assert(::toupper(character) == toupper(character));
					}
				puts("ascii::PASSED");
				}
	};
}
