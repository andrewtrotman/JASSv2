/*
	FORCEINLINE.H
	-------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief operating system and compiler independant definition of forceinline
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

/*!
	@brief tell the compuler that is should inline a function despite its better judgement.
*/
#ifdef __APPLE__
	#define forceinline __attribute__((always_inline)) inline
#elif defined(__GNUC__)
	#define forceinline __attribute__((always_inline)) inline
#elif defined(_MSC_VER)
	#define forceinline __forceinline
#else
	#define forceinline inline
#endif
