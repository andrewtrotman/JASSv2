/*
	INSTREAM_DIRECTORY_ITERATOR.H
	-----------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Subclass of instream for reading data from multiple files in a directory (as if they were all concatinated).
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include <filesystem>

#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DIRECTORY_ITERATOR
		---------------------------------
	*/
	/*!
		@brief Subclass of instream for reading data from multiple files in a directory (as if they were all concatinated).
	*/
	class instream_directory_iterator : public instream
		{
		protected:
			std::filesystem::directory_iterator source;		///< Use the C++17 directory iterator
			std::filesystem::directory_iterator current;		///< Where we are in the current iteration
			std::filesystem::directory_iterator last;			///< The point where the iterator finishes iterating

			std::shared_ptr<instream> reader;					///< A chain of instram objects responsible for sourcing the data
			size_t bytes_read;										///< The number of bytes that have been read from the file.

		public:
			/*
				INSTREAM_DIRECTORY_ITERATOR::INSTREAM_DIRECTORY_ITERATOR()
				----------------------------------------------------------
			*/
			/*!
				@brief Constructor
				@param directory_name [in] The name of the directory to search within
			*/
			instream_directory_iterator(const std::string &directory_name);

			/*
				INSTREAM_DIRECTORY_ITERATOR::~INSTREAM_DIRECTORY_ITERATOR()
				-----------------------------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~instream_directory_iterator()
				{
				/* Nothing */
				}

			/*
				INSTREAM_DIRECTORY_ITERATOR::READ()
				-----------------------------------
			*/
			/*!
				@brief Read buffer.contents.size() bytes of data into buffer.contents, resizing on eof.
				@param buffer [out] buffer.contents.size() bytes of data are read from source into buffer which is resized to the number of bytes read on eof.
			*/
			virtual void read(document &buffer);

			/*
				INSTREAM_DIRECTORY_ITERATOR::UNITTEST()
				---------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
}

