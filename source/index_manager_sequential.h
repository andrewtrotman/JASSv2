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

#include <sstream>

#include "parser.h"
#include "hash_table.h"
#include "index_manager.h"
#include "unittest_data.h"
#include "index_postings.h"
#include "instream_memory.h"
#include "instream_document_trec.h"

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
			dynamic_array<slice> primary_key;							///< The list of primary keys (i.e. external document identifiers) allocated in memory.
			
		public:
			/*
				CLASS INDEX_MANAGER_SEQUENTIAL::DELEGATE
				----------------------------------------
			*/
			/*!
				@brief Base class for the callback function called by iterate.
			*/
			class delegate : public index_manager::delegate
				{
				private:
					std::ostream &postings_out;				///< The unit test iterates into this.
					std::ostream &primary_keys_out;			///< The unit test iterates into this.

				public:
					/*
						INDEX_MANAGER_SEQUENTIAL::DELEGATE::OPERATOR()()
						------------------------------------------------
					*/
					/*!
						@brief Constructor
						@param postings [out] data is written into this object.
						@param primary_keys [in] primary key data is ertten into this object.
					*/
					delegate(std::ostream &postings, std::ostream &primary_keys) :
						postings_out(postings),
						primary_keys_out(primary_keys)
						{
						/* Nothing */
						}

					/*
						INDEX_MANAGER_SEQUENTIAL::DELEGATE::OPERATOR()()
						------------------------------------------------
					*/
					/*!
						@brief Destructor
					*/
					virtual ~delegate()
						{
						/* Nothing */
						}

					/*
						INDEX_MANAGER_SEQUENTIAL::DELEGATE::OPERATOR()()
						------------------------------------------------
					*/
					/*!
						@brief The callback function to serialise the postings (given the term) is operator().
						@param term [in] The term name.
						@param postings [in] The postings lists.
					*/
					virtual void operator()(const slice &term, const index_postings &postings)
						{
						postings_out << term << "->" << postings << std::endl;
						}

					/*
						INDEX_MANAGER_SEQUENTIAL::DELEGATE::OPERATOR()()
						------------------------------------------------
					*/
					/*!
						@brief The callback function to serialise the primary keys (external document ids) is operator().
						@param document_id [in] The internal document identfier.
						@param primary_key [in] This document's primary key (external document identifier).
					*/
					virtual void operator()(size_t document_id, const slice &primary_key)
						{
						primary_keys_out << document_id << "->" << primary_key << std::endl;
						}
				};

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
				index(memory),
				primary_key(memory)
				{
				/* Nothing */
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
				/* Nothing */
				}
			
			
			/*
				INDEX_MANAGER_SEQUENTIAL::BEGIN_DOCUMENT()
				------------------------------------------
			*/
			/*!
				@brief Tell this object that you're about to start indexing a new object.
				@param external_id [in] The document's primary key (or external document identifier).
			*/
			virtual void begin_document(const slice &external_id)
				{
				/*
					Tell index_manager that we have started a new document
				*/
				index_manager::begin_document(external_id);
				primary_key.push_back(slice(memory, external_id));
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
				INDEX_MANAGER_SEQUENTIAL::ITERATE()
				-----------------------------------
			*/
			/*!
				@brief Iterate over the index calling callback.operator() with each postings list.
				@param callback [in] The callback to call.
			*/
			virtual void iterate(index_manager::delegate &callback) const
				{
				/*
					Iterate over the hash table calling the callback function with each term->postings pair.
				*/
				for (const auto &listing : index)
					callback(listing.first, listing.second);

				/*
					Iterate over the primary keys calling the callback function with each docid->key pair.
					Note that the search engine counts documents from 1, not from 0.
				*/
				size_t instance = 0;
				callback(instance, slice("-"));
				for (const auto term : primary_key)
					callback(++instance, term);
				}

			/*
				INDEX_MANAGER_SEQUENTIAL::UNITTEST_BUILD_INDEX()
				------------------------------------------------
				Build a index from the standard 10-document collection.
			*/
			/*!
				@brief Build and index for the 10 sample documents.  This is used by several unit tests that need a valid index.
				@param index [out] The index once built.
			*/
			static void unittest_build_index(index_manager_sequential &index, const std::string &document_collection)
				{
				class parser parser;								// We need a parser
				document document;						// That creates documents
				std::shared_ptr<instream> file(new instream_memory(document_collection.c_str(), document_collection.size()));			// From this stream (the standard 10 document stream).
				instream_document_trec source(file);	// Set up the instream
				
				/*
					Build an index
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
					index.begin_document(document.primary_key);

					/*
						For each document, read the token stream and add to the hash table.
					*/
					bool finished = false;
					do
						{
						/*
							Get the next token.
						*/
						const auto &token = parser.get_next_token();
						
						/*
							Different behaviour based on its type.
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
				/*
					This is the postings answer that is expected
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
					This is the docid to primary_key answer
				*/
				std::string primary_key_answer
					(
					"0->-\n"
					"1->1\n"
					"2->2\n"
					"3->3\n"
					"4->4\n"
					"5->5\n"
					"6->6\n"
					"7->7\n"
					"8->8\n"
					"9->9\n"
					"10->10\n"
					);

				/*
				   Build the index for the standard 10 document collection
				*/
				index_manager_sequential index;
				unittest_build_index(index, unittest_data::ten_documents);

				/*
					Serialise it
				*/
				std::ostringstream computed_result;
				computed_result << index;
				
				/*
					Check it produced the expected index
				*/
				JASS_assert(computed_result.str() == answer);

				/*
					Test the iterating callback mechanism
				*/
				std::ostringstream postings_result;
				std::ostringstream primary_key_result;
				delegate callback(postings_result, primary_key_result);
				index.iterate(callback);

				JASS_assert(postings_result.str() == answer);
				JASS_assert(primary_key_result.str() == primary_key_answer);

				/*
					Done
				*/
				puts("index_manager_sequential::PASSED");
				}
		};
	}
