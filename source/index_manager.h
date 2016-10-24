/*
	INDEX_MANAGER.H
	---------------
*/
#pragma once

#include "parser.h"

namespace JASS
	{
	class index_manager
		{
		private:
			size_t highest_document_id;
			size_t highest_term_id;
			
		public:
			index_manager() :
				highest_document_id(0),
				highest_term_id(0)
				{
				/*
					Nothing.
				*/
				}
			
			virtual ~index_manager()
				{
				/*
					Nothing.
				*/
				}
			
			virtual void begin_document(void)
				{
				highest_document_id++;
				}
			
			virtual void term(const parser::token &term)
				{
				highest_term_id++;
				}
			
			virtual void end_document(void)
				{
				/*
					Nothing.
				*/
				}
			
			size_t get_highest_term_id(void) const
				{
				return highest_term_id;
				}
			
			size_t get_highest_document_id(void) const
				{
				return highest_document_id;
				}
			
			static void unittest(void)
				{
				index_manager index;
				
				JASS_assert(index.get_highest_term_id() == 0);
				JASS_assert(index.get_highest_document_id() == 0);
				
				parser::token token;
				index.begin_document();
				index.term(token);
				index.end_document();

				JASS_assert(index.get_highest_term_id() == 1);
				JASS_assert(index.get_highest_document_id() == 1);
				puts("index_manager::PASSED");
				}
		};
	}
