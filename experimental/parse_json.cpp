/*
	PARSE_JSON.CPP
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	experimental program to parse WashingtonPostV2 JSON file from TREC
*/
#include "file.h"

class tokenizer
	{
	public:
		class token
			{
			public:
				const char *start;
				size_t length;
			};

	private:
		const char *document;
		const char *current;
		token lexical;

	public:
		tokenizer(const char *document) : document(document), current(document)
			{
			}

		token *next()
			{
			if (*current == '\0')
				return NULL;
			else if (*current == '"')
				{
				lexical.start = current + 1;
				do
					current++;
				while (*current != '"' && *current != '\0');
				current++;

				lexical.length = current - lexical.start - 2;		// remove the quotes
				}
			else
				{
				lexical.start = current;
				current++;

				lexical.length = 1;
				}
			return &lexical;
			}

		token *first()
			{
			return next();
			}

	};

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	char *filename = (char *)"/Users/andrew/programming/data/WashingtonPostv2/first_document.jl";

	if (argc == 2)
		filename = argv[1];

	std::string file_buffer;

	JASS::file::read_entire_file(filename, file_buffer);

	tokenizer lexer(file_buffer.c_str());
	for (tokenizer::token *token = lexer.first(); token != NULL; token = lexer.next())
		{
		printf("%*.*s\n", (int)token->length, (int)token->length, token->start);
		}

	return 0;
	}
