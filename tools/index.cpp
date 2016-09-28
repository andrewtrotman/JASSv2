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

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	JASS::parser parser;
	JASS::document document;
	JASS::instream_file *file = new JASS::instream_file(argv[1]);
	JASS::instream_document_trec source(*file);

	size_t total_documents = 0;
	size_t start_doc_tags = 0;
	size_t end_doc_tags = 0;
	size_t alphas = 0;
	size_t numerics = 0;
	
	do
		{
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
					alphas++;
					break;
				case JASS::parser::token::numeric:
					numerics++;
					break;
				case JASS::parser::token::xml_start_tag:
					if (token.token.size() == 3 && strncmp((char *)token.token.address(), "DOC", 3) == 0)
						start_doc_tags++;
					break;
				case JASS::parser::token::xml_end_tag:
					if (token.token.size() == 3 && strncmp((char *)token.token.address(), "DOC", 3) == 0)
						end_doc_tags++;
					break;
				default:
					break;
				}
			}
		while (!finished);
		}
	while (!document.isempty());
	
	printf("Documents:%lld\n", (long long)total_documents);
	printf("<DOC>    :%lld\n", (long long)start_doc_tags);
	printf("</DOC>   :%lld\n", (long long)end_doc_tags);
	printf("alphas   :%lld\n", (long long)alphas);
	printf("numerics :%lld\n", (long long)numerics);

	return 0;
	}
