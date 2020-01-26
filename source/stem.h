/*
	STEM.H
	------
*/
/*!
	@file
	@brief Baseclass for stemmers
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#pragma once

#include <string>

#include "parser.h"

namespace JASS
	{
	/*
		CLASS STEM
		----------
	*/
	/*!
		@brief Baseclass for stemming algorithms
	*/
	class stem
		{
		public:
			/*
				STEM::STEM()
				------------
			*/
			/*!
				@brief Constructor
			*/
			stem()
				{
				/* Nothing */
				}

			/*
				STEM::~STEM()
				------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~stem()
				{
				/* Nothing */
				}

			 /*
			 	STEM::NAME()
			 	------------
			 */
			 /*!
			 	@brief Return the name of the stemming algorithm
			 	@return The name of the stemmer
			 */
			 virtual std::string name(void) = 0;

			/*
				STEM::TOSTEM()
				--------------
			*/
			/*!
				@brief Stem from source into destination
				@param destination [out] the result of the steming process (the stem)
				@param source [in] the term to stem
				@param source_length [in] the length of the string to stem
				@return the length of the stem
			*/
			virtual size_t tostem(char *destination, const char *source, size_t source_length) = 0;

			/*
				STEM::TOSTEM()
				--------------
			*/
			/*!
				@brief Stem from term into destination
				@param destination [out] the result of the steming process (the stem).
				@param source [in] the term to stem
				@details source and destination can be the same.
				@return the length of the stem
			*/
			virtual size_t tostem(parser::token &destination, const parser::token &source)
				{
				size_t length = tostem(reinterpret_cast<char *>(destination.buffer), reinterpret_cast<char *>(source.lexeme.address()), source.lexeme.size());
				destination.lexeme = slice(destination.buffer, length);
				return length;
				}

			/*
				STEM::UNITTEST()
				----------------
			*/
			/*!
				@brief Unit test this class.
			*/
			static void unittest(void);

		} ;
	}
