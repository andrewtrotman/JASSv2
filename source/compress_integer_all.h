/*
	COMPRESS_INTEGER_ALL.H
	----------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief A data structure representing all the integer compression schemes known by JASS
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <string>
#include "compress_integer.h"

namespace JASS
	{
	class compress_integer_all
		{
		public:
			std::string shortname;
			std::string longname;
			std::string description;
			compress_integer *codex;
		};
	}
