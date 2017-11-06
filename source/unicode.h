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
#include <string.h>

#include <vector>

#include "asserts.h"

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
extern unsigned char JASS_unicode_isxmlnamestartchar_data[]; ///< is the given character a XML NameStartChar (see XML production 4)
extern unsigned char JASS_unicode_isxmlnamechar_data[]; ///< is the given character a XML NameStartChar (see XML production 4a)

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
			static constexpr uint32_t replacement_character = 0xFFFD;		///< bad UTF-8 characters become codepoint U+FFFD (the Unicode replacement character designed for that purpose).
			static constexpr size_t max_casefold_expansion_factor = 18;		///< The maximum number of codepoints a case-folded codepoint can take.
			static constexpr size_t max_utf8_bytes = 4;							///< The maximum number of bytes that a UTF8 codepoint can take.
			static constexpr size_t max_codepoint = 0x10FFFF;					///< The highest valid Unicode codepoint
			
		public:
			/*
				UNICODE::UTF8_BYTES()
				---------------------
			*/
			/*!
				@brief Return the number of bytes the UTF-8 character starting at sequence is taking.
				@details Computing the length is done by looking only at the first character in the sequence which contains the length bits.
				@param sequence [in] A pointer to a UTF-8 character
				@return The length (in bytes) of the UTF-8 character starting at sequence, or 0 if the UTF-8 length byte is malformed.
			*/
			static size_t utf8_bytes(const void *sequence)
				{
				if (*(uint8_t *)sequence < 0x80)
					return 1;
				else if ((*(uint8_t *)sequence & 0xE0) == 0xC0)
					return 2;
				else if ((*(uint8_t *)sequence & 0xF0) == 0xE0)
					return 3;
				else if ((*(uint8_t *)sequence & 0xF8) == 0xF0)
					return 4;
					
				return 0;
				}

			/*
				UNICODE::UTF8_BYTES()
				---------------------
			*/
			/*!
				@brief Return the number of bytes necessary to convert the codepoint into UTF-8.
				@param codepoint [in] The codepoint to convert.
				@return The minimum length (in bytes) of the UTF-8 sequence necessary to store codepoint.
			*/
			static size_t utf8_bytes(uint32_t codepoint)
				{
				if (codepoint <= 0x007F)
					return 1;
				if (codepoint <= 0x07FF)
					return 2;
				if (codepoint <= 0xFFFF)
					return 3;
				if (codepoint <= 0x1FFFFF)
					return 4;

				return 0;
				}

			/*
				UNICODE::ISUTF8()
				-----------------
			*/
			/*!
				@brief Examine the UTF-8 sequence to determine whether or not it is valid UTF-8.
				@param utf8_start [in] The start of the UTF-8 sequence.
				@param end_of_buffer [in] The end of the buffer containing the UTF-8 sequence (not necessarily the end of the UTF-8 sequence).
				@return The length of the UTF-8 sequence, or 0 on bad sequence.  That is, true or false.
			*/
			static size_t isutf8(const void *utf8_start, const void *end_of_buffer)
				{
				/*
					get the length
				*/
				size_t number_of_bytes = utf8_bytes(utf8_start);

				/*
					check the length for validity
				*/
				if (number_of_bytes == 0)
					return 0;

				/*
					check for overflow
				*/
				if ((uint8_t *)utf8_start + number_of_bytes > end_of_buffer)
					return 0;

				/*
					check each byte is a valid UTF-8 continuation byte.
				*/
				const uint8_t *byte = (uint8_t *)utf8_start;
				for (size_t byte_number = 1; byte_number < number_of_bytes; byte_number++)
					if ((*++byte >> 6) != 2)
						return 0;

				/*
					success so return the length of the sequence
				*/
				return number_of_bytes;
				}

			/*
				UNICODE::UTF8_TO_CODEPOINT()
				----------------------------
			*/
			/*!
				@brief Convert the UTF-8 sequence into a Unicode codepoin (i.e. decode the UTF-8).
				@param utf8_start [in] The start of the UTF-8 sequence.
				@param end_of_buffer [in] The end of the buffer containing the UTF-8 sequence (not necessarily the end of the UTF-8 sequence).
				@param bytes_consumed [out] The number of bytes taken by the UTF-8 sequence (computed as a byproduct of decoding).
				@return The Unicode codepoint, or unicode::replacement_character on invalid input.
			*/
			static uint32_t utf8_to_codepoint(const void *utf8_start, const void *end_of_buffer, size_t &bytes_consumed)
				{
				const uint8_t *bytes = (const uint8_t *)utf8_start;

				bytes_consumed = isutf8(bytes, end_of_buffer);

				switch (bytes_consumed)
					{
					case 0:
						return replacement_character;						// invaid UTF-8 sequence
					case 1:
						return *bytes;
					case 2:
						return ((*bytes & 0x1F) << 6) | (*(bytes + 1) & 0x3F);
					case 3:
						return ((*bytes & 0x0F) << 12) | ((*(bytes + 1) & 0x3F) << 6) | (*(bytes + 2) & 0x3F);
					case 4:
						{
						uint32_t got = ((*bytes & 0x07) << 18) | ((*(bytes + 1) & 0x3F) << 12) | ((*(bytes + 2) & 0x3F) << 6) | (*(bytes + 3) & 0x3F);
						return got <= max_codepoint ? got : replacement_character;
						}
					default:
						return replacement_character;		// LCOV_EXCL_LINE	// Cannot happen, but Xcode gives a warning if this line is missing.
					}
				}

			/*
				UNICODE::CODEPOINT_TO_UTF8()
				----------------------------
			*/
			/*!
				@brief Encode the Unicode codepoint in UTF-8 into buffer utf8_start, not exceeding end_of_buffer.
				@param utf8_start [out] The start of the UTF-8 sequence.
				@param end_of_buffer [in] The end of the buffer to contain the UTF-8 sequence (not necessarily the end of the UTF-8 sequence).
				@param codepoint [in] The codepoint to turn into UTF-8.
				@return The number of bytes written into utf8_start, or 0 on error (e.g. overrun would occur, or invalid codepoint).
			*/
			static size_t codepoint_to_utf8(void *utf8_start, const void *end_of_buffer, uint32_t codepoint)
			{
			uint8_t *bytes = (uint8_t *)utf8_start;

			/*
				Find out how many bytes are needed and make sure it'll fit in the given space.
			*/
			size_t bytes_required = utf8_bytes(codepoint);
			if (bytes + bytes_required > end_of_buffer)
				return 0;

			/*
				It fits so write into the given space.
			*/
			switch (bytes_required)
				{
				case 0:
					break;			// invalid codepoint
				case 1:
					bytes[0] = (uint8_t)(codepoint);
					break;
				case 2:
					bytes[0] = (uint8_t)(0xC0 | (codepoint >> 6));
					bytes[1] = (uint8_t)(0x80 | (codepoint & 0x3F));
					break;
				case 3:
					bytes[0] = (uint8_t)(0xE0 | (codepoint >> 12));
					bytes[1] = (uint8_t)(0x80 | ((codepoint & 0xFC0) >> 6));
					bytes[2] = (uint8_t)(0x80 | (codepoint & 0x3F));
					break;
				case 4:
					bytes[0] = (uint8_t)(0xF0 | (codepoint >> 18));
					bytes[1] = (uint8_t)(0x80 | ((codepoint & 0x3F000) >> 12));
					bytes[2] = (uint8_t)(0x80 | ((codepoint & 0xFC0) >> 6));
					bytes[3] = (uint8_t)(0x80 | (codepoint & 0x3F));
					break;
				}

			/*
				return the number of bytes just written
			*/
			return bytes_required;
			}

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
				UNICODE::TOCASEFOLD()
				---------------------
			*/
			/*!
				@brief Return a pointer to a 0 terminated array of codepoints that is the casefolded normalised codepoint.
				@details This is the JASS character normalisation method.  It converts to Unicode "NFKD", strips all non-alpha-numerics, then
				performs Unicode casefolding "C+F".  As unicode decomposition is involved (and casefolding) the resulting string can be considerably
				larger than a single codepoint.  The worst case is the single codepoint U+FDFA becoming 18 codepoints once normalisd.  Two codepoints,
				U+FDFA and U+FDFB expand into strings that contain spaces; it is the caller's responsibility to manage this should it need to be managed.
				@param codepoint [in] The codepoint to normalise.
			*/
			static inline const uint32_t *tocasefold(uint32_t codepoint)
				{
				return JASS_normalisation[codepoint];
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
			static inline int isalpha(uint32_t codepoint)
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
			static inline int isalnum(uint32_t codepoint)
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
			static inline int isupper(uint32_t codepoint)
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
			static inline int islower(uint32_t codepoint)
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
			static inline int iscntrl(uint32_t codepoint)
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
			static inline int isdigit(uint32_t codepoint)
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
			static inline int isgraph(uint32_t codepoint)
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
			static inline int ispunct(uint32_t codepoint)
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
			static inline int isspace(uint32_t codepoint)
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
			static inline int isuspace(uint32_t codepoint)
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
			static inline int isxdigit(uint32_t codepoint)
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
			static inline int ismark(uint32_t codepoint)
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
			static inline int issymbol(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_issymbol_data[byte] & bit;
				}
			
			/*
				UNICODE::ISXMNAMESTARTCHAR()
				----------------------------
			*/
			/*!
				@brief Check to see if the codepoint is a valid character to start and XML tag name with.
				@details According to XML production 4, valid Unicode characters are (":" | [A-Z] | "_" | [a-z] | [\#xC0-\#xD6] | [\#xD8-\#xF6] | [\#xF8-\#x2FF] | [\#x370-\#x37D] | [\#x37F-\#x1FFF] | [\#x200C-\#x200D] | [\#x2070-\#x218F] | [\#x2C00-\#x2FEF] | [\#x3001-\#xD7FF] | [\#xF900-\#xFDCF] | [\#xFDF0-\#xFFFD] | [\#x10000-\#xEFFFF])
				For details see: "Extensible Markup Language (XML) 1.0 (Fifth Edition) W3C Recommendation 26 November 2008", https://www.w3.org/TR/REC-xml
				@param codepoint [in] The Unicode codepoint to check.
				@return true if an XML NameStartChar character, else false.
			*/
			static inline int isxmlnamestartchar(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isxmlnamestartchar_data[byte] & bit;
				}

			/*
				UNICODE::ISXMNAMETCHAR()
				------------------------
			*/
			/*!
				@brief Check to see if the codepoint is a valid character to follow a NameStartChar in an XML tag name.
				@details According to XML production 4a, valid Unicode characters are (NameStartChar | "-" | "." | [0-9] | \#xB7 | [\#x0300-\#x036F] | [\#x203F-\#x2040])
				For details see: "Extensible Markup Language (XML) 1.0 (Fifth Edition) W3C Recommendation 26 November 2008", https://www.w3.org/TR/REC-xml
				@param codepoint [in] The Unicode codepoint to check.
				@return true if an XML NameChar character, else false.
			*/
			static inline int isxmlnamechar(uint32_t codepoint)
				{
				uint32_t byte = codepoint >> 3;
				uint32_t bit = 1 << (codepoint & 0x07);

				return JASS_unicode_isxmlnamechar_data[byte] & bit;
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
				/*
					Test the UTF-8 methods.  These examples came from the Wikipedia here: https://en.wikipedia.org/wiki/UTF-8
				*/
				const uint8_t sequence_1[] = {0x24, 0x00, 0x00, 0x00};
				const uint8_t sequence_2[] = {0xC2, 0xA2, 0x00, 0x00};
				const uint8_t sequence_3[] = {0xE2, 0x82, 0xAC, 0x00};
				const uint8_t sequence_4[] = {0xF0, 0x90, 0x8D, 0x88};
				const uint8_t sequence_bad[] = {0xF8, 0x00, 0x00, 0x00, 0x00};
				
				/*				
					Check that the lengths are correctly decoded
				*/
				JASS_assert(utf8_bytes(sequence_1) == 1);
				JASS_assert(utf8_bytes(sequence_2) == 2);
				JASS_assert(utf8_bytes(sequence_3) == 3);
				JASS_assert(utf8_bytes(sequence_4) == 4);
				JASS_assert(utf8_bytes(sequence_bad) == 0);			// fail case
				JASS_assert(isutf8(sequence_bad, sequence_bad + sizeof(sequence_bad)) == false);
				
				/*
					Check that the lengths are correctly encoded
				*/
				JASS_assert(utf8_bytes(0x24) == 1);
				JASS_assert(utf8_bytes(0xA2) == 2);
				JASS_assert(utf8_bytes(0x20AC) == 3);
				JASS_assert(utf8_bytes(0x10348) == 4);
				JASS_assert(utf8_bytes(0x200000) == 0);				// fail case

				
				/*
					Check that it can decode UTF-8 correctly
				*/
				size_t consumed;
				JASS_assert(utf8_to_codepoint(sequence_1, sequence_1 + sizeof(sequence_1), consumed) == 0x24 && consumed == 1);
				JASS_assert(utf8_to_codepoint(sequence_2, sequence_2 + sizeof(sequence_2), consumed) == 0xA2 && consumed == 2);
				JASS_assert(utf8_to_codepoint(sequence_3, sequence_3 + sizeof(sequence_3), consumed) == 0x20AC && consumed == 3);
				JASS_assert(utf8_to_codepoint(sequence_4, sequence_4 + sizeof(sequence_4), consumed) == 0x10348 && consumed == 4);
				JASS_assert(utf8_to_codepoint(sequence_bad, sequence_bad + sizeof(sequence_bad), consumed) == unicode::replacement_character && consumed == 0);				// fail case

				
				/*
					Check that it can encode UTF-8 correctly
				*/
				uint8_t buffer[8];
				JASS_assert(codepoint_to_utf8(buffer, buffer + sizeof(buffer), 0x24) == 1);
				JASS_assert(memcmp(buffer, sequence_1, 1) == 0);
				JASS_assert(codepoint_to_utf8(buffer, buffer + sizeof(buffer), 0xA2) == 2);
				JASS_assert(memcmp(buffer, sequence_2, 2) == 0);
				JASS_assert(codepoint_to_utf8(buffer, buffer + sizeof(buffer), 0x20AC) == 3);
				JASS_assert(memcmp(buffer, sequence_3, 3) == 0);
				JASS_assert(codepoint_to_utf8(buffer, buffer + sizeof(buffer), 0x10348) == 4);
				JASS_assert(memcmp(buffer, sequence_4, 4) == 0);
				JASS_assert(codepoint_to_utf8(buffer, buffer + sizeof(buffer), 0x200000) == 0);			// failure case
				JASS_assert(codepoint_to_utf8(buffer, buffer + 1, 0x10348) == 0);						// failure case

				/*
					Test the ctype-like methods
				*/
				for (uint8_t character = 0; character <= 0x7F; character++)
					{
					JASS_assert((::isalpha(character) && unicode::isalpha(character)) || ((!::isalpha(character) && !unicode::isalpha(character))));
					JASS_assert((::isalnum(character) && unicode::isalnum(character)) || ((!::isalnum(character) && !unicode::isalnum(character))));
					JASS_assert((::isupper(character) && unicode::isupper(character)) || ((!::isupper(character) && !unicode::isupper(character))));
					JASS_assert((::islower(character) && unicode::islower(character)) || ((!::islower(character) && !unicode::islower(character))));
					JASS_assert((::isdigit(character) && unicode::isdigit(character)) || ((!::isdigit(character) && !unicode::isdigit(character))));
					JASS_assert((::isspace(character) && unicode::isspace(character)) || ((!::isspace(character) && !unicode::isspace(character))));
					JASS_assert((::iscntrl(character) && unicode::iscntrl(character)) || ((!::iscntrl(character) && !unicode::iscntrl(character))));
					JASS_assert((::isxdigit(character) && unicode::isxdigit(character)) || ((!::isxdigit(character) && !unicode::isxdigit(character))));

					/*
						Unicode does not consider character 32 (space) to be a graph character as it is a space character
					*/
					if (character == 32)
						JASS_assert((::isgraph(character) && !unicode::isgraph(character)) || ((!::isgraph(character) && unicode::isgraph(character))));
					else
						JASS_assert((::isgraph(character) && unicode::isgraph(character)) || ((!::isgraph(character) && !unicode::isgraph(character))));
					/*
						The rules for ispunct() are very different betwen C and Unicode so we won't even bother to check
					*/
					if (character <= 0x7F && unicode::isalnum(character))
						{
						std::vector<uint32_t> casefold;
						tocasefold(casefold, character);
						JASS_assert(casefold.size() == 1 && casefold[0] == (size_t)::tolower(character));
						}
					}
				/*
					Check the methods whose answer is either not in ASCII or differ from ASCII
				*/
				JASS_assert(unicode::ispunct(','));
				JASS_assert(unicode::isspace(' '));
				JASS_assert(unicode::isuspace(' '));
				JASS_assert(unicode::ismark(0x300));
				JASS_assert(unicode::issymbol(0x2600));

				puts("unicode::PASSED");
				}
		};
}
