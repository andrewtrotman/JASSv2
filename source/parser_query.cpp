/*
	PARSER_QUERY.CPP
	----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <sstream>

#include "unicode.h"
#include "parser_query.h"

namespace JASS
	{
	/*
		PARSER_QUERY::GET_NEXT_TOKEN()
		------------------------------
	*/
	/*!
		@brief Return the next parsed token from the source query.
		@param token [in] a slice of the token.
	*/
	parser_query::token_status parser_query::get_next_token(slice &token)
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
			if (bytes == 0 || (current += bytes) >= end_of_query)
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
			Write to the slice
		*/
		token = slice(start_of_token, buffer_pos - start_of_token);
		return valid_token;
		}

	/*
		PARSER_QUERY::UNITTEST_TEST_ONE()
		---------------------------------
	*/
	std::string parser_query::unittest_test_one(parser_query &parser, allocator &memory, const std::string &query)
		{
		std::ostringstream buffer;
		token_list tokens(memory);

		parser.parse(tokens, query);
		for (const auto &term : tokens)
			buffer << term;

		return buffer.str();
		}


	/*
		PARSER_QUERY::UNITTEST()
		------------------------
	*/
	void parser_query::unittest(void)
		{
		allocator_pool memory;
		parser_query parser(memory);
		std::string got;

		const uint8_t sequence_1[] = {0x41, 0x00, 0x00, 0x00};
		const uint8_t sequence_2[] = {0xC3, 0x80, 0x00, 0x00};
		const uint8_t sequence_3[] = {0xE6, 0x88, 0x91, 0x00};
		const uint8_t sequence_4[] = {0xF0, 0xA9, 0xB8, 0xBD};
		const uint8_t sequence_bad[] = {0xF8, 0x00, 0x00, 0x00, 0x00};

		got = unittest_test_one(parser, memory, "Example");
		JASS_assert(got == "(example,1)");

		got = unittest_test_one(parser, memory, "  Example  ");
		JASS_assert(got == "(example,1)");

		got = unittest_test_one(parser, memory, "  Example  QUERY");
		JASS_assert(got == "(example,1)(query,1)");

		got = unittest_test_one(parser, memory, "Example Query");
		JASS_assert(got == "(example,1)(query,1)");

		got = unittest_test_one(parser, memory, "   Example Query   ");
		JASS_assert(got == "(example,1)(query,1)");

		got = unittest_test_one(parser, memory, std::string((char *)sequence_1));
		JASS_assert(got == "(a,1)");

		got = unittest_test_one(parser, memory, std::string((char *)sequence_2));
		JASS_assert(got == "(a,1)");

		got = unittest_test_one(parser, memory, std::string((char *)sequence_3));
		JASS_assert(got == "(我,1)");

		got = unittest_test_one(parser, memory, std::string((char *)sequence_4));
		std::cout << got << std::endl;
		JASS_assert(got == "(𩸽,1)");

		got = unittest_test_one(parser, memory, std::string((char *)sequence_bad));
		JASS_assert(got == "");			// this should be the fail state

		puts("parser_query::PASSED");
		}

	}
