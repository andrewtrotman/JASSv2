/*
	INDEX_MANAGER_SEQUENTIAL.H
	--------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Non-thread-Safe indexer object.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "parser.h"
#include "hash_table.h"
#include "index_manager.h"
#include "unittest_data.h"
#include "index_postings.h"
#include "instream_memory.h"

namespace JASS
	{
	/*
		CLASS INDEX_MANAGER_SEQUENTIAL
		------------------------------
	*/
	/*!
		@brief Non-thread-Safe indexer object.
		@details This class is a non-thread-safe indexer used for regular sequential indexing.  It self-contains its memory, uses a hash-table with
		direct chaining (in a non-ballanced tree) and supports a positional index.
	*/
	
	class index_manager_sequential : public index_manager
		{
		private:
			allocator_pool memory;											///< All memory in allocatged from this allocator.
			hash_table<slice, index_postings, 24> index;				///< The index is a hash table of index_postings keyed on the term (a slice).
			
		public:
			/*
				INDEX_MANAGER_SEQUENTIAL::INDEX_MANAGER_SEQUENTIAL()
				----------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			index_manager_sequential() :
				index_manager(),
				index(memory)
				{
				/*
					Nothing
				*/
				}
			
			/*
				INDEX_MANAGER_SEQUENTIAL::~INDEX_MANAGER_SEQUENTIAL()
				-----------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~index_manager_sequential()
				{
				/*
					Nothing
				*/
				}
			
			
			/*
				INDEX_MANAGER_SEQUENTIAL::BEGIN_DOCUMENT()
				------------------------------------------
			*/
			/*!
				@brief Tell this object that you're about to start indexing a new object.
			*/
			virtual void begin_document(void)
				{
				/*
					Tell index_manager that we have started a new document
				*/
				index_manager::begin_document();
				}
			
			/*
				INDEX_MANAGER_SEQUENTIAL::TERM()
				--------------------------------
			*/
			/*!
				@brief Hand a new term from the token stream to this object.
				@param term [in] The term from the token stream.
			*/
			virtual void term(const parser::token &term)
				{
				/*
					Tell index_manager that we have a new term than pass it on to the correct index_postings to manage.
				*/

				index_manager::term(term);
				index[term.lexeme].push_back(get_highest_document_id(), get_highest_term_id());
				}
			
			/*
				INDEX_MANAGER_SEQUENTIAL::END_DOCUMENT()
				----------------------------------------
			*/
			/*!
				@brief Tell this object that you've finished with the current document (and are about to move on to the next, or are completely finished).
			*/
			virtual void end_document(void)
				{
				/*
					Tell index_manager that we have finished with this document.
				*/
				index_manager::end_document();
				}
			
			/*
				INDEX_MANAGER_SEQUENTIAL::TEXT_RENDER()
				---------------------------------------
			*/
			/*!
				@brief Dump a human-readable version of the index down the stream.
				@param stream [in] The stream to write to.
			*/
			virtual void text_render(std::ostream &stream) const
				{
				stream << index;
				}
			
			/*
				INDEX_MANAGER::UNITTEST()
				-------------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void)
				{
				parser parser;								// We need a parser
				document document;						// That creates documents
				instream *file = new instream_memory(unittest_data::ten_documents.c_str(), unittest_data::ten_documents.size());			// From this stream (the standard 10 document stream).
				instream_document_trec source(*file);	// Set up the instream
				index_manager_sequential index;			// And finally a index to populate
				
				do
					{
					/*
						Read the next document and give it to the parser (until eof).
					*/
					document.rewind();
					source.read(document);
					if (document.isempty())
						break;
					parser.set_document(document);
					
					/*
						Mark the start of a new document.
					*/
					index.begin_document();

					/*
						for each document, read the token stream and add to the hash table.
					*/
					bool finished = false;
					do
						{
						/*
							Get the next token.
						*/
						const auto &token = parser.get_next_token();
						
						/*
							different behaviour based on its type.
						*/
						switch (token.type)
							{
							case JASS::parser::token::eof:
								finished = true;						// finish up.
								break;
							case JASS::parser::token::alpha:
							case JASS::parser::token::numeric:
								index.term(token);					// add it to the index.
								break;
							default:
								break;									// ignore.
							}
						}
					while (!finished);
					
					/*
						Mark that we're at the end of the document
					*/
					index.end_document();
					}
				while (!document.isempty());
				
				std::cout << index;
				
				puts("index_manager_sequential::PASSED");
				}
		};
	}
