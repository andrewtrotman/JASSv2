/*
	UNICODEDATA_TO_UTYPE.C
	----------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Convert the Unicode Standard UnicodeData.txt and PropList.txt into a bunch of methods similar to the
	"C" standard library ctype routines.
	
	The latest version of UnicodeData.txt can be found here: http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
	The latest version of PropList.txt can be found here: http://www.unicode.org/Public/UCD/latest/ucd/PropList.txt
	The documentation on how to read them is here: http://www.unicode.org/reports/tr44/#UnicodeData.txt

	The functions we wish to mirror are described here: http://www.cplusplus.com/reference/cctype/
	
	They are:
		NAME           VARIABLE IN THIS PROGRAM		D'S DEFINITION
		isalnum			++ alpha + digit					-
		isalpha			++ alpha								"general Unicode category: Alphabetic" - isAlpha()
		isblank
		iscntrl			++ control							"general Unicode category: Cc" - isControl()
		isdigit			++ digit								"general Unicode category: Nd, Nl, No" - isNumber()
		isgraph			++ graphical						"general Unicode category: L, M, N, P, S, Zs" - isGraphical()
		islower			++ lowercase						"Unicode lowercase" - isLower()
		isprint
		ispunct			++ punc								"general Unicode category: Pd, Ps, Pe, Pc, Po, Pi, Pf" - IsPunctuation()
		-					++ space								"general Unicode category: Zs" - isSpace()
		isupper			++ uppercase						"Unicode uppercase" - isUpper()
		isxdigit			++ xdigit
		isspace			++ white								"Part of C0(tab, vertical tab, form feed, carriage return, and linefeed characters), Zs, Zl, Zp, and NEL(U+0085)" - isWhite()

		tolower
		toupper
		
	These are then dumped out as C++ methods.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>

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
std::vector<uint32_t>alpha;				// list of alphabetical characters
std::vector<uint32_t>uppercase;			// list of uppercase characters
std::vector<uint32_t>lowercase;			// list of lowercase characgers

/*
	Digits, or more accurately number characters including digit characters
*/
std::vector<uint32_t>digit;

/*
	Punctuaton
*/
std::vector<uint32_t>punc;

/*
	Space characters (by the unicode definition of space)
*/
std::vector<uint32_t>space;

/*
	White Space characters (by the 'D' Phobos definition of whitespace) which is:
	"general Unicode category: Part of C0(tab, vertical tab, form feed, carriage return, and linefeed characters), Zs, Zl, Zp, and NEL(U+0085))"
*/
std::vector<uint32_t>whitespace;

/*
	Diacritic marks
*/
std::vector<uint32_t>mark;

/*
	Symbols
*/
std::vector<uint32_t>symbol;

/*
	Control characters
*/
std::vector<uint32_t>control;

/*
	Graphical characters
*/
std::vector<uint32_t>graphical;

/*
	Hexadecimal digits
*/
std::vector<uint32_t>xdigit;

/*
	USAGE()
	------
*/
/*!
	@brief Tell the user how to use this program.
	@param filename [in] the name of this executable (normally argv[0]).
*/
void usage(char *filename)
	{
	printf("Usage:%s <UnicodeData.txt> <PropList.txt>\n", filename);
	exit(0);
	}

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
void serialise(const std::string &operation, const std::vector<uint32_t> &list)
	{
	JASS::bitstring bits;
	
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
	printf("static unsigned char unicode_%s_data[] = {", operation.c_str());
	
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
	}

