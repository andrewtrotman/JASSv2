/*
	PARSER_FASTA.H
	--------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Parser to turn DNA sequences in FASTA format into k-mers for indexing.
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include <stdint.h>

#include "slice.h"
#include "parser.h"
#include "document.h"

namespace JASS
	{
	/*
		CLASS PARSER_FASTA
		------------------
	*/
	/*!
		@brief Parser to turn DNA sequences in FASTA format into k-mers for indexing.
		@details kmers are character n-grams.  This parser takes the input document, strips the header from the FASTA record (i.e. drops the first line)
		then converts returns the remainder of the document as a set of character n-grams starting at the first DNA character.  It assumes the document
		is syntactically correct.  If the document is not syntactically correct the n-grams are computed from the whole document.
	*/
	class parser_fasta : public parser
		{
		private:
			size_t kmer_length;

		public:
			/*
				PARSER_FASTA::PARSER_FASTA()
				----------------------------
			*/
			/*!
				@brief Constructor
			*/
			parser_fasta(size_t kmer_length) :
				kmer_length(kmer_length)
				{
				/*
					Nothing
				*/
				}
			
			/*
				PARSER_FASTA::~PARSER_FASTA()
				-----------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~parser_fasta()
				{
				/*
					Nothing
				*/
				}

			/*
				PARSER_FASTA::SET_DOCUMENT()
				----------------------------
			*/
			/*!
				@brief Start parsing from the start of this document.
				@details  The document must be a '\0' terminated string.  Syntactically correct FASTA is assumed, if the necessary
				parts are not found then n-grams from the start of the whole document are used.
				@param document [in] The document to parse.
			*/
			virtual void set_document(const class document &document)
				{
				the_document = &document;
				end_of_document = static_cast<uint8_t *>(document.contents.address()) + document.contents.size();
				if ((current = static_cast<uint8_t *>(std::find(static_cast<uint8_t *>(document.contents.address()), end_of_document, '\n'))) == end_of_document)
					current = (uint8_t *)document.contents.address();			// document is badly formed so use the while document
				else
					current++;			// skip over the '\n' character.
				}

			/*
				PARSER_FASTA::GET_NEXT_TOKEN()
				------------------------------
			*/
			/*!
				@brief Continue parsing the input looking for the next token.
				@return A reference to a token object that is valid until either the next call to get_next_token() or the parser is destroyed.
			*/
			virtual const class parser::token &get_next_token(void);

			/*
				PARSER_FASTA::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
