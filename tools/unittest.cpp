/*
	UNITTEST.CPP
	------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "file.h"
#include "ascii.h"
#include "maths.h"
#include "slice.h"
#include "parser.h"
#include "unicode.h"
#include "bitstring.h"
#include "binary_tree.h"
#include "allocator_cpp.h"
#include "instream_file.h"
#include "allocator_pool.h"
#include "instream_memory.h"
#include "allocator_memory.h"
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
	puts("PERFORM ALL UNIT TESTS");

	puts("file");
	JASS::file::unittest();
	
	puts("bitstring");
	JASS::bitstring::unittest();

	puts("unicode");
	JASS::unicode::unittest();
	
	puts("ascii");
	JASS::ascii::unittest();
	
	puts("maths");
	JASS::maths::unittest();
	
	// JASS::allocator does not have a unittest because it is a virtual base class
	
	puts("allocator_pool");
	JASS::allocator_pool::unittest();
	
	puts("allocator_memory");
	JASS::allocator_memory::unittest();

	puts("document");
	JASS::document::unittest();
	
	puts("allocator_cpp");
	JASS::allocator_cpp<size_t>::unittest();
	
	puts("slice");
	JASS::slice::unittest();
	
	// instream does not have a unittest because it is a virtual base class
	
	puts("instream_file");
	JASS::instream_file::unittest();
	
	puts("instream_memory");
	JASS::instream_memory::unittest();
	
	puts("instream_document_trec");
	JASS::instream_document_trec::unittest();
	
	puts("parser");
	JASS::parser::unittest();
	
	puts("binary_tree");
	JASS::binary_tree<size_t, size_t>::unittest();
	
	puts("ALL UNIT TESTS HAVE PASSED");
	return 0;
	}
