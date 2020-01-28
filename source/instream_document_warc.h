/*
	INSTREAM_DOCUMENT_WARC.H
	------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Child class of instream for creating documents from TREC WARC files.
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/

#pragma once

#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DOCUMENT_WARC
		----------------------------
	*/
	/*!
		@brief Extract documents from a WARC archive
	*/
	class instream_document_warc : public instream
		{
		private:
			static constexpr size_t WARC_BUFFER_SIZE = 8 * 1024;		///< The internal buffer used to read lines one at a time
			
		private:
			std::string buffer;													///< An internal buffer used to store lines
			
		private:
			/*
				INSTREAM_DOCUMENT_WARC::FIND_STRING()
				-------------------------------------
			*/
			/*!
				@brief read lines from the WARC file until one starting with string is found.
				@param string [in] the string to look for (at the start of a line)
				@return a pointer to the remainder of the string
			*/
			const char *find_string(const std::string &string);

		public:
			/*
				INSTREAM_DOCUMENT_WARC::INSTREAM_DOCUMENT_WARC()
				------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			instream_document_warc(std::shared_ptr<instream> &source) :
				instream(source)
				{
				buffer.resize(WARC_BUFFER_SIZE);
				}

			/*
				INSTREAM_DOCUMENT_WARC::~INSTREAM_DOCUMENT_WARC()
				-------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~instream_document_warc()
				{
				/* Nothing */
				}

			/*
				INSTREAM_DOCUMENT_WARC::READ()
				------------------------------
			*/
			/*!
				@brief Read the next document from the source instream into document.
				@param buffer [out] The next document in the source instream.
			*/
			virtual void read(document &buffer);

			/*
				INSTREAM_DOCUMENT_WARC::UNITTEST()
				----------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
