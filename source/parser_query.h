/*
	PARSER_QUERY.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#pragma once

#include "unicode.h"
#include "allocator.h"
#include "dynamic_array.h"

namespace JASS
	{
	class parser_query
		{
		private:
			enum token_status
				{
				eof_token,
				bad_token,
				valid_token
				};

		public:
			class query_term
				{
				public:
					size_t frequency;
					slice term;
				};

		private:
			allocator &memory;
			dynamic_array<query_term> terms;
			uint8_t *current;
			uint8_t *end_of_query;

		public:
			parser_query(allocator &memory) :
				memory(memory),
				query_terms(memory)
				{
				/*
					Nothing
				*/
				}


			token_status get_next_token(slice &token)
				{
				size_t bytes;
				uint32_t codepoint;

				/*
					Skipping over all Unicode that isn't alpha-numeric.
				*/
				codepoint = unicode::utf8_to_codepoint(current, end_of_query, bytes);
				while (!unicode::isalnum(codepoint))
					{
					if ((current += bytes) >= end_of_document)
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
							size_t rewrite_bytes += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow
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
 							size_t rewrite_bytes += unicode::codepoint_to_utf8(buffer_pos, buffer_end, *folded);						// won't write on overflow
							buffer_pos += rewrite_bytes;
							if (rewrite_bytes == 0)
								return bad_token;
							}

						/*
							Get the next codepoint
						*/
						if ((current += bytes) >= end_of_document || bytes == 0)
							break;
						codepoint = unicode::utf8_to_codepoint(current, end_of_document, bytes);
						}
					while (unicode::isdigit(codepoint));
					}

				/*
					write to the slice
				*/
				slice.pointer =
				slice.length =
				return valid_token;
				}



			void parse(std::string query)
				{
				current = query.c_str();
				end_of_query = current + query.size();

				slice term;

				while (get_next_token(term) != eof_token)
					{
					buffer_pos =		// start of token buffer
					buffe_end =			// end of token buffer
					terms.push_back(term);
					}
				}
		};
	}
