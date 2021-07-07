/*
	INSTREAM_FILE.H
	---------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Subclass of instream for reading data from a disk file.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include "file.h"
#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_FILE
		-------------------
	*/
	/*!
		@brief Subclass of the instream base class used for reading data from a disk file.
	*/
	class instream_file : public instream
		{
		protected:
			file disk_file;			///< The file to read from.
			size_t file_length;		///< The length of the file.
			size_t bytes_read;		///< The number of bytes that have been read from the file.

		protected:
			/*
				INSTREAM_FILE::INSTREAM_FILE()
				------------------------------
			*/
			/*!
				@brief Constructor
				@param file [in] the FILE * object that this instream_file object should use (see instream_file_star.h)
			*/
			instream_file(FILE *file);

		public:
			/*
				INSTREAM_FILE::INSTREAM_FILE()
				------------------------------
			*/
			/*!
				@brief Constructor
				@param filename [in] The name of the file to use as the input stream
			*/
			instream_file(const std::string &filename);

			/*
				INSTREAM_FILE::~INSTREAM_FILE()
				-------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~instream_file()
				{
				/* Nothing */
				}

			/*
				INSTREAM_FILE::READ()
				---------------------
			*/
			/*!
				@brief Read buffer.contents.size() bytes of data into buffer.contents, resizing on eof.
				@param buffer [out] buffer.contents.size() bytes of data are read from source into buffer which is resized to the number of bytes read on eof.
			*/
			virtual void read(document &buffer);
			
			/*
				INSTREAM_FILE::UNITTEST()
				-------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
}
