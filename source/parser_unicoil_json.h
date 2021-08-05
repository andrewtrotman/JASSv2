/*
	PARSER_UNICOIL_JSON.H
	---------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Parser for documents from the UniCOIL data in JSON format
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@details A document has had much of the unnecessary stuff already removed and should look like this (i.e. a forward index):

	"the": 94, "presence": 102, "of": 80, "communication": 147, "amid": 124, "scientific": 134, "minds": 108, "was": 71, "equally": 106,
	"important": 142, "to": 59, "success": 146, "manhattan": 162, "project": 116, "as": 34, "intellect": 135, ".": 63, "only": 98, "cloud": 117, "hanging": 80, "over": 62,
	"impressive": 112, "achievement": 112, "atomic": 149, "researchers": 94, "and": 48, "engineers": 101, "is": 35, "what": 82, "their": 78, "truly": 89, "meant": 84, ";": 58,
	"hundreds": 100, "thousands": 101, "innocent": 101, "lives": 80, "ob": 103, "##lite": 88, "##rated": 96, "[SEP]": 0, "amongst": 36, "scientists": 126, "why": 54, "?": 50,
	"about": 25, "so": 53, "a": 29, "importance": 95, "purpose": 45, "how": 27, "significant": 61, "in": 36, "for": 34, "believe": 49, "who": 42, "did": 68

	{"importance": 95} means the term "importance" has an impact score of 95.
*/
#pragma once

#include <charconv>

#include "ascii.h"
#include "parser.h"
#include "unicode.h"

namespace JASS
	{
	/*
		CLASS PARSER_UNICOIL_JSON
		-------------------------
	*/
	/*!
		@brief Parser for documents from the UniCOIL data in JSON format.
	*/
	class parser_unicoil_json : public parser
		{
		public:
			/*
				PARSER_UNICOIL_JSON::PARSER_UNICOIL_JSON()
				------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			parser_unicoil_json() :
				parser()
				{
				/* Nothing */
				}

			/*
				PARSER_UNICOIL_JSON::~PARSER_UNICOIL_JSON()
				-------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~parser_unicoil_json()
				{
				/*
					Nothing
				*/
				}

			/*
				PARSER_UNICOIL_JSON::GET_NEXT_TOKEN()
				-------------------------------------
			*/
			/*!
				@brief Continue parsing the input looking for the next token.  Note that the definition of token here is no the normal JASS definition, ##term (and worse) are considered tokens!
				@return A reference to a token object that is valid until either the next call to get_next_token() or the parser is destroyed.
			*/
			virtual const class parser::token &get_next_token(void)
				{
				/*
					Check for EOF
				*/
				if (current >= end_of_document)
					return eof_token;

				/*
					Set up the parser
				*/
				uint8_t *buffer_pos = current_token.buffer;
				uint8_t *buffer_end = current_token.buffer + sizeof(current_token.buffer);

				/*
					Find the term
				*/
				while (current < end_of_document)
					{
					if (*current == '"')
						break;
					current++;
					}

				current++;

				/*
					Copy the term to the token buffer
				*/
				while (current < end_of_document)
					{
					if (*current == '\\')
						{
						if (buffer_pos < buffer_end)
							*buffer_pos++ = *current;
						current++;				// escaped characters in JSON
						}
					else if (*current == '"')
						break;
					if (buffer_pos < buffer_end)
						*buffer_pos++ = *current;
					current++;
					}

				/*
					Find the term count
				*/
				while (current < end_of_document)
					{
					if (ascii::isdigit(*current))
						break;
					current++;
					}

				const uint8_t *count_start = current;

				while (current < end_of_document)
					{
					if (!ascii::isdigit(*current))
						break;
					current++;
					}

				const uint8_t *count_finish = current;

				/*
					Construct a token object
				*/
				current_token.lexeme = slice((void *)current_token.buffer, (void *)buffer_pos);
				current_token.type = token::alpha;
				std::from_chars((char *)count_start, (char *)count_finish, current_token.count);

				if (current_token.count > index_postings_impact::largest_impact)
					{
					std::cout << current_token.lexeme;
					std::cout << " " << current_token.count << "\n";
					current_token.count = index_postings_impact::largest_impact;
					}

				return current_token.count > 0 ? current_token : get_next_token();
				}

			/*
				PARSER_UNICOIL_JSON::UNITTEST()
				-------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
