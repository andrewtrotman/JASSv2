/*
	UNICODE_DATABASE_TO_C.CPP
	-------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Convert the Unicode Standard UnicodeData.txt and PropList.txt into a bunch of methods similar to the
	C standard library ctype routines.
	
	The latest version of UnicodeData.txt can be found here: http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
	The latest version of PropList.txt can be found here: http://www.unicode.org/Public/UCD/latest/ucd/PropList.txt
	The latest version of CaseFolding.txt can be found here: http://www.unicode.org/Public/UCD/latest/ucd/CaseFolding.txt
	The documentation on how to read them is here: http://www.unicode.org/reports/tr44/#UnicodeData.txt

	The functions we wish to mirror are described here: http://www.cplusplus.com/reference/cctype/
	
	They are:
		NAME           VARIABLE IN THIS PROGRAM		D'S DEFINITION
		isalnum			alpha + digit						-
		isalpha			alpha									"general Unicode category: Alphabetic" - isAlpha()
		isblank
		iscntrl			control								"general Unicode category: Cc" - isControl()
		isdigit			digit									"general Unicode category: Nd, Nl, No" - isNumber()
		isgraph			graphical							"general Unicode category: L, M, N, P, S, Zs" - isGraphical()
		islower			lowercase							"Unicode lowercase" - isLower()
		isprint
		ispunct			punc									"general Unicode category: Pd, Ps, Pe, Pc, Po, Pi, Pf" - IsPunctuation()
		-					space									"general Unicode category: Zs" - isSpace()
		isupper			uppercase							"Unicode uppercase" - isUpper()
		isxdigit			xdigit
		isspace			white									"Part of C0(tab, vertical tab, form feed, carriage return, and linefeed characters), Zs, Zl, Zp, and NEL(U+0085)" - isWhite()
		-					mark									"general Unicode category: Mn, Me, Mc"
		-					symbol								"general Unicode category: Sm, Sc, Sk, So"
		tolower
		toupper
	
	For the latext XML spec see: https://www.w3.org/TR/REC-xml/#NT-extSubsetDecl  These definitions are taken from "Extensible Markup Language (XML) 1.0 (Fifth Edition) W3C Recommendation 26 November 2008"
	Added (for XML and HTML parsing) are:
		NAME                                         DEFINITION
		isxmlnamestartchar                           ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
		isxmlnamechar                                NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]

	These are then dumped out as C routines.
*/
/*!
	@file
	@brief Generate C sourcecode for is() methods for Unicode UTF-8
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <iostream>
#include <vector>
#include <map>

#include "file.h"
#include "bitstring.h"

/*
	The highest possible Unicode codepoint.
*/
static const size_t MAX_CODEPOINT = 0x10FFFF;

/*
	alphabetic characters.  Note that alpha != lowercase + uppercase because there are many
	characters that are caseless (such as the phonetic characters).
*/
std::vector<size_t>alpha;				///< list of alphabetical characters
std::vector<size_t>uppercase;			///< list of uppercase characters
std::vector<size_t>lowercase;			///< list of lowercase characgers

/*
	Digits, or more accurately number characters including digit characters
*/
std::vector<size_t>digit;				///< list of digits

/*
	Alphanumerics
*/
std::vector<size_t>alphanumeric;		///< list of alphanumeric characters

/*
	Punctuaton
*/
std::vector<size_t>punc;					///< list of punctuation symbols

/*
	Space characters (by the unicode definition of space)
*/
std::vector<size_t>space;				///< list of Unicode space characters (not a superset C's isspace())

/*
	White Space characters (by the D Phobos definition of whitespace) which is:
	"general Unicode category: Part of C0(tab, vertical tab, form feed, carriage return, and linefeed characters), Zs, Zl, Zp, and NEL(U+0085))"
*/
std::vector<size_t>whitespace;			///< list of space characters (is a superset C's isspace())

/*
	Diacritic marks
*/
std::vector<size_t>mark;					///< list of diacritic marks

