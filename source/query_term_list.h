/*
	QUERY_TERM_LIST.H
	-----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A list of query terms (i.e. a query)
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include "query_term.h"
#include "dynamic_array.h"

namespace JASS
	{
	/*
		TYPEDEF QUERY_TERM_LIST
		-----------------------
	*/
	/*!
		@typedef query_term_list
		@brief A list of query tokens (i.e. a query).
	*/
	typedef dynamic_array<query_term> query_term_list;
	}
