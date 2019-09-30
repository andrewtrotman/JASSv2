/*
	PARSER_FASTA.CPP
	----------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include <algorithm>

#include "ascii.h"
#include "maths.h"
#include "assert.h"
#include "unicode.h"
#include "document.h"
#include "parser_fasta.h"

namespace JASS
	{
	/*
		PARSER_FASTA::GET_NEXT_TOKEN()
		------------------------------
	*/
	const class parser::token &parser_fasta::get_next_token(void)
		{
		/*
			Make sure we don't read past end of document
		*/
		if (current >= end_of_document)
			return eof_token;

		/*
			Skip over any leading spaces
		*/
		while (ascii::isspace(*current))
			current++;
		if (current >= end_of_document)
			return eof_token;

		/*
			Find the next k-mre
		*/
		uint8_t *buffer_pos = current_token.buffer;
		current_token.type = current_token.alpha;
		uint8_t *from = current++;
		size_t byte = 0;
		while (from < end_of_document)
			{
			if (!ascii::isspace(*from))
				{
				*buffer_pos++ = ascii::tolower(*from++);
				if (++byte >= kmer_length)
					break;
				}
			else
				from++;

			}

		/*
			Do we have enough characters to for a k-mer (or are we at EOF)?
		*/
		if (current >= end_of_document || byte != kmer_length)
			return eof_token;

		/*
			Return the k-mer as a token.
		*/
		current_token.lexeme = slice((void *)current_token.buffer, (void *)buffer_pos);
		return current_token;
		}

	/*
		PARSER_FASTA::UNITTEST()
		------------------------
	*/
	void parser_fasta::unittest(void)
		{
		parser_fasta tokenizer(5);								// declare a tokenizer

		/*
			Test a set of Unicode and ASCII tokens all intermixed to make sure we get the right answer
		*/
		static const std::string text = ">NR_118889.1\nG GTC\nTNA TA";

		std::string text_answer[] =
			{
			"ggtct",
			"gtctn",
			"tctna",
			"ctnat",
			"tnata",
			};
		(void)text_answer;				// Silence the  "Unused variable" message when in Release

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
				JASS_assert(std::string((char *)token.lexeme.address(), token.lexeme.size()) == text_answer[count]);
			count++;
			type = token.type;
			}
		while (type != token::eof);

		/*
			make sure we got the right number of tokens
		*/
		JASS_assert(count == 6);


		/*
			Yay, we passed
		*/
		puts("parser_fasta::PASSED");
		}
	}
