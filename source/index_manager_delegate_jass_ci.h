/*
	INDEX_MANAGER_DELEGATE_JASS_CI.H
	--------------------------------
*/
#pragma once

#include "slice.h"
#include "index_manager.h"


namespace JASS
	{
	class index_manager_delegate_jass_ci : public index_manager::delegate
		{
		public:
			virtual void operator()(const slice &term, const void *postings)
				{
				std::cout << term << '\n';
				}
				
			static void unittest(void)
				{
				index_manager_delegate_jass_ci delegate;
				delegate(slice("TERM"), "dunno");
				}
		};
	}