/*
	Symbols
*/
std::vector<size_t>symbol;				///< list of symbols

/*
	Control characters
*/
std::vector<size_t>control;				///< list of control characters

/*
	Graphical characters
*/
std::vector<size_t>graphical;			///< list of graphical (printable) characters

/*
	Hexadecimal digits
*/
std::vector<size_t>xdigit;				///< list of Unicode defined hexadecimal characters

/*
	JASS normalisation
*/
std::map<int, std::vector<int>> JASS_normalisation;		///< JASS normalisation rules (one codepoint can become more than one codepoint)

/*
	Unicode Casefolding
*/
std::map<int, std::vector<int>> casefold;		///< The casefolded version of the codepoint

/*
	Fast alphanumeric lookup (used as part of JASS normalisation)
*/
std::map<size_t, bool> codepoint_isalnum;

/*
	XML NameStartChar and XML NameChar
*/
std::vector<size_t> xmlnamestartchar;
std::vector<size_t> xmlnamechar;

/*
	USAGE()
	------
*/
/*!
	@brief Tell the user how to use this program.
	@param filename [in] the name of this executable (normally argv[0]).
*/
// LCOV_EXCL_START
void usage(char *filename)
	{
	printf("Usage:%s <UnicodeData.txt> <PropList.txt> <CaseFolding.txt>\n", filename);
	exit(0);
	}
// LCOV_EXCL_STOP

/*
	SERIALISE()
	-----------
	serialise into the methods
*/
/*!
	@brief Walk through each of the lists turning it into C code.
	@param operation [in] The name of the "is" operation.
	@param list [in] The list of which codepoints are valid for this "is" condition.
*/
void serialise(const std::string &operation, const std::vector<size_t> &list)
	{
	static bool first_time = true;		// used to make sure includes are only included once
	JASS::bitstring bits;					// the list is converted into this bitstring
	
	/*
		If this is the first time we're called then output any header files we need
	*/
	if (first_time)
		{
	   time_t ltime;
	   time(&ltime);

		puts("/*");
		printf("\tThis file was generated on %s", ctime(&ltime));
		printf("\tIt was generated by unicode_database_to_c as part of the JASS build process\n");
		puts("*/");
		puts("#include <stdint.h>");
		}
	first_time = false;
	
	/*
		Make sure the bitstring is long enough (the highest Codepoint is MAX_CODEPOINT, so the bitstring is one bit longer (to store the 0)
	*/
	bits.resize(MAX_CODEPOINT + 1);

	/*
		Turn the vector into a bitstring
	*/
	for (const auto &codepoint : list)
		bits.unsafe_setbit(codepoint);
		
	/*
		Turn the bitstring into a bytestring
	*/
	size_t length;
	auto bytes = bits.serialise(length);
	
	/*
		Write out the name of the operation
	*/
	printf("unsigned char JASS_unicode_%s_data[] = {", operation.c_str());
	
	/*
		Write out the bytestring as C
	*/
	for (size_t byte = 0; byte < length; byte++)
		{
		if (byte % 16 == 0)
			puts("");
		printf("0x%02X,", bytes[byte]);
		}

	/*
		Correctly terminate the string
	*/
	printf("};\n\n");

	/*
		Add a C Method.  There are several reasons for doing this, but it basically boils down to the
		unit tests doing a comparison to the "D" methods and the need to link against a non-inline method.
	*/
	printf("int JASS_unicode_%s(uint32_t codepoint)\n", operation.c_str());
	puts("{");
	puts("uint32_t byte = codepoint >> 3;");
	puts("uint32_t bit = 1 << (codepoint & 0x07);");
	printf("return JASS_unicode_%s_data[byte] & bit;\n", operation.c_str());
	puts("}\n");
	}

