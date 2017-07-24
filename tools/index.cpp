/*
	INDEX.CPP
	---------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	Originally from the ATIRE codebase (where it was also written by Andrew Trotman)
*/
/*!
	@file
	@brief The JASS indexer
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#include <string.h>

#include "parser.h"
#include "instream_file.h"
#include "instream_memory.h"
#include "instream_document_trec.h"
#include "index_manager_sequential.h"

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	JASS::parser parser;
	JASS::document document;
	JASS::instream_file file(argv[1]);
	JASS::instream_document_trec source(file);
	JASS::index_manager_sequential index;
	
	size_t total_documents = 0;
	
	do
		{
		document.rewind();
		source.read(document);
		if (document.isempty())
			break;
		total_documents++;
		if (total_documents % 10000 == 0)
			printf("Documents:%lld\n", (long long)total_documents);
		parser.set_document(document);
		
		bool finished = false;
		do
			{
			const auto &token = parser.get_next_token();
			
			switch (token.type)
				{
				case JASS::parser::token::eof:
					finished = true;
					break;
				case JASS::parser::token::alpha:
				case JASS::parser::token::numeric:
					index.term(token);
					break;
				case JASS::parser::token::xml_start_tag:
					index.begin_document(document.primary_key);
					break;
				case JASS::parser::token::xml_end_tag:
					index.end_document();
					break;
				default:
					break;
				}
			}
		while (!finished);
		}
	while (!document.isempty());
	
	printf("Documents:%lld\n", (long long)total_documents);

	return 0;
	}

