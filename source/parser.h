/*
	PARSER.H
	--------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Simple XML parser that does't do either attributes or entities
	@details This parser is a bare-bones parser that manages (as reasonably as possible) XML and HTML.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdint.h>
#include "document.h"
#include "slice.h"

namespace JASS
	{
	/*
		CLASS PARSER
		------------
	*/
	/*!
		@brief Simple, but fast, XML parser.
		@details This is the parser (tokenizer, or lexical analyser) that is most likely to get used for most documents, especially
		TREC collections.  It does not manage entity references (it will strip the '&' and the ';'.  It does not manage attributes,
		which are ignored.  It does, however, manage start tags, end tags, alphabetic tokens, alphanumeric tokens, comments, and many
		other XML characteristics.
		
		An example tying documents, instreams, and parsing to count the number of document and non-unique symbols is:
		@code
		
		int main(int argc, char *argv[])
			{
			//
			// allocate a document object and a parser object.
			//
			JASS::document document;
			JASS::parser parser;
			
			//
			// build a pipeline - recall that deletes cascade so file is deleted when source goes out of scope.
			//
			JASS::instream_file *file = new JASS::instream_file(argv[1]);
			JASS::instream_document_trec source(*file);

			//
			// this program counts document and alphbetic tokens in those documents.
			//
			size_t total_documents = 0;
			size_t alphas = 0;

			//
			// read document, then parse them.
			//
			do
				{
				//
				// read the next document into the same memory the last document used.
				//
				document.rewind();
				source.read(document);

				//
				// eof is signaled as an empty document.
				//
				if (document.isempty())
					break;

				//
				// count documents.
				//
				total_documents++;

				//
				// now parse the docment.
				//
				parser.set_document(document);
				bool finished = false;
				do
					{
					//
					// get the next token
					//
					const auto &token = parser.get_next_token();
					
					//
					// what type is that token
					//
					switch (token.type)
						{
						case JASS::parser::token::eof:
							//
							// At end of document so signal to leave the loop.
							//
							finished = true;
							break;
						case JASS::parser::token::alpha:
							//
							// Count the number of alphabetic tokens.
							//
							alphas++;
							break;
						default:
							//
							// else ignore the token.
							//
							break;
						}
					}
				while (!finished);
				}
			while (!document.isempty());
			
			//
			// Dump out the the number of documents and the numner of tokens.
			//
			printf("Documents:%lld\n", (long long)total_documents);
			printf("alphas   :%lld\n", (long long)alphas);

			return 0;
			}

		@endcode
	*/
	class parser
		{
		public:
			/*
				CLASS PARSER::TOKEN
				-------------------
			*/
			/*!
				@brief A token as returned by the parser.
				@details The parser returns a reference to a token that the caller must not copy.  That reference is valid untill the next
				call to get_next_token() or the destruction of the parser object, because it is a reference to a member of the parser object.
				A token is of a particular type, numeric, alphabetic, start tag, end tag, etc., which is stored in the type member.  The token
				itself is in token.token member, which is just a slice of the (not '\0' terminated) token.buffer.
				
				The token manages its own memory using the token.buffer member.  That member is an array of bytes of length token.max_token_length.
				Normally a "valid" token will be only a few bytes long, but some over-long tokens are seen.  The maximum length was intially
				1KB, but is prone to change over time.  Note that the longest token that has been seen by the authors is 1MB.  It was pi to
				a million decimal places.  JASS handles extra-long tokens by truncating at length token.max_token_length.
			*/
			class token
				{
				public:
					/*!
						@enum token_type
						@brief The type of the token.
					*/
					enum token_type
						{
						alpha,								///< alphabetic token
						numeric,								///< numeric token
						xml_start_tag,						///< XML start tag (just the tag name)
						xml_empty_tag,						///< XML empty tag (without the "<" or "/>").
						xml_end_tag,						///< XML end tag (without the "<" or ">".
						xml_comment,						///< XML comment (with the "<!--" and-->" removed).
						xml_cdata,							///< XML CDATA (just the CDATA)
						xml_definition,					///< XML definition (DOCTYPE, ELEMENT, etc) without the "<!" and ">".
						xml_conditional,					///< XML conditional.  This is not properly interpreted.  its just the "INCLUDE" or "IGNORE" that is returned (and any space around it).
						xml_processing_instruction,	///< XML processing instruction (a "<?" sequence) with the "<?" and "?>" removed/
						other,								///< Other type of token (punctuation, etc).
						eof									///< The final token is marked as an EOF token (and has no content).
						};
				public:
					static constexpr size_t max_token_length = 1024;		///< Any token longer that this will be truncated at this length
					
				public:
					uint8_t buffer[max_token_length];	///< The token manages its memory through this buffer
					slice lexeme;								///< The token itself, stored as a slice (pointer / length pair)
					token_type type;							///< The type of this token (See token_type)
				};
			
		protected:
			token eof_token;						///< Sentinal returned when reading past end of document.
			const document *document;			///< The document that is currently being parsed.
			uint8_t *current;						///< The current location within the document.
			uint8_t *end_of_document;			///< Pointer to the end of the document, used to avoid read past end of buffer.
			token token;							///< The token that is currently being build.  A reference to this is returned when the token is complete.
			
		protected:
			/*
				PARSER::BUILD_UNICODE_ALPHABETIC_TOKEN()
				----------------------------------------
			*/
			/*!
				@brief Helper function used to build alphabetic token from UTF-8
				@param codepoint [in] The Unicoode codepoint of the first character in the token (which must, by definition, be alphabetic).
				@param bytes [in] The length of the UTF-8 representation of codepoint.
				@param buffer_pos [in/out] Where the UTF-8 representation of the token should be written.
				@param buffer_end [in] The end of the buffer_pos buffer (used to prevent write past end of buffer).
			*/
			void build_unicode_alphabetic_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end);

			/*
				PARSER::BUILD_UNICODE_NUMERIC_TOKEN()
				-------------------------------------
			*/
			/*!
				@brief Helper function used to build numeric token from UTF-8
				@param codepoint [in] The Unicoode codepoint of the first character in the token (which must, by definition, be numeric).
				@param bytes [in] The length of the UTF-8 representation of codepoint.
				@param buffer_pos [in/out] Where the UTF-8 representation of the token should be written.
				@param buffer_end [in] The end of the buffer_pos buffer (used to prevent write past end of buffer).
			*/
			void build_unicode_numeric_token(uint32_t codepoint, size_t bytes, uint8_t *&buffer_pos, uint8_t *buffer_end);

		public:
			/*
				PARSER::PARSER()
				----------------
			*/
			/*!
				@brief Constructor
			*/
			parser()
				{
				/*
					Nothing
				*/
				current = NULL;
				eof_token.type =  token::eof;
				}
			
			/*
				PARSER::~PARSER()
				-----------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~parser()
				{
				/*
					Nothing
				*/
				}

			/*
				PARSER::SET_DOCUMENT()
				----------------------
			*/
			/*!
				@brief Start parsing from the start of this document.
				@param document [in] The document to parse.
			*/
			virtual void set_document(const class document &document)
				{
				this->document = &document;
				current = (uint8_t *)document.contents.address();
				end_of_document = (uint8_t *)document.contents.address() + document.contents.size();
				}
			
			/*
				PARSER::GET_NEXT_TOKEN()
				------------------------
			*/
			/*!
				@brief Continue parsing the input looking for the next token.
				@return A reference to a token object that is valid until either the next call to get_next_token() or the parser is destroyed.
			*/
			virtual const class parser::token &get_next_token(void);
			
			/*
				PARSER::UNITTEST()
				------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