/*
	PROCESS_UNICODEDATA()
	---------------------
	0000;<control>;Cc;0;BN;;;;;N;NULL;;;;
	
	The order of the parameters is:
		codepoint
		name
		general category (is it a number or a letter, etc)
		...
		
	We're interested in the codepoint and the category only (nothing else), but sometimes we see ranges:
	
		20000;<CJK Ideograph Extension B, First>;Lo;0;L;;;;;N;;;;;
		2A6D6;<CJK Ideograph Extension B, Last>;Lo;0;L;;;;;N;;;;;
	
	In which case we *are* interested in the range.

*/
/*!
	@brief Given a line form the UnicodeData.txt file, work out which functions should know about this codepoint
	@param line [in] The line to process.
	@param last_codepoint [in] The start of the current unicode range
	@return The last Unicode codepoint we've seen (which must be passed back next call for ranges).
*/
unsigned process_unicodedata(const char *line, unsigned last_codepoint)
	{
	unsigned codepoint;			// unicode number of the codepoint
	unsigned range_start;		// start of the range for this line in the Unicode database
	unsigned range_end;			// end of the range for this line in the Unicode database
	char category[3];		// Two letter catagory code for the codepoint
	
	/*
		the first parmeter is the codepoint
	*/
	if (strstr(line, ", Last>") != nullptr)
		{
		range_start = last_codepoint;
		sscanf(line, "%x", &range_end);
		}
	else
		{
		sscanf(line, "%x", &range_start);
		range_end = range_start;
		}

	/*
		step over the second parameter
	*/
	const char *semicolon;
	if ((semicolon = strchr(line, ';')) == nullptr)
		exit(printf("Badly formed line:%s\n", line));				// LCOV_EXCL_LINE
	if ((semicolon = strchr(semicolon + 1, ';')) == nullptr)
		exit(printf("Badly formed line:%s\n", line));				// LCOV_EXCL_LINE
		
	/*
		The third parameter is the type
	*/
	strncpy(&category[0], semicolon + 1, 2);
	category[2] = '\0';
	
	/*
		Work out what type of character we have, according the the C ctype.h conventions
		
		Alphabetic is defined as: Lowercase + Uppercase + Lt + Lm + Lo + Nl + Other_Alphabetic
		where
			Lowercase is defined as:Ll + Other_Lowercase
			Uppercase is defined as:Lu + Other_Uppercase
		and
			Other_Alphabetic, Other_Lowercase, and  Other_Uppercase ae defined in PropList.txt
		
	*/
	for (codepoint = range_start; codepoint <= range_end; codepoint++)
		{
		/*
			Graphical characters
		*/
		if (*category == 'L')							// Letter
			graphical.push_back(codepoint);
		if (*category == 'M')							// Mark
			graphical.push_back(codepoint);
		if (*category == 'N')							// Number
			graphical.push_back(codepoint);
		if (*category == 'P')							// Punctuation
			graphical.push_back(codepoint);
		if (*category == 'S')							// Symbol
			graphical.push_back(codepoint);
		if (strcmp(category, "Zs") == 0)				// Space_Separator
			graphical.push_back(codepoint);
			
		/*
			Alphabetical, upppercase, and lowercase characters
		*/
		if (strcmp(category, "Lu") == 0)				// an uppercase letter
			{
			uppercase.push_back(codepoint);
			alpha.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "Ll") == 0)				// a lowercase letter
			{
			lowercase.push_back(codepoint);
			alpha.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "Lt") == 0)				// a digraphic character, with first part uppercase
			{
			alpha.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "Lm") == 0)				// a modifier letter
			{
			alpha.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "Lo") == 0)				// other letters, including sylables and ideographs
			{
			alpha.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "Nl") == 0)				// a letterlike numeric character
			{
			alpha.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}

		/*
			Numeric characters
		*/
		if (strcmp(category, "Nd") == 0)				// Decimal_Number
			{
			digit.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "Nl") == 0)				// Letter_Number
			{
			digit.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}
		if (strcmp(category, "No") == 0)				// Other_Number
			{
			digit.push_back(codepoint);
			alphanumeric.push_back(codepoint);
			}

		/*
			Punctuation characters
		*/
		if (strcmp(category, "Pc") == 0)				// Connector_Punctuation
			punc.push_back(codepoint);
		if (strcmp(category, "Pd") == 0)				// Dash_Punctuation
			punc.push_back(codepoint);
		if (strcmp(category, "Ps") == 0)				// Open_Punctuation
			punc.push_back(codepoint);
		if (strcmp(category, "Pe") == 0)				// Close_Punctuation
			punc.push_back(codepoint);
		if (strcmp(category, "Pi") == 0)				// Initial_Punctuation
			punc.push_back(codepoint);
		if (strcmp(category, "Pf") == 0)				// Final_Punctuation
			punc.push_back(codepoint);
		if (strcmp(category, "Po") == 0)				// Other_Punctuation
			punc.push_back(codepoint);

		/*
			Space characters
		*/
		if (strcmp(category, "Zs") == 0)				// Space_Separator
			space.push_back(codepoint);

		/*
			Whitespace characters (a more useful version of space characters)
		*/
		if (strcmp(category, "Zs") == 0)				// Space_Separator
			whitespace.push_back(codepoint);
		if (strcmp(category, "Zl") == 0)				// Line_Separator
			whitespace.push_back(codepoint);
		if (strcmp(category, "Zp") == 0)				// Paragraph_Separator
			whitespace.push_back(codepoint);
		if (codepoint <= 0xFF && isspace((int)codepoint))		// space, tab, vertical tab, form feed, carriage return, and linefeed characters
			whitespace.push_back(codepoint);
		if (codepoint == 0x85)								// NEL character (the Next Line character)
			whitespace.push_back(codepoint);

		/*
			Mark characters
		*/
		if (*category == 'M')									// Mark
			mark.push_back(codepoint);

		/*
			Symbol characters
		*/
		if (*category == 'S')									// Symbol
			symbol.push_back(codepoint);

		/*
			Control characters
		*/
		if (strcmp(category, "Cc") == 0)						// a C0 or C1 control code
			control.push_back(codepoint);

		/*
			XML NameStartChar and NameChar
		*/
		if ((codepoint == ':') || (codepoint >= 'A' && codepoint <= 'Z') || (codepoint == '_') || (codepoint >= 'a' && codepoint <= 'z') || (codepoint >= 0xC0 && codepoint <= 0xD6) || (codepoint >= 0xD8 && codepoint <= 0xF6) || (codepoint >= 0xF8 && codepoint <= 0x2FF) || (codepoint >= 0x370 && codepoint <= 0x37D) || (codepoint >= 0x37F && codepoint <= 0x1FFF) || (codepoint >= 0x200C && codepoint <= 0x200D) || (codepoint >= 0x2070 && codepoint <= 0x218F) || (codepoint >= 0x2C00 && codepoint <= 0x2FEF) || (codepoint >= 0x3001 && codepoint <= 0xD7FF) || (codepoint >= 0xF900 && codepoint <= 0xFDCF) || (codepoint >= 0xFDF0 && codepoint <= 0xFFFD) || (codepoint >= 0x10000 && codepoint <= 0xEFFFF))
			{
			xmlnamechar.push_back(codepoint);
			xmlnamestartchar.push_back(codepoint);
			}
			
		/*
			XML NameChar
		*/
		if ((codepoint == '-') || (codepoint == '.') || (codepoint >= '0' && codepoint <= '9') || (codepoint == 0xB7) || (codepoint >= 0x0300 && codepoint <= 0x036F) || (codepoint >= 0x203F && codepoint <= 0x2040))
			xmlnamechar.push_back(codepoint);
		}
		
	return range_end;
	}
