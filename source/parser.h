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
					enum type
						{
						alpha,
						numeric
						};
				public:
					static constexpr size_t max_token_length = 1024;
					
				public:
					uint8_t buffer[max_token_length];
					slice token;
					type type;
				};
			
		protected:
			document *document;
			uint8_t *current;
			uint8_t *end_of_document;
			token token;

		public:
			parser()
				{
				/*
					Nothing
				*/
				current = NULL;
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
				
			virtual class token &get_next_token(void);
		};
	}
