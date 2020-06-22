/*
	PARSER_USE.CPP
	--------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include "parser.h"
#include "instream_file.h"
#include "instream_document_trec.h"

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	/*
		allocate a document object and a parser object.
	*/
	JASS::document document;
	JASS::parser parser;
	
	/*
		build a pipeline - recall that deletes cascade so file is deleted when source goes out of scope.
	*/
	std::string filename;
	try
		{
		filename = argv[1];
		}
	catch (...)
		{
		exit(printf("Cannot parse filename\n"));
		}
	std::shared_ptr<JASS::instream> file(new JASS::instream_file(filename));
	JASS::instream_document_trec source(file);

	/*
		this program counts document and alphbetic tokens in those documents.
	*/
	size_t total_documents = 0;
	size_t alphas = 0;

	/*
		read document, then parse them.
	*/
	do
		{
		/*
			read the next document into the same memory the last document used.
		*/
		document.rewind();
		source.read(document);

		/*
			eof is signaled as an empty document.
		*/
		if (document.isempty())
			break;

		/*
			count documents.
		*/
		total_documents++;

		/*
			now parse the docment.
		*/
		parser.set_document(document);
		bool finished = false;
		do
			{
			/*
				get the next token
			*/
			const auto &token = parser.get_next_token();
			
			/*
				what type is that token
			*/
			switch (token.type)
				{
				case JASS::parser::token::eof:
					/*
						At end of document so signal to leave the loop.
					*/
					finished = true;
					break;
				case JASS::parser::token::alpha:
					/*
						Count the number of alphabetic tokens.
					*/
					alphas++;
					break;
				default:
					/*
						else ignore the token.
					*/
					break;
				}
			}
		while (!finished);
		}
	while (!document.isempty());
	
	/*
		Dump out the the number of documents and the numner of tokens.
	*/
	printf("Documents:%lld\n", (long long)total_documents);
	printf("alphas   :%lld\n", (long long)alphas);

	return 0;
	}
