/*
	PARSER.CPP
	----------
*/
#include <string.h>

#include "ascii.h"
#include "parser.h"
#include "unicode.h"
#include "document.h"
#include "unittest_data.h"

namespace JASS
	{
	
	void parser::build_unicode_alphabetic_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end)
		{
		do
			{
			for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
				buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow

			if ((current += bytes) >= end_of_document)
				break;
			codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
			}
		while (unicode::isalpha(codepoint));
		}
		
		
	void parser::build_unicode_numeric_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end)
		{
		do
			{
			for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
				buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow

			if ((current += bytes) >= end_of_document)
				break;
			codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
			}
		while (unicode::isdigit(codepoint));
		}

	
	/*
		PARSER::GET_NEXT_TOKEN()
		------------------------
	*/
	const class parser::token &parser::get_next_token(void)
		{
		size_t bytes;
		uint32_t codepoint = 0;

		/*
			Make sure we don't read past end of document
		*/
		if (current >= end_of_document)
			return eof_token;
			
		/*
			We skip over non-indexable characters before the start of the first token
			Start with the assumption that we're skipping over ASCII white space
		*/
		while (ascii::isspace(*current))
			if (++current >= end_of_document)
				return eof_token;

		/*
			If we've hit a non-white space then either its an ASCII non-white space,
			else we have to skip over Unicode white space
		*/
		if (ascii::isascii(*current))
			bytes = 1;
		else
			{
			/*
				Skipping over Unicode whitespace
			*/
			codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
			while (unicode::isspace(codepoint))
				{
				if ((current += bytes) >= end_of_document)
					return eof_token;
				codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
				}
			}

		/*
			Process based on the character type (ASCII or Unicode).
			Note that since this parser manages Unicode, both alpha and
			numeric tokens must be copied into the internal buffer as
			Unicode normalisation can result in one codepoint becoming
			more than one ASCII digit.
		*/
		uint8_t *buffer_pos = token.buffer;
		uint8_t *buffer_end = token.buffer + sizeof(token.buffer);

		/*
			ASCII Alphabetics
		*/
		if (ascii::isalpha(*current))
			{
			token.type = token.alpha;
			/*
				As the first character is ASCII then assume the remainder will be.
			*/
			*buffer_pos++ = ascii::tolower(*current);
			current++;
		
			while (ascii::isalpha(*current))
				{
				if (buffer_pos < buffer_end)
					*buffer_pos++ = ascii::tolower(*current);				// won't write on overflow
				if (++current >= end_of_document)
					break;															// at end of input so we must be at end of token
				}

			/*
				We might have terminated when we found a non-ASCII alphabetic so keep processing as Unicode
			*/
			if (current < end_of_document && !ascii::isascii(*current))
				{
				uint32_t codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
				build_unicode_alphabetic_token(codepoint, bytes, buffer_pos, buffer_end);
				}
			}
		/*
			ASCII Numerics
		*/
		else if (ascii::isdigit(*current))
			{
			token.type = token.numeric;
			/*
				As the first character is ASCII then assume the remainder will be.
			*/
			*buffer_pos++ = *current;
			current++;
		
			while (ascii::isdigit(*current))
				{
				if (buffer_pos < buffer_end)
					*buffer_pos++ = *current;				// won't write on overflow
				if (++current >= end_of_document)
					break;										// at end of input so we must be at end of token
				}
	
			/*
				We might have terminated when we found a non-ASCII numeric so keep processing as Unicode
			*/
			if (current < end_of_document && !ascii::isascii(*current))
				{
				uint32_t codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
				build_unicode_numeric_token(codepoint, bytes, buffer_pos, buffer_end);
				}
			}
		/*
			ASCII Others (Punctuation / tags, etc)
		*/
		else if (ascii::isascii(*current))
			{
			if (*current != '<')
				{
				/*
					Just plain old non alphanumerics.  Returned one character at a time
				*/
				token.type = token.other;
				*buffer_pos++ = *current;
				current++;
				}
			else
				{
				/*
					FIX THIS..
				*/
				token.type = token.other;
				*buffer_pos++ = *current;
				current++;
				/*
					XML or HTML markup
					Concepts to deal with here include:
						XML open tags <xx>
						XML close tags </xx>
						XML processing instructons <? xxx ?>
						HTML processing instructions <? xxx >
						XML comment <!-- xxx -->
						HTML comment <!-- xxx >
						Other XML stuff <![CDATA[<greeting>Hello, world!</greeting>]]>
				*/
				}
			}
		else
			{
			/*
				Unicode Alphabetic (codepoint already contains the current character and bytes is the UTF-8 length in bytes).
			*/
			if (unicode::isalpha(codepoint))
				{
				token.type = token.alpha;
				build_unicode_alphabetic_token(codepoint, bytes, buffer_pos, buffer_end);
				}
			/*
				Unicode Numeric
			*/
			else if (unicode::isdigit(codepoint))
				{
				token.type = token.numeric;
				build_unicode_numeric_token(codepoint, bytes, buffer_pos, buffer_end);
				}
			/*
				Unicode Others (Punctuation / tags, etc)
			*/
			else
				{
				/*
					Return a single characters being of type "other"
				*/
				token.type = token.other;
				current += bytes;
				
				for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
					buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);
				}
			}

		token.token = slice((void *)token.buffer, (void *)buffer_pos);
		return token;
		}
		
	/*
		PARSER::UNITTEST()
		------------------
	*/
	void parser::unittest(void)
		{
		parser tokenizer;								// declare a tokenizer

		/*
			Test a set of Unicode and ASCII tokens all intermixed to make sure we get the right answer
		*/
		std::string text = "abc123 αβγ①②③ aβc1②3 αbγ①2③ ab℃½3         αβγ½③";
		
		/*
			The correct answer
		*/
		std::string text_answer[] =
			{
			"abc",
			"123",
			"αβγ",
			"123",
			"aβc",
			"123",
			"αbγ",
			"123",
			"abc",
			"123",
			"αβγ",
			"123"
			};
		
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
				assert(std::string((char *)token.token.address(), token.token.size()) == text_answer[count]);
				}
			count++;
			type = token.type;
			}
		while (type != token::eof);
		
		/*
			make sure we got the right number of tokens
		*/
		assert(count == 13);
		
		
		/*
			Now check a document for the unittests
		*/
		example.contents = slice((void *)unittest_data::ten_document_1.c_str(), unittest_data::ten_document_1.size());
		tokenizer.set_document(example);
		count = 0;
		do
			{
			const auto &token = tokenizer.get_next_token();
			if (token.type != token::eof)
				printf("%*.*s\n", (int)token.token.size(), (int)token.token.size(), token.token.address());
			count++;
			type = token.type;
			}
		while (type != token::eof);
		assert(count == 13);
		
		/*
			Yay, we passed
		*/
		puts("parser::PASSED");
		}
	}
