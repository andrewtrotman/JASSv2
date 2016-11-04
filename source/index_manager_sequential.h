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
				INDEX_MANAGER_SEQUENTIAL::UNITTEST()
				------------------------------------
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
				
				/*
					This is the answer that is expected
				*/
				std::string answer
					(
					"6-><6,1,21>\n"
					"1-><1,1,1>\n"
					"4-><4,1,10>\n"
					"5-><5,1,15>\n"
					"3-><3,1,6>\n"
					"8-><8,1,36>\n"
					"7-><7,1,28>\n"
					"2-><2,1,3>\n"
					"9-><9,1,45>\n"
					"10-><10,1,55>\n"
					"four-><7,1,35><8,1,43><9,1,52><10,1,62>\n"
					"eight-><3,1,9><4,1,13><5,1,18><6,1,24><7,1,31><8,1,39><9,1,48><10,1,58>\n"
					"five-><6,1,27><7,1,34><8,1,42><9,1,51><10,1,61>\n"
					"seven-><4,1,14><5,1,19><6,1,25><7,1,32><8,1,40><9,1,49><10,1,59>\n"
					"two-><9,1,54><10,1,64>\n"
					"six-><5,1,20><6,1,26><7,1,33><8,1,41><9,1,50><10,1,60>\n"
					"three-><8,1,44><9,1,53><10,1,63>\n"
					"one-><10,1,65>\n"
					"nine-><2,1,5><3,1,8><4,1,12><5,1,17><6,1,23><7,1,30><8,1,38><9,1,47><10,1,57>\n"
					"ten-><1,1,2><2,1,4><3,1,7><4,1,11><5,1,16><6,1,22><7,1,29><8,1,37><9,1,46><10,1,56>\n"
					);

				/*
					The easiest way to test this is to actually build the index for a set of document... and this is where unittest_data::ten_documents comes in handy
				*/
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
				
				std::ostringstream computed_result;
				computed_result << index;
				
				JASS_assert(computed_result.str() == answer);

				/*
					Test the iterator
				*/
				for (const auto pair : index)
					std::cout << pair.first;

				/*
					Done
				*/
				puts("index_manager_sequential::PASSED");
				}
		};
	}
