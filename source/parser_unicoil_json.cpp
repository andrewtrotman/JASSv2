/*
	INSTREAM_DOCUMENT_UNICOIL_JSON.CPP
	----------------------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/

#include "parser_unicoil_json.h"

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_UNICOIL_JSON::UNITTEST()
		------------------------------------------
	*/
	void parser_unicoil_json::unittest(void)
		{
		parser_unicoil_json tokenizer;
		/*
			An example document
		*/
		std::string text = "\"the\": 94, \"another\": 101";

		/*
			create a document object and give it a the document.
		*/
		class document example;
		example.contents = slice((void *)text.c_str(), text.size());

		/*
			Give the document to the tokenizer
		*/
		tokenizer.set_document(example);

		/*
			Get as many tokens as it'll return until we reach eof, checking each one for correctness
		*/
		size_t count = 0;
		parser::token::token_type type;
		do
			{
			const auto &token = tokenizer.get_next_token();

			if (token.type != token::eof)
				{
				std::cout << token.get() << ' ' <<  token.count << '\n';
				}
			count++;
			type = token.type;
			}
		while (type != token::eof);

		/*
			make sure we got the right number of tokens
		*/
		JASS_assert(count == 2);

		/*
			Success
		*/
		puts("parser_unicoil_json::PASSED");
		}
	}
