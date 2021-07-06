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
		@details k-mers are character n-grams.  This parser takes the input document, strips the header from the FASTA record (i.e. drops the first line)
		then returns the remainder of the document as a set of character n-grams starting at the first DNA character.  It assumes the document
		is syntactically correct.  If the document is not syntactically correct the n-grams are computed from the whole document.  If a non-base is seen the
		the parser skips that token (its invalid) and finds the next valid token.
	*/
	class parser_fasta : public parser
		{
		private:
			/*!
				The first line of a document is text and should be indexed as such.  The remainder of the document is DNA and should be converted into k-mers
			*/
			enum parser_mode
				{
				TEXT,
				DNA
				};

		private:
			size_t kmer_length;					///< The length of the k-mers to compute from the DNA sequences
			parser_mode mode;						///< The mode (TEXT or DNA) of the tokenizer;
			uint8_t *end_of_fasta_document;		///< Pointer to the end of the FASTA document, end_of_document points to the end of the first line (the primary key) before the DNA starts.

		protected:
			/*
				PARSER_FASTA::GET_NEXT_TOKEN_DNA()
				----------------------------------
			*/
			/*!
				@brief Continue parsing the input looking for the next DNA k-mer token.
				@return A reference to a token object that is valid until either the next call to get_next_token() or the parser is destroyed.
			*/
			const class parser::token &get_next_token_dna(void);

		public:
			/*
				PARSER_FASTA::PARSER_FASTA()
				----------------------------
			*/
			/*!
				@brief Constructor
			*/
			parser_fasta(size_t kmer_length) :
				kmer_length(kmer_length),
				mode(TEXT),
				end_of_fasta_document(nullptr)
				{
				/*
					We can only return alpha tokens or eof tokens so set the token type here to alpha
				*/
				current_token.type = current_token.alpha;
				current_token.count = 1;
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
				current = (uint8_t *)document.contents.address();
				end_of_fasta_document = (uint8_t *)document.contents.address() + document.contents.size();

				/*
					Start in TEXT mode and "pretend" that the document stops at the start of the DNA (the end of the first line)
				*/
				mode = TEXT;
				end_of_document = static_cast<uint8_t *>(std::find(static_cast<uint8_t *>(document.contents.address()), end_of_fasta_document, '\n'));
				}

			/*
				PARSER_FASTA::GET_NEXT_TOKEN()
				------------------------------
			*/
			/*!
				@brief Continue parsing the input looking for the next token.
				@return A reference to a token object that is valid until either the next call to get_next_token() or the parser is destroyed.
			*/
			virtual const class parser::token &get_next_token(void)
				{
				if (mode == DNA)
					return get_next_token_dna();
				else
					{
					const token &got = parser::get_next_token();
					if (got.type == token::token_type::eof)
						{
						mode = DNA;
						end_of_document = end_of_fasta_document;		// shift to the end of document being the end of the FASTA document not just the end of the text.
						return get_next_token_dna();
						}
					return got;
					}
				}

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
