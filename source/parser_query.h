/*
	PARSER_QUERY.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

#include "unicode.h"
#include "allocator.h"
#include "query_term.h"
#include "dynamic_array.h"

namespace JASS
	{
	class parser_query
		{
		private:
			enum token_status
				{
				eof_token,								///< At the end of porcessing tokens
				bad_token,								///< We have a bad token (for exa
				valid_token
				};

		private:
			allocator &memory;						///< All memory associated with the query.
			token_list terms;							///< Array of query terms once parsed.
			uint8_t *current;							///< Currtne locaton (in the input query string) of the parser during parsing.
			uint8_t *end_of_query;					///< Pointer to the end of the inoput query string.
			uint8_t *buffer_pos;						///< Loction where the next token will be written during tokenization and normaloisation.
			uint8_t *buffer_end;						///< End of the normalised token buffer.

		public:
			parser_query(allocator &memory) :
				memory(memory),
				terms(memory)
				{
				/*
					Nothing
				*/
				}


			token_list &get_tokens(void)
				{
				return terms;
				}



			token_status get_next_token(slice &token)
				{
				size_t bytes;
				uint32_t codepoint;
				uint8_t *start_of_token = buffer_pos;

				/*
					Skipping over all Unicode that isn't alpha-numeric.
				*/
				codepoint = unicode::utf8_to_codepoint(current, end_of_query, bytes);
				while (!unicode::isalnum(codepoint))
					{
					if ((current += bytes) >= end_of_query)
						return eof_token;
					codepoint = unicode::utf8_to_codepoint(current, end_of_query, bytes);
					}
				
				/*
					The token is the sequence of alpha or numerics as by default alphas are split from numerics
				*/
				if (unicode::isalpha(codepoint))
					{
					/*
						while we have alphabetics, case fold into the token buffer.
					*/
					do
						{
						/*
							Case fold.
						*/
						for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
							{
							size_t rewrite_bytes = unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow
							buffer_pos += rewrite_bytes;
							if (rewrite_bytes == 0)
								return bad_token;
							}

						/*
							Get the next codepoint
						*/
						if ((current += bytes) >= end_of_query || bytes == 0)
							break;
						codepoint = unicode::utf8_to_codepoint(current, end_of_query, bytes);
						}
					while (unicode::isalpha(codepoint));
					}
				/*
					Unicode Numeric
				*/
				else // if (unicode::isdigit(codepoint))
					{
					/*
						while we have numerics, case fold into the token buffer.
					*/
					do
						{
						/*
							Case fold.  Yes, its necessary to casefold numerics.  Some Unicode codepoints turn into more than
							one digit when normalised.  For example, the sumbol for a half (1/2) becomes a 1 and a 2.
						*/
						for (const uint32_t *folded = unicode::tocasefold(codepoint); *folded != 0; folded++)
							{
							buffer_pos += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow
 							size_t rewrite_bytes = unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow
							buffer_pos += rewrite_bytes;
							if (rewrite_bytes == 0)
								return bad_token;
							}

						/*
							Get the next codepoint
						*/
						if ((current += bytes) >= end_of_query || bytes == 0)
							break;
						codepoint = unicode::utf8_to_codepoint(current, end_of_query, bytes);
						}
					while (unicode::isdigit(codepoint));
					}

				/*
					write to the slice
				*/
				token = slice(start_of_token, buffer_pos - start_of_token);
				return valid_token;
				}



			void parse(const std::string query)
				{
				current = (uint8_t *)(const_cast<char *>(query.c_str()));							// get a pointer to the start of the query string
				end_of_query = current + query.size();			// get a pointer to the end of the query string

				/*
					Allocat4e space for the tokenised query
				*/
				size_t worse_case_normalised_query_length = unicode::max_casefold_expansion_factor * unicode::max_utf8_bytes * query.size();		// might be as much as (18 * 4) times the size iof the input string (worst case0
				buffer_pos = (uint8_t *)memory.malloc(worse_case_normalised_query_length);
				buffer_end = buffer_pos + worse_case_normalised_query_length;

				slice term;												// Each term as returned by the parser.

				while (get_next_token(term) != eof_token)		// get th next token
					terms.push_back(query_term(term));
				}


				

			static void unittest(void)
				{
				allocator_pool memory;
				parser_query parser(memory);
				parser.parse("Example Query");


				token_list &tokens = parser.get_tokens();
				for (const auto &t : tokens)
					{
					std::cout << t;
					}
				}
		};
	}