/*
	PROCESS_PROPLIST()
	------------------
	Format:
		0009..000D    ; White_Space # Cc   [5] <control-0009>..<control-000D>
		0020          ; White_Space # Zs       SPACE
*/
/*!
	@brief Given a line form the PropList.txt file, work out which functions should know about this codepoint
	@param line [in] The line to process.
*/
void process_proplist(const char *line)
{
/*
	Ignore comments
*/
if (*line == '#')
	return;

unsigned int range_start;
unsigned int range_end;

if (sscanf(line, "%x..%x", &range_start, &range_end) == 1)
	range_end = range_start;
	
const char *semicolon;

if ((semicolon = strchr(line, ';')) == nullptr)
	exit(printf("badly formed line:%s\n", line));				// LCOV_EXCL_LINE
	
const char *hash;

if ((hash = strchr(line, '#')) == nullptr)
	exit(printf("badly formed line:%s\n", line));				// LCOV_EXCL_LINE

for (size_t codepoint = (size_t)range_start; codepoint <= (size_t)range_end; codepoint++)
	{
	if (strncmp(semicolon, "; Other_Alphabetic #", hash - semicolon) == 0)				// Other_Alphabetic
		{
		alpha.push_back(codepoint);
		alphanumeric.push_back(codepoint);
		}
		
	if (strncmp(semicolon, "; Other_Lowercase #", hash - semicolon) == 0)				// Other_Lowercase
		{
		alpha.push_back(codepoint);
		alphanumeric.push_back(codepoint);
		lowercase.push_back(codepoint);
		}

	if (strncmp(semicolon, "; Other_Uppercase #", hash - semicolon) == 0)				// Other_Uppercase
		{
		alpha.push_back(codepoint);
		alphanumeric.push_back(codepoint);
		uppercase.push_back(codepoint);
		}
		
	if (strncmp(semicolon, "; Hex_Digit #", hash - semicolon) == 0)						// Hex_Digit
		xdigit.push_back(codepoint);
	if (strncmp(semicolon, "; ASCII_Hex_Digit #", hash - semicolon) == 0)				// ASCII_Hex_Digit
		xdigit.push_back(codepoint);
		
	if (strncmp(semicolon, "; Dash #", hash - semicolon) == 0)								// Dash
		punc.push_back(codepoint);
	if (strncmp(semicolon, "; Terminal_Punctuation #", hash - semicolon) == 0)			// Terminal_Punctuation
		punc.push_back(codepoint);
	}
}

