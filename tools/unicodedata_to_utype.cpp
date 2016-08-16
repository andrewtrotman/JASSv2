/*
	UNICODEDATA_TO_UTYPE.C
	----------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Convert the Unicode Standard UnicodeData.txt into a bunch of methods similar to the
	"C" standard library ctype routines.
	
	The latest version of UnicodeData.txt can be found here: http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt
	The documentation on how to read it is here: http://www.unicode.org/reports/tr44/#UnicodeData.txt
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <vector>

#include "file.h"


/*
	alphabetic characters.  Note that alpha != lowercase + uppercase because there are many
	characters that are caseless (such as the phonetic characters).
*/
std::vector<uint32_t>alpha;				// list of alphabetical characters
std::vector<uint32_t>uppercase;			// list of uppercase characters
std::vector<uint32_t>lowercase;			// list of lowercase characgers


/*
	USAGE()
	------
*/
void							// no return value
usage
	(
	char *filename			// [in] the name of this executable
	)
	{
	printf("Usage:%s <UnicodeData.txt>\n", filename);
	exit(0);
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
*/
void							// no return value
process
	(
	const char *line	// [in] the line to process
	)
	{
	uint32_t codepoint;					// unicode number of the codepoint
	char name[1024];				// Name of the codepoint
	char category[5];				// Two letter catagory code for the codepoint
	
	sscanf(line, "%x;%s;%s;", &codepoint, name, category);
	
	if (*category == 'L')
		alpha.push_back(codepoint);
		
	if (strcmp(category, "Lu") == 0)						// an uppercase letter
		uppercase.push_back(codepoint);
	else if (strcmp(category, "Ll") == 0)				// a lowercase letter
		lowercase.push_back(codepoint);
	else if (strcmp(category, "Lt") == 0)				// a digraphic character, with first part uppercase
		uppercase.push_back(codepoint);

	/*
		KEEP GOING HERE
		then write out the vectors as bitstrings.
	*/
		
	puts((char *)line);
	}

/*
	MAIN()
	------
*/
int						// [out] return code to the OS, always 0
main
	(
	int argc,			// [in] number of command line parameters
	char *argv[]		// [in] array of pointers to command line parameters
	)
	{
	/*
		Check we have the right number of parameters
	*/
	if (argc != 2)
		usage(argv[0]);
		
	/*
		get the name of the file to parse
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
	for (const auto &line : lines)
		{
		process((const char *)line);
		}
		
	return 0;
	}