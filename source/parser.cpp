/*
	PARSER.CPP
	----------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include <algorithm>

#include "ascii.h"
#include "maths.h"
#include "parser.h"
#include "assert.h"
#include "unicode.h"
#include "document.h"
#include "unittest_data.h"

namespace JASS
	{
	/*
		PARSER::BUILD_UNICODE_ALPHABETIC_TOKEN()
		----------------------------------------
	*/
	void parser::build_unicode_alphabetic_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end)
		{
		/*
			while we have alphabetics, case fold into the token buffer.
		*/
		while (unicode::isalpha(codepoint))
			{
			/*
				Case fold.
			*/
			for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
				buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow

			/*
				Get the next codepoint
			*/
			if ((current += bytes) >= end_of_document || bytes == 0)
				break;
			codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
			}
		}
		
	/*
		PARSER::BUILD_UNICODE_NUMERIC_TOKEN()
		-------------------------------------
	*/
	void parser::build_unicode_numeric_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end)
		{
		/*
			while we have numerics, case fold into the token buffer.
		*/
		while (unicode::isdigit(codepoint))
			{
			/*
				Case fold.  Yes, its necessary to casefold numerics.  Some Unicode codepoints turn into more than
				one digit when normalised.  For example, the sumbol for a half (1/2) becomes a 1 and a 2.
			*/
			for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
				buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow

			/*
				Get the next codepoint
			*/
			if ((current += bytes) >= end_of_document || bytes == 0)
				break;
			codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
			}
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
		uint8_t *buffer_pos = current_token.buffer;
		uint8_t *buffer_end = current_token.buffer + sizeof(current_token.buffer);

		/*
			ASCII Alphabetics
		*/
		if (ascii::isalpha(*current))
			{
			current_token.type = current_token.alpha;
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
			current_token.type = current_token.numeric;
			
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
				*buffer_pos++ = *current++;
				current_token.type = current_token.other;				// Just plain old non alphanumerics.  Returned one character at a time.
				}
			else
				{
				current++;
				/*
					The XML rules from: "Extensible Markup Language (XML) 1.0 (Fifth Edition) W3C Recommendation 26 November 2008", at
					https://www.w3.org/TR/REC-xml are followed as much as is reasonably possible without a writing a
					full XML parser. Square brackets below are XML production numbers.
				*/
				uint32_t codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
 				if (unicode::isxmlnamestartchar(codepoint))
					{
					/*
						[40]     STag          ::= '<' Name (S Attribute)* S? '>'
						[5]      Name          ::= NameStartChar (NameChar)*
						[4]      NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D] | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
						[4a]     NameChar      ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]

						[44]     EmptyElemTag ::= '<' Name (S Attribute)* S? '/>'
					*/
					/*
						Get the name of the tag (without the '<' or '>')
					*/
					const uint8_t *start = current;

					current += bytes;
					codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
					while (current < end_of_document && unicode::isxmlnamechar(codepoint) && bytes != 0)
						{
						current += bytes;
						codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
						}
					const uint8_t *end = current;

					/*
						Attributes are ignored at the moment (Last time I needed to index attributes was in the 1990s!)
					*/
					while (current < end_of_document && *current != '>')
						{
						/*
							skipping over double-quoted attributes (that might contain single-quotes) and vice versa
						*/
						if (*current == '"')
							do
								current++;
							while (current < end_of_document && *current != '"');		// the skip over the '"' happens when this loop exits.
						else if (*current == '\'')
							do
								current++;
							while (current < end_of_document && *current != '\'');		// the skip over the '"' happens when this loop exits.
							
						current++;
						}
						
					/*
						was it an open tag or an empty tag?
					*/
					if (*(current - 1) == '/')
						current_token.type = current_token.xml_empty_tag;				// Empty tag
					else
						current_token.type = current_token.xml_start_tag;				// Start tag.
						
					/*
						copy the tag into the token buffer
					*/
					size_t bytes = maths::minimum((size_t)(end - start), sizeof(current_token.buffer));
					memcpy(current_token.buffer, start, bytes);
					buffer_pos += bytes;
					current++;
					}
				else if (codepoint == '/')
					{
					/*
						[42]     ETag        ::= '</' Name S? '>'
					*/
					codepoint = unicode::utf8_to_codepoint(current + 1, end_of_document, bytes);
	 				if (unicode::isxmlnamestartchar(codepoint))
						{
						current++;
						const uint8_t *found = std::find(current, end_of_document, '>');
						size_t bytes = maths::minimum((size_t)(found - current), sizeof(current_token.buffer));
						memcpy(current_token.buffer, current, bytes);
						buffer_pos = current_token.buffer + bytes;
						current = found + 1;
						current_token.type = current_token.xml_end_tag;
						}
					else
						{
						*buffer_pos++ = *(current - 1);
						current_token.type = current_token.other;				// Just plain old non alphanumerics.  Returned one character at a time.
						}
					}
				else if (codepoint == '?')
					{
					/*
						[16]     PI          ::= '<?' PITarget (S (Char* - (Char* '?>' Char*)))? '?>'
					*/
					static uint8_t close_pi[] = "?>";
					current++;
					const uint8_t *found = std::search(current, end_of_document, close_pi, close_pi + sizeof(close_pi) - 1);
					size_t bytes = maths::minimum((size_t)(found - current), sizeof(current_token.buffer));
					memcpy(current_token.buffer, current, bytes);
					buffer_pos = current_token.buffer + bytes;
					current = found + sizeof(close_pi) - 1;
					current_token.type = current_token.xml_processing_instruction;
					}
				else if (codepoint == '!')
					{
					current++;
					if (current + 4 < end_of_document && *current == '-' && *(current + 1) == '-')
						{
						/*
							[15]     Comment     ::= '<!--' ((Char - '-') | ('-' (Char - '-')))* '-->'
						*/
						static uint8_t close_comment[] = "-->";
						current += 2;
						const uint8_t *found = std::search(current, end_of_document, close_comment, close_comment + sizeof(close_comment) - 1);
						size_t bytes = maths::minimum((size_t)(found - current), sizeof(current_token.buffer));
						memcpy(current_token.buffer, current, bytes);
						buffer_pos = current_token.buffer + bytes;
						current = found + sizeof(close_comment) - 1;
						current_token.type = current_token.xml_comment;
						}
					else if (current < end_of_document && ascii::isupper(*current))
						{
						/*
							[28]   	doctypedecl ::= '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>'
							[45]     elementdecl ::= '<!ELEMENT' S Name S contentspec S? '>'
							[52]     AttlistDecl ::= '<!ATTLIST' S Name AttDef* S? '>'
							[71]     GEDecl      ::= '<!ENTITY' S Name S EntityDef S? '>'
							[72]     PEDecl      ::= '<!ENTITY' S '%' S Name S PEDef S? '>'
							[82]     NotationDecl::= '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
						*/
						const uint8_t *found = std::find(current, end_of_document, '>');
						size_t bytes = maths::minimum((size_t)(found - current), sizeof(current_token.buffer));
						memcpy(current_token.buffer, current, bytes);
						buffer_pos = current_token.buffer + bytes;
						current = found + 1;
						current_token.type = current_token.xml_definition;
						}
					else if (current + 9 < end_of_document && *current == '[' && strncmp((char *)current + 1, "CDATA[", 6) == 0)
						{
						/*
							[18]     CDSect      ::= CDStart CData CDEnd
							[19]     CDStart     ::= '<![CDATA['
							[20]     CData       ::= (Char* - (Char* ']]>' Char*))
							[21]     CDEnd       ::= ']]>'
						*/
						static uint8_t close_block[] = "]]>";
						current += 7;
						const uint8_t *found = std::search(current, end_of_document, close_block, close_block + sizeof(close_block) - 1);
						size_t bytes = maths::minimum((size_t)(found - current), sizeof(current_token.buffer));
						memcpy(current_token.buffer, current, bytes);
						buffer_pos = current_token.buffer + bytes;
						current = found + sizeof(close_block) - 1;
						current_token.type = current_token.xml_cdata;
						}
					else if (current + 4 < end_of_document && *current == '[')
						{
						/*
							[62]     includeSect        ::= '<![' S? 'INCLUDE' S? '[' extSubsetDecl ']]>'
							[63]     ignoreSect         ::= '<![' S? 'IGNORE' S? '[' ignoreSectContents* ']]>'
							[64]     ignoreSectContents ::= Ignore ('<![' ignoreSectContents ']]>' Ignore)*
							[65]     Ignore             ::= Char* - (Char* ('<![' | ']]>') Char*)
							[31]     extSubsetDecl      ::= ( markupdecl | conditionalSect | DeclSep)*
						*/
						/*
							This one is a bit nasty.  An IGNORE ends at the first "]]>", but and INCLUDE does not - it can be pretty much the remainder of the XML file.
							So what we'll return is the tokens between the "<![" and the "[".  Its the caller's problem to deal with the close square brackets.
						*/
						current++;
						const uint8_t *found = std::find(current, end_of_document, '[');
						size_t bytes = maths::minimum((size_t)(found - current), sizeof(current_token.buffer));
						memcpy(current_token.buffer, current, bytes);
						buffer_pos = current_token.buffer + bytes;
						current = found + 1;
						current_token.type = current_token.xml_conditional;
						}
					else
						{
						*buffer_pos++ = *(current - 1);
						current_token.type = current_token.other;				// Just plain old non alphanumerics.  Returned one character at a time.
						}
					}
				else
					{
					*buffer_pos++ = *(current - 1);
					current_token.type = current_token.other;				// Just plain old non alphanumerics.  Returned one character at a time.
					}
				}
			}
		else
			{
			/*
				Unicode Alphabetic (codepoint already contains the current character and bytes is the UTF-8 length in bytes).
			*/
			if (unicode::isalpha(codepoint))
				{
				current_token.type = current_token.alpha;
				build_unicode_alphabetic_token(codepoint, bytes, buffer_pos, buffer_end);
				}
			/*
				Unicode Numeric
			*/
			else if (unicode::isdigit(codepoint))
				{
				current_token.type = current_token.numeric;
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
				current_token.type = current_token.other;
				current += bytes == 0 ? 1 : bytes;
				
				for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
					buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);
				}
			}

		current_token.lexeme = slice((void *)current_token.buffer, (void *)buffer_pos);
		return current_token;
		}

	/*
		PARSER::UNITTEST_COUNT()
		------------------------
	*/
	size_t parser::unittest_count(const char *string)
		{
		parser tokenizer;				// declare a tokenizer
		document example;				// declare an object to pass to the tokenizer
		size_t count;
		parser::token::token_type type;

		example.contents = slice((void *)string, strlen(string));
		tokenizer.set_document(example);
		count = 0;
		do
			{
			const auto &token = tokenizer.get_next_token();
			count++;
			type = token.type;
			}
		while (type != token::eof);

		return count - 1;
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
			"ab",
			"c",
			"123",
			"αβγ",
			"123"
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
		JASS_assert(count == 14);
		

		/*
			Check a whole load of XML stuff
		*/
		const uint8_t xml_data[] = "<![ INCLUDE [<!DOCTYPE note SYSTEM \"Note.dtd\"><DOC a=\"'h\"><?JASS ignore?><!--rem--><![CDATA[<t>text</t>]]><empty/>< notopen></ notclose>< notempty/></DOC>]]> ";
		std::string xml_data_answer[] =
			{
			" INCLUDE ",
			"DOCTYPE note SYSTEM \"Note.dtd\"",
			"DOC",
			"JASS ignore",
			"rem",
			"<t>text</t>",
			"empty",
			"<",
			"notopen",
			">",
			"<",
			"/",
			"notclose",
			">",
			"<",
			"notempty",
			"/",
			">",
			"DOC",
			"]",
			"]",
			">"
			};
		(void)xml_data_answer;				// Silence the  "Unused variable" message when in Release
		example.contents = slice((void *)xml_data, sizeof(xml_data) - 1);
		tokenizer.set_document(example);
		count = 0;
		do
			{
			const auto &token = tokenizer.get_next_token();
			if (token.type != token::eof)
				JASS_assert(std::string((char *)token.lexeme.address(), token.lexeme.size()) == xml_data_answer[count]);
			count++;
			type = token.type;
			}
		while (type != token::eof);
		JASS_assert(count == 23);
		
		
		/*
			Check broken UTF-8 at end of file and inside tag name - all of which are graceful (i.e. non-crashing) "undefined behaviour".
		*/
		JASS_assert(unittest_count("<DOC\xc3></DOC\xc3>\xc3 3\xc3\xE2\x80\x8C") == 6);

		/*
			Check unicode whitespace at end of string
		*/
		JASS_assert(unittest_count(" Z \xE2\x80\x87") == 1);

		/*
			Check alphas and numerics at end of string
		*/
		JASS_assert(unittest_count("Zap") == 1);
		JASS_assert(unittest_count("123") == 1);
		JASS_assert(unittest_count("Zap\xE1\x8E\xA0") == 1);
		JASS_assert(unittest_count("123\xE2\x85\xA9") == 1);

		/*
			Check unusual cases
		*/
		JASS_assert(unittest_count("<name at='val'>") == 1);
		JASS_assert(unittest_count("<![t]>") == 5);
		JASS_assert(unittest_count("\xE1\xAC\xBB") == 1);

		/*
			Yay, we passed
		*/
		puts("parser::PASSED");
		}
	}
