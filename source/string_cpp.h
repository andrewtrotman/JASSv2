/*
	STRING_CPP.H
	------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief String type that uses a custom allocator.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#include <string>

#include "allocator_cpp.h"

namespace JASS
	{
	/*!
		@typedef string
		@brief A JASS string that uses a JASS allocator_cpp for storage
	*/
	typedef std::basic_string<char, std::char_traits<char>, allocator_cpp<char>>string;
	}
