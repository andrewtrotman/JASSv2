/*
	CHANNEL_FILE.H
	--------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Input and output channel for disk files and stdin / stdout.
	@author Andrew Trotman
	@copyright 2017 Andrew Trotman
*/
#pragma once

#include <stdio.h>

#include  <memory>

#include "file.h"
#include "channel.h"
#include "instream_file.h"

namespace JASS
	{
	/*
		CLASS CHANNEL_FILE
		------------------
	*/
	/*!
		@brief Input and output channel for disk files and stdin / stdout.
	*/
	class channel_file : public channel
		{
		private:
			std::string filename;						///< the name of the file being used
			std::unique_ptr<instream> infile;		///< the input stream
			FILE *outfile;									///< the output stream (uses delayed opening)
			bool eof;										///< are we at eof of the inoput stream?

		protected:

			/*
				CHANNEL_FILE::BLOCK_WRITE()
				---------------------------
			*/
			/*!
				@brief All output happens via the block_write method.
				@param buffer [in] write length number of bytes from buffer
				@param length [in] The number of bytes go write.
				@return The number of bytes written (usually equal to length).
			*/
			virtual size_t block_write(const void *buffer, size_t length);

			/*
				CHANNEL_FILE::BLOCK_READ()
				--------------------------
			*/
			/*!
				@brief All input happens via the block_read method.
				@param into [out] length number of bytes are written into into
				@param length [in] The number of bytes to read.
				@return The number of buyes read (usually equal to length).
			*/
			virtual size_t block_read(void *into, size_t length);

		public:
			/*
				CHANNEL_FILE::CHANNEL_FILE()
				----------------------------
			*/
			/*!
				@brief Constructor using stdin and stdout as the files.
			*/
			channel_file();

			/*
				CHANNEL_FILE::CHANNEL_FILE()
				----------------------------
			*/
			/*!
				@brief Constructor using the named file as the two streams.
				@param filename [in] The nane of the file.
			*/
			channel_file(const std::string &filename);

			/*
				CHANNEL_FILE::~CHANNEL_FILE()
				-----------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~channel_file();

			/*
				CHANNEL_FILE::UNITTEST()
				------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
