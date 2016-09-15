/*
	UNITTEST.CPP
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "file.h"
#include "ascii.h"
#include "maths.h"
#include "unicode.h"
#include "bitstring.h"
#include "allocator.h"
#include "allocator_cpp.h"
#include "instream_file.h"
#include "instream_document_trec.h"

/*
	MAIN()
	------
*/
int main(void)
	{
	/*
		Test the JASS classes
	*/
	JASS::file::unittest();
	JASS::bitstring::unittest();
	JASS::unicode::unittest();
	JASS::ascii::unittest();
	JASS::maths::unittest();
	JASS::allocator::unittest();
	JASS::allocator_cpp<size_t>::unittest();
	JASS::instream_file::unittest();
	JASS::instream_document_trec::unittest();
	
	return 0;
	}
