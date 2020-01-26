/*
	STEM_PORTER.H
	-------------
*/
/*!
	@file
	@brief Porter's stemer
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include <stdio.h>

#include "stem.h"
#include "ascii.h"

#define MAX_TERM_LENGTH 1024

namespace JASS
	{
	/*
		CLASS STEM_PORTER
		-----------------
	*/
	/*!
		@brief Generate the stem of a word using Porter's algorithm (version 1)
		@details see:
			M.F. Porter, An algoritm for suffix stripping, Program, Vol 14, No 3, pp 130-137, July 1980
			This code came from ATIRE, and has a few bug fixes applied (especially to rule 1)
	*/
	class stem_porter : public stem
		{
		private:
			std::string workspace;		// temporary workspace used to reverse the string.
			
		private:
			/*
				STEM_PORTER::ISVOWELY()
				-----------------------
			*/
			/*!
				@brief Is the character at this point a vowel according to Porter's definision
				@details
				Porter states:
				"A consonant in a word is a letter other than A, E, I, O or U, and other than Y preceded by a consonant."
				...
				"If a letter is not a consonant it is a vowel."
				so a vowely is {aeiou} or {y} not folowed by a {aeiou}
				@param c [in] Pointer to the character to check.
				@return true if a vowely or false otherwise.
			*/
			bool isvowely(const char *c)
				{
				return *c != '\0' && (ascii::isvowel(*c) || (*c == 'y' && !ascii::isvowel(*(c + 1))));
				}

			/*
				STEM_PORTER::CVC()
				------------------
				the stem ends cvc, where the second c is not W, X or Y (e.g. -WIL, -HOP).
			*/
			/*!
				@brief Porter's "*o" rule, the stem ends cvc, where the second c is not W, X or Y (e.g. -WIL, -HOP).
				@param what [in] where to check
				@return true or false.
			*/
			bool cvc(const char *what)
				{
				return ((strchr("aeiouwxy", *what) == NULL) && (isvowely(what + 1)) && (!isvowely(what + 2))) ? true : false;
				}

			/*
				STEM_PORTER::LENGTH()
				---------------------
			*/
			/*!
				@brief Return Porter's m in [C](VC)m[V], the length of the stem
				@param reversed [in] The string to count m on.
				@return The length of the stem in units of m
			*/
			size_t length(const char *reversed);

			/*
				STEM_PORTER::HAS_VOWEL()
				------------------------
			*/
			/*!
				@brief Does the string contain a vowel?
				@param what [in] Where to check.
				@return true if the string contains a vowel, false otherwise.
			*/
			bool has_vowel(const char *what);

		public:
			/*
				STEM_PORTER::~STEM_PORTER()
				---------------------------
			*/
			/*!
				@brief Detructor
			*/
			virtual ~stem_porter()
				{
				/* Nothing */
				}

			/*
			  STEM_PORTER::NAME()
			  -------------------
			*/
			/*!
			  @brief Return the name of the stemming algorithm
			  @return The name of the stemmer
			*/
			virtual std::string name(void)
				{
				return "Porter";
				}

			/*
				STEM_PORTER::TOSTEM()
				---------------------
			*/
			/*!
				@brief Stem from source into destination
				@param destination [out] the result of the steming process (the stem)
				@param source [in] the term to stem
				@param source_length [in] the length of the string to stem
				@details source and destination can be the same.
				@return the length of the stem
			*/
			using stem::tostem;
			virtual size_t tostem(char *destination, const char *source, size_t source_length);

			/*
				STEM_PORTER::UNITTEST()
				-----------------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void);
		} ;

	}
