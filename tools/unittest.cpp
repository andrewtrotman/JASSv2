/*
	UNITTEST.CPP
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "file.h"
#include "bitstring.h"
#include "unicode.h"

int					// [out] returns 0 to the OS on success, else it will assert
main
	(
	void				// no parameters
	)
	{
	/*
		Test the JASS file I/O methods
	*/
	JASS::file::unittest();
	JASS::bitstring::unittest();
	JASS::unicode::unittest();
	return 0;
	}