/*
	MAKE_CODEPOINT_ISALNUM()
	------------------------
*/
/*!
	@brief Construct the codepoint_isalnum[] map for determining whether or not a given codepoint is alphanumeric.
*/
void make_codepoint_isalnum(void)
	{
	for (const auto &codepoint : alphanumeric)
		codepoint_isalnum[codepoint] = true;
	}

/*
	FOLDCASE()
	----------
	@brief Casefold a sequence of codepoints (a string).  Note, some casefolded codepoints are longer than one codepoint (e.g. Latin Small Sharp S)
	@param destination [out] the casefolded codepoints.
	@param codepoint [in] The codepoint to casefold.
*/
void foldcase(std::vector<int> &destination, int codepoint)
	{
	if (casefold[codepoint].size() == 0)
		{
		if (codepoint_isalnum[codepoint] || codepoint == 0x20)
			destination.push_back(codepoint);
		}
	else
		for (const auto &point : casefold[codepoint])
			if (codepoint_isalnum[point] ||codepoint == 0x20)
				destination.push_back(point);
	}

/*
	PROCESS_NORMALISATION_RECURSIVELY()
	-----------------------------------
*/
/*!
	@brief Given a codepoint apply the normalisation rules recursively to get an expansion
	@param answer [out] The recursively expanded (but not re-ordered) answer to the expanstion.
	@param head_codepoin [in] The codepoint to compute the expansion for.
*/
void process_normalisation_recursively(std::vector<int> &answer, int head_codepoint)
{
if (JASS_normalisation[head_codepoint].size() == 0)
	foldcase(answer, head_codepoint);			// this happens for HAN normalisations (such as U+2E9F)
else
	for (const auto &codepoint : JASS_normalisation[head_codepoint])
		{
		if (JASS_normalisation[codepoint].size() == 1 && JASS_normalisation[codepoint][0] == head_codepoint)
			foldcase(answer, codepoint);
		else
			process_normalisation_recursively(answer, codepoint);
		}
}

