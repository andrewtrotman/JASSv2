/*
	QUERY16_T.H
	-----------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief a query type that uses uint16_t for an accumulator (see query.h).
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <iostream>

#include "query.h"

namespace JASS
	{
	/*
		TYPEDEF QUERY16_T
		-----------------
	*/
	/*!
		@typedef query16_t
		@brief a query type that uses uint16_t for an accumulator (see query.h).
	*/
	typedef query<uint16_t> query16_t;
	}