/*
	PROCESS()
	---------
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
int process(const char *line, int last_codepoint)
	{
	int codepoint;			// unicode number of the codepoint
	int range_start;		// start of the range for this line in the Unicode database
	int range_end;			// end of the range for this line in the Unicode database
	char category[3];		// Two letter catagory code for the codepoint
	
	/*
		the first parmeter is the codepoint
	*/
	if (strstr(line, ", Last>") != NULL)
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
	char *semicolon;
	if ((semicolon = strchr(line, ';')) == NULL)
		exit(printf("Badly formed line:%s\n", line));
	if ((semicolon = strchr(semicolon + 1, ';')) == NULL)
		exit(printf("Badly formed line:%s\n", line));
		
	/*
		The third parameter is the type
	*/
	strncpy(&category[0], semicolon + 1, 2);
	category[2] = '\0';
	
	/*
		Work out what type of character we have, according the the "C" ctype.h conventions
		
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
			}
		if (strcmp(category, "Ll") == 0)				// a lowercase letter
			{
			lowercase.push_back(codepoint);
			alpha.push_back(codepoint);
			}
		if (strcmp(category, "Lt") == 0)				// a digraphic character, with first part uppercase
			{
			uppercase.push_back(codepoint);
			alpha.push_back(codepoint);
			}
		if (strcmp(category, "Lm") == 0)				// a modifier letter
			alpha.push_back(codepoint);
		if (strcmp(category, "Lo") == 0)				// other letters, including sylables and ideographs
			alpha.push_back(codepoint);
		if (strcmp(category, "Nl") == 0)				// a letterlike numeric character
			alpha.push_back(codepoint);

		/*
			Numeric characters
		*/
		if (strcmp(category, "Nd") == 0)				// Decimal_Number
			digit.push_back(codepoint);
		if (strcmp(category, "Nl") == 0)				// Letter_Number
			digit.push_back(codepoint);
		if (strcmp(category, "No") == 0)				// Other_Number
			digit.push_back(codepoint);

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
		if (codepoint <= 0xFF && isspace(codepoint))		// space, tab, vertical tab, form feed, carriage return, and linefeed characters
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

int range_start;
int range_end;

if (sscanf(line, "%x..%x", &range_start, &range_end) == 1)
	range_end = range_start;
	
char *semicolon;

if ((semicolon = strchr(line, ';')) == NULL)
	exit(printf("badly formed line:%s\n", line));
	
char *hash;

if ((hash = strchr(line, '#')) == NULL)
	exit(printf("badly formed line:%s\n", line));

for (int codepoint = range_start; codepoint <= range_end; codepoint++)
	{
	if (strncmp(semicolon, "; Other_Alphabetic #", hash - semicolon) == 0)				// Other_Alphabetic
		alpha.push_back(codepoint);
		
	if (strncmp(semicolon, "; Other_Lowercase #", hash - semicolon) == 0)				// Other_Lowercase
		{
		alpha.push_back(codepoint);
		lowercase.push_back(codepoint);
		}

	if (strncmp(semicolon, "; Other_Uppercase #", hash - semicolon) == 0)				// Other_Uppercase
		{
		alpha.push_back(codepoint);
		uppercase.push_back(codepoint);
		}
		
	if (strncmp(semicolon, "; Hex_Digit #", hash - semicolon) == 0)						// Hex_Digit
		xdigit.push_back(codepoint);
	if (strncmp(semicolon, "; ASCII_Hex_Digit #", hash - semicolon) == 0)				// ASCII_Hex_Digit
		xdigit.push_back(codepoint);
		
/*
	D doesn't use this definition for punctuation, but it isn't yet clear that it
	adds anything that isn't already there.
*/
/*
	if (strncmp(semicolon, "; Dash #", hash - semicolon) == 0)								// Dash
					punc.push_back(codepoint);
	if (strncmp(semicolon, "; Terminal_Punctuation #", hash - semicolon) == 0)			// Terminal_Punctuation
					punc.push_back(codepoint);
*/
	}
}

/*
	MAIN()
	------
	read UnicodeData.txt, compute the set of is() routines and dump then out
*/
int main(int argc, char *argv[])
	{
	/*
		Check we have the right number of parameters
	*/
	if (argc != 3)
		usage(argv[0]);
		
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
	int codepoint = 0;
	for (const auto &line : lines)
		codepoint = process((const char *)line, codepoint);
		
	/*
		get the name of the Properties file
	*/
	filename = argv[2];
	
	/*
		read the file and turn it into a bunch of lines
		the process each line
	*/
	JASS::file::read_entire_file(filename, file);
	JASS::file::buffer_to_list(lines, file);
	for (const auto &line : lines)
		process_proplist((const char *)line);

	/*
		Dump out each method
	*/
	serialise("isalpha", alpha);
	serialise("isupper", uppercase);
	serialise("islower", lowercase);
	serialise("isdigit", digit);
	serialise("ispunc", punc);
	serialise("isspace", space);
	serialise("iswhitespace", whitespace);
	serialise("ismark", mark);
	serialise("issymbol", symbol);
	serialise("iscontrol", control);
	serialise("isgraph", graphical);
	serialise("isxdigit", xdigit);

	/*
		success
	*/
	return 0;
	}