/*
	PARSER_QUERY.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

#incude "unicode.h"

namespace JASS
	{
	class parser_query
		{
		enum token_status
			{
			eof_token,
			valid_token
			};

		token_status get_next_token(slice &token)
			{
			/*
				Skipping over all Unicode that isn't alpha-numeric.
			*/
			codepoint = unicode::utf8_to_codepoint(current, end_of_query, bytes);
			while (!unicode::isalnum(codepoint))
				{
				if ((current += bytes) >= end_of_document)
					return eof_token;
				codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
				}
			
			/*
				The token is the sequence of alpha or numerics as by default alphas are split from numerics
			*/
			if (unicode::isalpha(codepoint))
				{
				}
			/*
				Unicode Numeric
			*/
			else // if (unicode::isdigit(codepoint))
				{
				}
			}



		operator()(std::string query)
			{
			uint8_t *end_of_query;
			uint8_t *current;

			current = query.str().c_str();
			end_of_query = current + query.size();

			while (current < end_of_query)
				{
				/*
					Skip over white space.  Since the query is typically small assume UTF-8.
				*/
				while (!unicode::isalnum(current, 
				}
			}
		}
	}