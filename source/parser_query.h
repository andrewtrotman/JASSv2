/*
	PARSER_QUERY.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Simple parser for queries
	@details This parser is a bare-bones parser that generates a list of terms in the query
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/

#pragma once

#include "allocator.h"
#include "dynamic_array.h"
#include "query_term_list.h"

namespace JASS
	{
	/*
		CLASS PARSER_QUERY
		------------------
	*/
	/*!
		@brief Simple interface to parsing queries
	*/
	class parser_query
		{
		private:
			/*!
				@enum token_status
				@brief the parser returns whether the next token is valid or not.
			*/
			enum token_status
				{
				eof_token,								///< At the end of porcessing tokens so not a valid token.
				bad_token,								///< The token is a bad token (for example, it might have an invalid UTF8 character in it).
				valid_token								///< The token is a valid token
				};

		private:
			allocator &memory;						///< All memory associated with the query.
			uint8_t *current;							///< Currtne locaton (in the input query string) of the parser during parsing.
			uint8_t *end_of_query;					///< Pointer to the end of the inoput query string.
			uint8_t *buffer_pos;						///< Loction where the next token will be written during tokenization and normaloisation.
			uint8_t *buffer_end;						///< End of the normalised token buffer.

		private:
			/*
				PARSER_QUERY::GET_NEXT_TOKEN()
				------------------------------
			*/
			/*!
				@brief Return the next parsed token from the source query.
				@param token [in] a slice of the token.
			*/
			token_status get_next_token(slice &token);

			/*
				PARSER_QUERY::UNITTEST_TEST_ONE()
				---------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static std::string unittest_test_one(parser_query &parser, allocator &memory, const std::string &query);


		public:
			/*
				PARSER_QUERY::PARSER_QUERY()
				----------------------------
			*/
			/*!
				@brief Constructor
				@param memory [in] all allocations happen in the memory object.
			*/
			parser_query(allocator &memory) :
				memory(memory),
                current(nullptr),
                end_of_query(nullptr),
                buffer_pos(nullptr),
                buffer_end(nullptr)
				{
				/* Nothing */
				}

			/*
				PARSER_QUERY::PARSE()
				---------------------
			*/
			/*!
				@brief parse and return the list of query tokens.
				@tparam STRING_TYPE either a std::string or JASS::string (or other string type)
				@param parsed_query [out] The parsed query once parsed.
				@param query [in] The query to be parsed.
			*/
			template <typename STRING_TYPE>
			void parse(query_term_list &parsed_query, const STRING_TYPE &query)
				{
				current = (uint8_t *)(const_cast<char *>(query.c_str()));							// get a pointer to the start of the query string
				end_of_query = current + query.size();			// get a pointer to the end of the query string

				/*
					Allocate space for the normalised query terms
				*/
				size_t worse_case_normalised_query_length = unicode::max_casefold_expansion_factor * unicode::max_utf8_bytes * query.size();		// might be as much as (18 * 4) times the size iof the input string (worst case0
				buffer_pos = (uint8_t *)memory.malloc(worse_case_normalised_query_length);
				if (buffer_pos == nullptr)
					return;				// LCOV_EXCL_LINE			// At time of writing this can't happen because either malloc will assert or delatyed allocation will not return nullptr!
				buffer_end = buffer_pos + worse_case_normalised_query_length;

				slice term;												// Each term as returned by the parser.

				token_status status;
				while ((status = get_next_token(term)) != eof_token)		// get the next token
					if (status == valid_token)
						parsed_query.push_back(query_term(term));
				}

			/*
				PARSER_QUERY::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
