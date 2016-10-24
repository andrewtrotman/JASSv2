/*
	INDEX_MANAGER_SEQUENTIAL.H
	--------------------------
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
	class index_manager_sequential : public index_manager
		{
		private:
			allocator_pool memory;
			hash_table<slice, index_postings, 24> index;
			
		public:
			index_manager_sequential() :
				index_manager(),
				index(memory)
				{
				/*
					Nothing.
				*/
				}
			
			virtual ~index_manager_sequential()
				{
				/*
					Nothing.
				*/
				}
			
			virtual void begin_document(void)
				{
				index_manager::begin_document();
				}
			
			virtual void term(const parser::token &term)
				{
				index_manager::term(term);
				index[term.lexeme].push_back(get_highest_document_id(), get_highest_term_id());
				}
			
			virtual void end_document(void)
				{
				index_manager::end_document();
				}
			
			void serialise(void)
				{
				std::cout << index;
				}
			
			static void unittest(void)
				{
				parser parser;
				document document;
				instream *file = new instream_memory(unittest_data::ten_documents.c_str(), unittest_data::ten_documents.size());
				instream_document_trec source(*file);
				index_manager_sequential index;
				
				do
					{
					document.rewind();
					source.read(document);
					if (document.isempty())
						break;
					parser.set_document(document);
					
					bool finished = false;
					do
						{
						const auto &token = parser.get_next_token();
						
						switch (token.type)
							{
							case JASS::parser::token::eof:
								finished = true;
								break;
							case JASS::parser::token::alpha:
							case JASS::parser::token::numeric:
								index.term(token);
								break;
							case JASS::parser::token::xml_start_tag:
								if (token.lexeme.size() == 3 && strncmp((char *)token.lexeme.address(), "DOC", 3) == 0)
									index.begin_document();
								break;
							case JASS::parser::token::xml_end_tag:
								if (token.lexeme.size() == 3 && strncmp((char *)token.lexeme.address(), "DOC", 3) == 0)
									index.end_document();
								break;
							default:
								break;
							}
						}
					while (!finished);
					}
				while (!document.isempty());
				
				index.serialise();
				
				puts("index_manager_sequential::PASSED");
				}
		};
	}
