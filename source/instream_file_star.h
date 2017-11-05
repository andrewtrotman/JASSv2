/*
	INSTREAM_FILE_STAR.H
	--------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Subclass of instream_file for construction using a FILE *
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <stdio.h>

#include "instream_file.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_FILE_STAR
		------------------------
	*/
	/*!
		@brief Subclass of the instream_file class used for reading data from a disk file that already exists (e.g. stdin)
	*/
	class instream_file_star : public instream_file
		{
		public:
			/*
				INSTREAM_FILE_STAR::INSTREAM_FILE_STAR()
				----------------------------------------
			*/
			/*!
				@brief Constructor
				@param file [in] The FILE * object to use as an instrean.
			*/
			instream_file_star(FILE *file) :
				instream_file(file)
				{
				/* Nothing */
				}

			/*
				INSTREAM_FILE_STAR::~INSTREAM_FILE_STAR()
				-----------------------------------------
			*/
			/*!
				@brief Destructior
			*/
			virtual ~instream_file_star()
				{
				/* Nothing */
				}

			/*
				INSTREAM_FILE_STAR::UNITTEST()
				------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void)
				{
				/*
					Example test string to read / write
				*/
				const char example[] = "123456789012345678901234567890";			// sample to be written and read back

				/*
					create the file  Note that ::tmpfile() cannot be calle because its insecure (according to Coverity)
				*/
				auto filename = file::mkstemp("jass");
				FILE *fp = ::fopen(filename.c_str(), "w+b");
				(void)::remove(filename.c_str());								// delete the file once we're done with it (cast to void to remove Coverity warning)

				/*
					write to the file and rewind to the start
				*/
				::fwrite(example, sizeof(example), 1, fp);
				::fflush(fp);
				::rewind(fp);

				/*
					create an instream_file_star and test it.
				*/
				instream_file_star reader(fp);
				document document;
				document.contents = slice(document.contents_allocator, 30);
			
				/*
					read from it making sure we got what we should have.
				*/
				reader.read(document);
				JASS_assert(document.contents.size() == 30);
				for (size_t index = 0; index < document.contents.size(); index++)
					JASS_assert(document.contents[index] == example[index]);
			
				/*
					Yay, we passed
				*/
				puts("instream_file_star::PASSED");
				}
	};
}
