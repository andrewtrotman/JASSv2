/*
	GLOBAL_NEW_DELETE.H
	-------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Replacement global new, new[], delete and delete[] operators.  NOTE: This file can only be included once per executable.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#include <stdlib.h>

/*!
	@var replace
	@brief If replace is true then the global new and delete operators are overridden with versions that abort() when called.
*/
static bool replace = false;

/*
	GLOBAL_NEW_DELETE_REPLACE()
	---------------------------
*/
/*!
	@brief Replace the global new and delete operators with versions that cause the program to terminate.
*/
void global_new_delete_replace(void)
	{
	replace = true;
	}

/*
	GLOBAL_NEW_DELETE_RETURN()
	--------------------------
*/
/*!
	@brief Return to using the global new and delete operators from the standard C++ library.
*/
void global_new_delete_return(void)
	{
	replace = false;
	}

/*
	OPERATOR NEW()
	--------------
*/
/*!
	@brief Replacement verison of new.
	@param bytes [in] Number of bytes to allocate.
	@return Space as requested.
*/
void *operator new(size_t bytes) throw(std::bad_alloc)
	{
	if (replace)
		abort();
	else
		return ::malloc(bytes);
	}

/*
	OPERATOR DELETE()
	-----------------
*/
/*!
	@brief Replacement verison of delete.
	@param pointer [in] Pointer to memory to free up.
*/
void operator delete(void *pointer) throw()
	{
	if (replace)
		abort();
	else
		free(pointer);
	}

/*
	OPERATOR NEW[]()
	----------------
*/
/*!
	@brief Replacement verison of new[].
	@param bytes [in] Number of bytes to allocate.
	@return Space as requested.
*/
void *operator new[](size_t bytes) throw(std::bad_alloc)
	{
	if (replace)
		abort();
	else
		return ::malloc(bytes);
	}

/*
	OPERATOR DELETE[]()
	-------------------
*/
/*!
	@brief Replacement verison of delete [].
	@param pointer [in] Pointer to memory to free up.
*/
void operator delete[](void *pointer) throw()
	{
	if (replace)
		abort();
	else
		free(pointer);
	}
