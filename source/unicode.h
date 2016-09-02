/*
	UNICODE.H
	---------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Methods that work on Unicode codepoints.
	@details These methods are (where possible) Unicode equivelants to C's ctype methods.  For the ASCII block
	these methods should exactly match C's is() routines. The case changing methods toupper() an tolower() are
	replaced with a tocasefold() method because uppercase and lowercase are meaningless for many languages.
	
	See unicode_database_to_c.cpp (in the tools directory) for how to generate the matching unicode.cpp file.
	
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <ctype.h>
#include <stdint.h>
#include <assert.h>

extern unsigned char JASS_unicode_isalpha_data[];	///< is the given codepoint alphabetic
extern unsigned char JASS_unicode_isalnum_data[];	///< is the given codepoint alphanumeric
extern unsigned char JASS_unicode_isupper_data[];	///< is the given codepoint uppercase
extern unsigned char JASS_unicode_islower_data[];	///< is the given codepoint lowercase
extern unsigned char JASS_unicode_iscntrl_data[];	///< is the given codepoint a control character
extern unsigned char JASS_unicode_isdigit_data[];	///< is the given codepoint a numeric or digit character
extern unsigned char JASS_unicode_isgraph_data[];	///< is the given codepoint a graphic character
extern unsigned char JASS_unicode_ispunct_data[];	///< is the given codepoint punctuation
extern unsigned char JASS_unicode_isspace_data[];	///< is the given codepoint whitespace
extern unsigned char JASS_unicode_isuspace_data[];	///< is the given codepoint a space under the Unicode definition of space
extern unsigned char JASS_unicode_isxdigit_data[];	///< is the given codepoint a hexadecimal digit
extern unsigned char JASS_unicode_ismark_data[];	///< is the given codepoint a mark
extern unsigned char JASS_unicode_issymbol_data[];	///< is the given codepoint a symbol

extern const uint32_t *JASS_normalisation[];		///< an array of pointers to JASS normalised codepoints for the given codepoint

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
				UNICODE::TOCASEFOLD()
				---------------------
			*/
			/*!
				@brief Strip all accents, non-alphanumerics, and then casefold.
				@details This is the JASS character normalisation method.  It converts to Unicode "NFKD", strips all non-alpha-numerics, then
				performs Unicode casefolding "C+F".  As unicode decomposition is involved (and casefolding) the resulting string can be considerably
				larger than a single codepoint.  The worst case is the single codepoint U+FDFA becoming 18 codepoints once normalisd.  Two codepoints,
				U+FDFA and U+FDFB expand into strings that contain spaces; it is the caller's responsibility to manage this should it need to be managed.
				@param casefolded [out] The normalise Unicode codepoint string is appended to this parameter.
				@param codepoint [in] The codepoint to normalise.
			*/
			static inline void tocasefold(std::vector<uint32_t> &casefolded, uint32_t codepoint)
				{
				const uint32_t *got = JASS_normalisation[codepoint];
				while (*got != 0)
					casefolded.push_back(*got++);
				}
			
			/*
				UNICODE::ISALPHA()
				------------------
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
				UNICODE::ISALNUM()
				------------------
			*/
			/*!
				@brief Unicode version is isalnum().
				@details Character is of the general Unicode category "Alphabetic" or of the general Unicode category "Nd, Nl, No".  That is,
				the character is alphanumeric
				@param codepoint [in] The Unicode codepoint to check.
				@return true if alphabetic, else false.
			*/
			static inline bool isalum(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isalnum_data[byte] & bit;
				}
			
			/*
				UNICODE::ISUPPER()
				------------------
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
				UNICODE::ISLOWER()
				------------------
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
				UNICODE::ISCNTRL()
				------------------
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
				UNICODE::ISDIGIT()
				------------------
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
				UNICODE::ISGRAPH()
				------------------
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
				UNICODE::ISPUNCT()
				------------------
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
				UNICODE::ISSPACE()
				------------------
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
				UNICODE::ISUSPACE()
				-------------------
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
				UNICODE::ISXDIGIT()
				-------------------
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
				UNICODE::ISMARK()
				-----------------
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
				UNICODE::ISSYMBOL()
				-------------------
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
				UNICODE::UNITTEST()
				-------------------
				The initial unit test was against the D language standard library (Phobos) but
				that test fails because Phobos std.uni isn't against any particular version of the
				standard and these methods are generated against Unicode version 9.0.
			*/
			/*!
				@brief Unit test this class.
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
					if (character <= 0x7F && isalnum(character))
						{
						std::vector<uint32_t> casefold;
						tocasefold(casefold, character);
						assert(casefold.size() == 1 && casefold[0] == ::tolower(character));
						}
					}
				puts("unicode::PASSED");
				}
		};
}
