/*
	INSTREAM_DEFLATE.H
	------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Subclass of instream for reading from .gz files (files compressed with deflate).
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#pragma once

#include "zlib.h"
#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DEFLATE
		----------------------
	*/
	/*!
		@brief Subclass of instream for reading from .gz files (files compressed with deflate).
	*/
	class instream_deflate : public instream
		{
		private:
			z_stream stream;														///< The Zlib stream processing data structure
			uint64_t bytes_read;													///< How many bytes of data have been decoded from the stream
			uint8_t *buffer;														///< Internal deflation buffer
			static const size_t buffer_length = (10 * 1024 * 1024);	///< size of the internal buffer

		public:
			/*
				INSTREAM_DEFLATE::INSTREAM_DEFLATE()
				------------------------------------
			*/
			/*!
				@brief Constructor
				@param filename [in] The name of the file to use as the input stream
			*/
			instream_deflate(std::shared_ptr<instream> &source);

			/*
				INSTREAM_DEFLATE::~INSTREAM_DEFLATE()
				-------------------------------------
			*/
			/*!
				@brief Destructor.
			*/
			virtual ~instream_deflate()
				{
				/* Nothing */
				}

			/*
				INSTREAM_DEFLATE::READ()
				------------------------
			*/
			/*!
				@brief Read buffer.contents.size() bytes of data into buffer.contents, resizing on eof.
				@param buffer [out] buffer.contents.size() bytes of data are read from source into buffer which is resized to the number of bytes read on eof.
			*/
			virtual void read(document &buffer);

			/*
				INSTREAM_DEFLATE::UNITTEST()
				----------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
}
