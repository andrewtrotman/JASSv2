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

#include <vector>
#include <sstream>

#include "parser.h"
#include "posting.h"
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
			allocator_pool memory;														///< All memory in allocatged from this allocator.
			hash_table<slice, index_postings, 24> index;							///< The index is a hash table of index_postings keyed on the term (a slice).
			dynamic_array<slice> primary_key;										///< The list of primary keys (i.e. external document identifiers) allocated in memory.
			
			/*
				Each of these buffers is re-used in the serialisation process
			*/
			compress_integer::integer *document_ids;					///< The re-used buffer storing decoded document ids
			index_postings_impact::impact_type *term_frequencies;	///< The re-used buffer storing the term frequencies
			size_t temporary_size;											///< The number of bytes in temporary
			uint8_t *temporary;												///< Temporary buffer - cannot be used to store anything between calls

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
						@param documents_in_collection [in] The total number of documents in the collection
						@param postings [out] data is written into this object.
						@param primary_keys [in] primary key data is ertten into this object.
					*/
					delegate(size_t documents_in_collection, std::ostream &postings, std::ostream &primary_keys) :
						index_manager::delegate(documents_in_collection),
						postings_out(postings),
						primary_keys_out(primary_keys)
						{
						/* Nothing */
						}

					/*
						INDEX_MANAGER_SEQUENTIAL::DELEGATE::FINISH()
						--------------------------------------------
					*/
					/*!
						@brief Any final clean up.
					*/
					virtual void finish(void)
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
						@param document_frequency [in] The documment frequency of term (the length of document_ids and term_frequencies).
						@param document_ids [in] The array of document ids in which the term is found
						@param term_frequencies [in] The number of occurences of the term in each document id in document_ids
					*/
					virtual void operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
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
		protected:
			/*
				INDEX_MANAGER_SEQUENTIAL::MAKE_SPACE()
				--------------------------------------
			*/
			/*!
				@brief make sure all the internal buffers needed for iteration have been allocated
			*/
			void make_space(void)
				{
				auto new_temporary_size = get_highest_document_id() * (sizeof(*document_ids) / 7 + 1) * sizeof(*temporary);

				if (new_temporary_size > temporary_size)
					{
					temporary_size = new_temporary_size;
					/*
						we don't delete the old buffers because the memory object doesn't support allow us to do so
						but, this would only be needed if the called serialises then adds then serialises without
						deleting this object and newing another.
					*/
					document_ids = reinterpret_cast<decltype(document_ids)>(memory.malloc(get_highest_document_id() * sizeof(*document_ids)));
					term_frequencies = reinterpret_cast<decltype(term_frequencies)>(memory.malloc(get_highest_document_id() * sizeof(*term_frequencies)));
					temporary = reinterpret_cast<decltype(temporary)>(memory.malloc(temporary_size));
					}
				}

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
				primary_key(memory, 1000, 1.5),
				document_ids(nullptr),
				term_frequencies(nullptr),
				temporary_size(0),
				temporary(nullptr)
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
				@param document_primary_key [in] The document's primary key (or external document identifier).
			*/
			virtual void begin_document(const slice &document_primary_key)
				{
				/*
					Tell index_manager that we have started a new document
				*/
				index_manager::begin_document(document_primary_key);
				primary_key.push_back(slice(memory, document_primary_key));
				}


			/*
				INDEX_MANAGER_SEQUENTIAL::SET_PRIMARY_KEYS()
				--------------------------------------------
			*/
			/*!
				@brief Add a list of primary keys to the current list.  Normally used to set it without actually indexing (warning)
				@details Normally this method would only be called when an index is being "pushed" into an object
				rather than indexing document at a time.  This method actually adds to the end of the primary key list which
				is assumed to be empty before the method is called, but might not be if some indexing has already happened.
				@param keys [in] The vector of primary keys.
			*/
			virtual void set_primary_keys(const std::vector<slice> &keys)
				{
				for (auto key : keys)
					primary_key.push_back(key);
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
				index[term.lexeme].push_back(get_highest_document_id(), term.count);
				}

			/*
				INDEX_MANAGER_SEQUENTIAL::TERM()
				--------------------------------
			*/
			/*!
				@brief Hand a new term with a pre-computed postings list to this object.
				@param term [in] The term from the token stream.
				@param postings_list [in] The pre-computed D1-encoded postings list
			*/
			virtual void term(const parser::token &term, const std::vector<posting> &postings_list)
				{
				index[term.lexeme].push_back(postings_list);
				}

			/*
				INDEX_MANAGER_SEQUENTIAL::TERM()
				--------------------------------
			*/
			/*!
				@brief Hand a new term with a pre-computed postings list to this object.
				@param term [in] The term from the token stream.
				@param docid [in] The docid to shove on the end of of the list (with tf=1).
			*/
			virtual void term(const parser::token &term, compress_integer::integer docid)
				{
				index[term.lexeme].push_back(docid);
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
			virtual void iterate(index_manager::delegate &callback)
				{
				/*
					Make sure we have allocated the memory necessary for iteration (i.e. to linearize the postings lists).
				*/
				make_space();

				/*
					Iterate over the hash table calling the callback function with each term->postings pair.
				*/
				for (const auto &[key, value] : index)
					{
					auto document_frequency = value.linearize(temporary, temporary_size, document_ids, term_frequencies, get_highest_document_id());
					callback(key, value, document_frequency, document_ids, term_frequencies);
					}

				/*
					Iterate over the primary keys calling the callback function with each docid->key pair.
					Note that the search engine counts documents from 1, not from 0.
				*/
				size_t instance = 0;
				callback(instance, slice("-"));
				for (const auto &term : primary_key)
					callback(++instance, term);
				}

			/*
				INDEX_MANAGER_SEQUENTIAL::ITERATE()
				-----------------------------------
			*/
			/*!
				@brief Iterate over the index calling callback.operator() with each postings list.
				@param quantizer [in] The quantizer that will quantize then call the serialiser callback.
				@param callback [in] The callback that the quantizer should call.
			*/
			virtual void iterate(index_manager::quantizing_delegate &quantizer, index_manager::delegate &callback)
				{
				/*
					Make sure we have allocated the memory necessary for iteration (i.e. to linearize the postings lists).
				*/
				make_space();

				/*
					Iterate over the hash table calling the callback function with each term->postings pair.
				*/
				for (const auto &[term, postings] : index)
					{
					auto document_frequency = postings.linearize(temporary, temporary_size, document_ids, term_frequencies, get_highest_document_id());
					quantizer(callback, term, postings, document_frequency, document_ids, term_frequencies);
					}
					
				/*
					Iterate over the primary keys calling the callback function with each docid->key pair.
					Note that the search engine counts documents from 1, not from 0.
				*/
				size_t instance = 0;
				quantizer(callback, instance, slice("-"));
				for (const auto &term : primary_key)
					quantizer(callback, ++instance, term);
				}

			/*
				INDEX_MANAGER_SEQUENTIAL::UNITTEST_BUILD_INDEX()
				------------------------------------------------
				Build a index from the standard 10-document collection.
			*/
			/*!
				@brief Build and index for the 10 sample documents.  This is used by several unit tests that need a valid index.
				@param index [out] The index once built.
				@param document_collection [in] The documents to index.
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
					compress_integer::integer document_length = 0;
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
								document_length++;
								finished = true;						// finish up.
								break;
							case JASS::parser::token::alpha:
							case JASS::parser::token::numeric:
								document_length++;
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
					index.end_document(document_length);
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
					"6-><6,1>\n"
					"1-><1,1>\n"
					"4-><4,1>\n"
					"5-><5,1>\n"
					"3-><3,1>\n"
					"8-><8,1>\n"
					"7-><7,1>\n"
					"2-><2,1>\n"
					"9-><9,1>\n"
					"10-><10,1>\n"
					"four-><7,1><8,1><9,1><10,1>\n"
					"eight-><3,1><4,1><5,1><6,1><7,1><8,1><9,1><10,1>\n"
					"five-><6,1><7,1><8,1><9,1><10,1>\n"
					"seven-><4,1><5,1><6,1><7,1><8,1><9,1><10,1>\n"
					"two-><9,1><10,1>\n"
					"six-><5,1><6,1><7,1><8,1><9,1><10,1>\n"
					"three-><8,1><9,1><10,1>\n"
					"one-><10,1>\n"
					"nine-><2,1><3,1><4,1><5,1><6,1><7,1><8,1><9,1><10,1>\n"
					"ten-><1,1><2,1><3,1><4,1><5,1><6,1><7,1><8,1><9,1><10,1>\n"
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
				delegate callback(10, postings_result, primary_key_result);
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
