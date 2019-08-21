/*
	ASSERT.H
	--------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief replacement for the C runtime library assert that also works in release.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <stdlib.h>

/*
	JASS_ASSERT()
	-------------
*/
/*!
	@brief Drop in replacement for assert() that aborts in Release as well as Debug.
	@details Each of the JASS classes has a unit test function with the signature static void unittest(void).  These unit tests
	call assert() if something isn't as expected.  However, as assert compiles to an empty function on release, there is no
	obvious way to make the unit tests also work in release.  To get around this, call JASS_assert() rather than assert() and
	the assert will also work in release.
	
	If you want a genuine assert() that goes away in release, then call assert().  If you want an assert() for unittests call
	JASS_assert().
	
	@param expression [in] The expression to check.
*/
#define JASS_assert(expression) ((expression) ? ((void)0) : JASS::fail(__func__, __FILE__, __LINE__, #expression))

namespace JASS
	{
	/*
		FAIL()
		------
	*/
	/*!
		@brief Helper function to JASS_assert().  Prints a message and calls abort().
		@param function [in] The name of the function in which the JASS assert triggered.
		@param file [in] The name of the file in which the JASS assert triggered.
		@param line [in] The line number of the file in which the JASS assert triggered.
		@param expression [in] The expression that caused assert to trigger.
	*/
	void fail(const char *function, const char *file, size_t line, const char *expression);
	}
