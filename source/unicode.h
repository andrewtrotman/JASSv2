/*
	UNICODE.H
	---------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief ctype methods that work on Unicode codepoints rather than on ASCII values.
	@details As Unicode is a superset of ASCII, these methods should exactly match C's is() routines.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <ctype.h>
#include <stdint.h>
#include <assert.h>

extern unsigned char JASS_unicode_isalpha_data[];
extern unsigned char JASS_unicode_isupper_data[];
extern unsigned char JASS_unicode_islower_data[];
extern unsigned char JASS_unicode_iscntrl_data[];
extern unsigned char JASS_unicode_isdigit_data[];
extern unsigned char JASS_unicode_isgraph_data[];
extern unsigned char JASS_unicode_ispunct_data[];
extern unsigned char JASS_unicode_isspace_data[];
extern unsigned char JASS_unicode_isuspace_data[];
extern unsigned char JASS_unicode_isxdigit_data[];
extern unsigned char JASS_unicode_ismark_data[];
extern unsigned char JASS_unicode_issymbol_data[];


namespace JASS
	{
	/*
		CLASS UNICODE
		-------------
	*/
	/*!
		@brief Implementation of the ctype methods on Unicode codepoints.
	*/
	class unicode
		{
		public:
			/*
				ISALPHA()
				---------
			*/
			/*!
				@brief Unicode version is isalpha().
				@details Character is of the general Unicode category "Alphabetic".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if alphabetic, else false.
			*/
			static inline bool isalpha(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isalpha_data[byte] & bit;
				}
				
			/*
				ISUPPER()
				---------
			*/
			/*!
				@brief Unicode version is isupper().
				@details Character is of the general Unicode category "Unicode uppercase".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if uppercase alphabetic, else false.
			*/
			static inline bool isupper(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isupper_data[byte] & bit;
				}
				
			/*
				ISLOWER()
				---------
			*/
			/*!
				@brief Unicode version is islower().
				@details Character is of the general Unicode category "Unicode lowercase".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if lowercase alphabetic, else false.
			*/
			static inline bool islower(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_islower_data[byte] & bit;
				}
				
			/*
				ISCNTRL()
				---------
			*/
			/*!
				@brief Unicode version is iscntrl().
				@details Character is of the general Unicode category "Cc".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a control character, else false.
			*/
			static inline bool iscntrl(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_iscntrl_data[byte] & bit;
				}
				
			/*
				ISDIGIT()
				---------
			*/
			/*!
				@brief Unicode version is isdigit().
				@details Character is of the general Unicode category "Nd, Nl, No".  These categories include
				numeric characters that are not digits, but this routine maintains its name for backwards compatibility with the "C" routine
				of the same name.
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a numeric character, else false.
			*/
			static inline bool isdigit(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isdigit_data[byte] & bit;
				}
			
			/*
				ISGRAPH()
				---------
			*/
			/*!
				@brief Unicode version is isgraph().
				@details Character is of the general Unicode category "L, M, N, P, S, Zs".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a graphical character, else false.
			*/
			static inline bool isgraph(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isgraph_data[byte] & bit;
				}

			/*
				ISPUNCT()
				---------
			*/
			/*!
				@brief Unicode version is ispunct().
				@details Character is of the general Unicode category "Pd, Ps, Pe, Pc, Po, Pi, Pf".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a punctuation character, else false.
			*/
			static inline bool ispunct(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_ispunct_data[byte] & bit;
				}

			/*
				ISSPACE()
				---------
			*/
			/*!
				@brief Unicode version is isspace(), by the "C" isspace() and Unicode definition.
				@details Character is "Part of C0(tab, vertical tab, form feed, carriage return, and linefeed characters), Zs, Zl, Zp, and NEL(U+0085)".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a space character, else false.
			*/
			static inline bool isspace(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isspace_data[byte] & bit;
				}

			/*
				ISUSPACE()
				----------
			*/
			/*!
				@brief Unicode version is isspace(), by the Unicode definition.
				@details Character is "Zs".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a space character, else false.
			*/
			static inline bool isuspace(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isuspace_data[byte] & bit;
				}

			/*
				ISXDIGIT()
				----------
			*/
			/*!
				@brief Unicode version is isxdigit().
				@details Character is of the general Unicode category "Hex_Digit or ASCII_Hex_Digit".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a hex character, else false.
			*/
			static inline bool isxdigit(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isxdigit_data[byte] & bit;
				}

			/*
				ISMARK()
				--------
			*/
			/*!
				@brief Check to see if the codepoint is a mark.
				@details Character is of the general Unicode category "M".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a mark character, else false.
			*/
			static inline bool ismark(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_ismark_data[byte] & bit;
				}

			/*
				ISSYMBOL()
				----------
			*/
			/*!
				@brief Check to see if the codepoint is a symbol.
				@details Character is of the general Unicode category "S".
				@param codepoint [in] The Unicode codepoint to check.
				@return true if a symbol character, else false.
			*/
			static inline bool issymbol(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_issymbol_data[byte] & bit;
				}
				
			/*
				UNITTEST()
				----------
				The initial unit test was against the D language standard library (Phobos) but
				that test fails because Phobos std.uni isn't against any particular version of the
				standard and these methods are generated against Unicode version 9.0.
			*/
			static void unittest(void)
				{
				for (uint8_t character = 0; character <= 0x7F; character++)
					{
					assert(::isalpha(character) == isalpha(character));
					assert(::isupper(character) == isupper(character));
					assert(::islower(character) == islower(character));
					assert(::isdigit(character) == isdigit(character));
					assert(::isspace((uint8_t)character) == isspace(character));
					assert(::isxdigit((uint8_t)character) == isxdigit(character));
					assert(::iscntrl(character) == iscntrl(character));

					/*
						Unicode does not consider character 32 (space) to be a graph character as it is a space character
					*/
					if (character == 32)
						assert(::isgraph(character) != isgraph(character));
					else
						assert(::isgraph(character) == isgraph(character));
					/*
						The rules for ispunct() are very different betwen C and Unicode so we won't even bother to check
					*/
					}
				puts("unicode::PASSED");
				}
		};
}
