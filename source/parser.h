/*
	PARSER.H
	--------
*/
#pragma once

#include <stdint.h>
#include "document.h"
#include "slice.h"

namespace JASS
	{
	/*
		class PARSER
		------------
	*/
	class parser
		{
		public:
			class token
				{
				public:
					enum token_type
						{
						alpha,
						numeric,
						other,
						eof
						};
				public:
					static constexpr size_t max_token_length = 1024;
					
				public:
					uint8_t buffer[max_token_length];
					slice token;
					token_type type;
				};
			
		protected:
			token eof_token;
			document *document;
			uint8_t *current;
			uint8_t *end_of_document;
			token token;
			
		protected:
			void build_unicode_alphabetic_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end);
			void build_unicode_numeric_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end);

		public:
			parser()
				{
				/*
					Nothing
				*/
				current = NULL;
				eof_token.type =  token::eof;
				}
			virtual ~parser()
				{
				/*
					Nothing
				*/
				}

			void set_document(class document &document)
				{
				this->document = &document;
				current = (uint8_t *)document.contents.address();
				end_of_document = (uint8_t *)document.contents.address() + document.contents.size();
				}
				
			virtual const class parser::token &get_next_token(void);
			
			static void unittest(void);
		};
	}
