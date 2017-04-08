/*
	SERIALISE_CI.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
 */
/*!
	@file
	@brief Serialise an index in source code
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
 */
#pragma once

#include "file.h"
#include "index_manager.h"

namespace JASS
	{
	class serialise_ci : public index_manager::delegate
		{
		private:
			file postings_file;
			file postings_header_file;
			file vocab_file;
			uint64_t terms;

		public:
			serialise_ci();
			~serialise_ci();

			virtual void operator()(const slice &term, const index_postings &postings);

			static void unittest(void);
		};
	}