/*
	PROCESS_JASS_NORMALIZATION()
	----------------------------
	Now for normalisation.  In JASS we're interested in a unique normalisation:
		Unicode NFKD normalization
			Normal Form Kompatibility Decomposition
			This seperates ligatures into individual characters (e.g. ff -> f f)
		remove all combining marks, punctuation, and spaces
		case fold everything
	This can be done one a codepoint by codepoint basis - so we simply create a table of the answers.
	NOTE: the worst case is:U+FDFA (see here:http://unicode.org/faq/normalization.html (Question "What are 
			the maximum expansion factors for the different normalization forms?").  It is turned into 18
			codepoints! That is:
				U+FDFA -> U+0635 U+0644 U+0649 U+0020 U+0627 U+0644 U+0644 U+0647 U+0020 U+0639 U+0644 U+064A U+0647 U+0020 U+0648 U+0633 U+0644 U+0645
			According to this program, which is verified against the online calculator here: http://minaret.info/test/normalize.msp
*/
/*!
	@brief Process a single line of UnicodeData.txt and extract the normaliation of that codepoint.
	@param line [in] a single line from UnicodeData.txt.
*/
void process_JASS_normalization(const char *line)
{
/*
	get the codepoint for this line
*/
unsigned int codepoint;
sscanf(line, "%x", &codepoint);

/*
	Look for field 5 in UnicodeData.txt
*/
const char *semicolon = line;
for (uint32_t which_semicolon = 0; which_semicolon < 5; which_semicolon++)
	if ((semicolon = strchr(semicolon + 1, ';')) == nullptr)
		return;			// LCOV_EXCL_LINE

/*
	Get the normalisation rules
*/
uint32_t normalisation = codepoint;
const char *digit = semicolon + 1;
while ((digit = strpbrk(digit, ";0123456789")) != nullptr)
	{
	if (*digit == ';')
		{
		if (normalisation == (uint32_t)codepoint)
			JASS_normalisation[codepoint].push_back(normalisation);
		break;
		}

	sscanf(digit, "%x", &normalisation);
	JASS_normalisation[codepoint].push_back(normalisation);
	digit = strpbrk(digit, " ;");
	}
}

/*
	NORMALIZE()
	-----------
*/
/*!
	@Brief Compute the normalisation for the entire Unicode database.
*/
void normalize(void)
	{
	std::map<int, std::vector<int>> table_of_normalisations;
	
	/*
		Apply the normalisation rules recursively and write out the translation
	*/
	for (int codepoint = 0; codepoint <= 0x10FFFF; codepoint++)
		{
		std::vector<int> answer;
		
		process_normalisation_recursively(answer, codepoint);

		/*
			remove all preceeding and ending spaces
		*/
		while (answer.size() >= 1 && answer[0] == 0x20)
			answer.erase(answer.begin());
		while (answer.size() >= 1 && answer[answer.size() - 1] == 0x20)			// LCOV_EXCL_LINE
			answer.pop_back();													// LCOV_EXCL_LINE

		/*
			Only write it out if it isn't an empty string (as most are empty and we only need to store that once).
		*/
		if (answer.size() != 0)
			{
			printf("static const uint32_t JASS_normalisation_%x[] = {", codepoint);

			for (const auto &cp : answer)
				printf("0x%X, ", cp);
			puts("0x00};");
			}
			
		table_of_normalisations[codepoint] = answer;
		}
	/*
		Write out the empty string
	*/
	printf("static const uint32_t JASS_normalisation_0[] = {0};\n");

	/*
		Write out the global lookup table
	*/
	printf("const uint32_t *JASS_normalisation[] = {\n");
	for (int codepoint = 0; codepoint <= 0x10FFFF; codepoint++)
		if (table_of_normalisations[codepoint].size() == 0)
			printf("JASS_normalisation_0,\n");
		else
			printf("JASS_normalisation_%x,\n", codepoint);
	puts("};");
	}

