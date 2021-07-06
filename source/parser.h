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

#include "slice.h"
#include "document.h"
#include "index_postings_impact.h"

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
		
		@include parser_use.cpp
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
					uint8_t buffer[max_token_length];				///< The token manages its memory through this buffer
					slice lexeme;											///< The token itself, stored as a slice (pointer / length pair)
					token_type type;										///< The type of this token (See token_type)
					index_postings_impact::impact_type count;		///< The number of times the token is seen (normally 1, but if parsing a forward index it might be known to be larget).

				public:
					/*
						PARSER::TOKEN::GET()
						--------------------
					*/
					/*!
						@brief Return a reference to the token as a slice.  The life of the slice is the life of the object.
						@return The token.
					*/
					const slice &get(void) const
						{
						return lexeme;
						}

					/*
						PARSER::TOKEN::SET()
						--------------------
					*/
					/*!
						@brief create a token from a slice without copying it and without looking to see what kind of token it is.
					*/
					void set(slice term)
						{
						*buffer = '\0';
						lexeme = term;
						type = other;
						count = 1;
						}
				};

		private:
			document build_document;			///< A document used when a string is passed into this object.

		protected:
			token eof_token;						///< Sentinal returned when reading past end of document.
			const document *the_document;		///< The document that is currently being parsed.
			const uint8_t *current;				///< The current location within the document.
			const uint8_t *end_of_document;	///< Pointer to the end of the document, used to avoid read past end of buffer.
			token current_token;					///< The token that is currently being build.  A reference to this is returned when the token is complete.
			
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
				current = NULL;
				the_document = NULL;
				end_of_document = NULL;
				eof_token.type =  token::eof;
				current_token.count = 1;			// We're generating a stream of individual tokens, so each all counts are 1.
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
				@details  The document must be a '\0' terminated string.
				@param document [in] The document to parse.
			*/
			virtual void set_document(const class document &document)
				{
				the_document = &document;
				current = (uint8_t *)document.contents.address();
				end_of_document = (uint8_t *)document.contents.address() + document.contents.size();
				}

			/*
				PARSER::SET_DOCUMENT()
				----------------------
			*/
			/*!
				@brief Parse a string (rather than a document).
				@param document [in] The document to parse.  Must remain in scope for the entire parsing process (a copy is not taken).
			*/
			virtual void set_document(const std::string &document)
				{
				current = reinterpret_cast<const uint8_t *>(document.c_str());
				end_of_document = current + document.size();
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
				PARSER::UNITTEST_COUNT()
				------------------------
			*/
			/*!
				@brief count the numner of tokens in the given string.
				@param string [in] The string to count characters in.
				@return The number of tokens in string.
			*/
			static size_t unittest_count(const char *string);

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