/*
	PROCESS_CASEFOLDING()
	---------------------
*/
/*!
	@brief process a single line of CaseFolding.txt and extract the full case folding data (that is, the "C+F" subset)
	@param line [in] A single line from CaseFolding.txt
	@details The JASS normalisation process is: Unicode NFKD normalization, remove all non-alphanumerics, then case fold.

*/
void process_casefolding(const char *line)
	{
	if (*line == '#')
		return;
		
	/*
		get the codepoint
	*/
	unsigned int codepoint;
	char type;
	sscanf(line, "%X; %c;", &codepoint, &type);
	if (type != 'C' && type != 'F')
		return;									// we only want 'C' = "common case folding" and 'F' = "full case folding"
	
	/*
		extract the sequence it becomes
	*/
	const char *semicolon = line;
	semicolon = strchr(semicolon + 1, ';');
	semicolon = strchr(semicolon + 1, ';');
	const char *digit = semicolon + 1;
	std::vector<int> folded;						// the case folded translation
	while ((digit = strpbrk(digit, ";0123456789")) != nullptr)
		{
		if (*digit == ';')
			break;

		unsigned int fold;
		sscanf(digit, "%x", &fold);
		folded.push_back(fold);
		digit = strpbrk(digit, " ;");
		}
	/*
		Now add it to the translation table
	*/
	casefold[codepoint] = folded;
	}

/*
	MAIN_EVENT()
	------------
	@brief Read the Unicode database files, compute the set of "C" routines and then dump them out
*/
int main_event(int argc, char *argv[])
	{
	/*
		Check we have the right number of parameters
	*/
	if (argc != 4)
		usage(argv[0]);						// LCOV_EXCL_LINE
		
	/*
		get the name of the UnicodeData.txt file
	*/
	char *filename = argv[1];

	/*
		read the file
	*/
	std::string file;		// the UnicodeData.txt file
	JASS::file::read_entire_file(filename, file);
	
	/*
		turn into an array of lines
	*/
	std::vector<uint8_t *>lines;
	JASS::file::buffer_to_list(lines, file);

	/*
		Process each line.
	*/
	unsigned codepoint = 0;
	for (const auto &line : lines)
		codepoint = process_unicodedata((const char *)line, codepoint);
		
	/*
		get the case folding file
	*/
	filename = argv[3];
	std::string casefold_file;
	std::vector<uint8_t *>casefold_lines;
	JASS::file::read_entire_file(filename, casefold_file);
	JASS::file::buffer_to_list(casefold_lines, casefold_file);
	for (const auto &line : casefold_lines)
		process_casefolding((const char *)line);
		
	/*
		get the name of the Properties file
		read the file and turn it into a bunch of lines
		the process each line
	*/
	filename = argv[2];
	std::string proplist_file;
	std::vector<uint8_t *>proplist_lines;
	JASS::file::read_entire_file(filename, proplist_file);
	JASS::file::buffer_to_list(proplist_lines, proplist_file);
	for (const auto &line : proplist_lines)
		process_proplist((const char *)line);

	/*
		Dump out each method
	*/
	serialise("isalpha", alpha);
	serialise("isalnum", alphanumeric);
	serialise("isupper", uppercase);
	serialise("islower", lowercase);
	serialise("isdigit", digit);
	serialise("ispunct", punc);
	serialise("isuspace", space);
	serialise("isspace", whitespace);
	serialise("ismark", mark);
	serialise("issymbol", symbol);
	serialise("iscntrl", control);
	serialise("isgraph", graphical);
	serialise("isxdigit", xdigit);
	/*
		Including the XML methods
	*/
	serialise("isxmlnamechar", xmlnamechar);
	serialise("isxmlnamestartchar", xmlnamestartchar);

	/*
		Now for case folded normalisation.
	*/
	make_codepoint_isalnum();
	for (const auto &line : lines)
		process_JASS_normalization((const char *)line);

	normalize();
	/*
		success
	*/
	return 0;
	}

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	try
		{
		return main_event(argc, argv);
		}
	catch (...)
		{
		puts("Unexpected exception");
		return 1;
		}
	}
