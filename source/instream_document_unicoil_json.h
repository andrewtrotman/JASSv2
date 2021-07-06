/*
	INSTREAM_DOCUMENT_UNICOIL_JSON.H
	--------------------------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Child class of instream for creating documents from the UniCOIL data in JSON format.
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

	@details A document (1 per line) looks like this:

	{"id": "0", "contents": "The presence of communication amid scientific minds was equally important to the success of the Manhattan Project as scientific intellect was.
	The only cloud hanging over the impressive achievement of the atomic researchers and engineers is what their success truly meant; hundreds of thousands of innocent lives
	obliterated.", "vector": {"the": 94, "presence": 102, "of": 80, "communication": 147, "amid": 124, "scientific": 134, "minds": 108, "was": 71, "equally": 106,
	"important": 142, "to": 59, "success": 146, "manhattan": 162, "project": 116, "as": 34, "intellect": 135, ".": 63, "only": 98, "cloud": 117, "hanging": 80, "over": 62,
	"impressive": 112, "achievement": 112, "atomic": 149, "researchers": 94, "and": 48, "engineers": 101, "is": 35, "what": 82, "their": 78, "truly": 89, "meant": 84, ";": 58,
	"hundreds": 100, "thousands": 101, "innocent": 101, "lives": 80, "ob": 103, "##lite": 88, "##rated": 96, "[SEP]": 0, "amongst": 36, "scientists": 126, "why": 54, "?": 50,
	"about": 25, "so": 53, "a": 29, "importance": 95, "purpose": 45, "how": 27, "significant": 61, "in": 36, "for": 34, "believe": 49, "who": 42, "did": 68}}

	Where "id" is the primary key, "contents" is the document, and "vector" is the forward index.  In the forward index, {"importance": 95} means the term "importance" has an
	impact score of 95.
*/
#pragma once

#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DOCUMENT_UNICOIL_JSON
		------------------------------------
	*/
	/*!
		@brief Extract documents from a JSON formatted UniCOIL archive
	*/
	class instream_document_unicoil_json : public instream
		{
		private:
			size_t buffer_size;					///< Current size of the buffer
			std::string buffer;					///< An internal buffer used to store lines
			const char *buffer_start;					///< Pointer to the current working point in the buffer;
			const char *buffer_end;						///< Pointer to the end of the buffer

		public:
			/*
				INSTREAM_DOCUMENT_UNICOIL_JSON::INSTREAM_DOCUMENT_UNICOIL_JSON()
				----------------------------------------------------------------
			*/
			/*!
				@brief Constructor
			*/
			instream_document_unicoil_json(std::shared_ptr<instream> &source) :
				instream(source)
				{
				buffer_size = 1024 * 1024;
				buffer.resize(buffer_size);
				buffer_start = &buffer[0];
				size_t bytes = source->fetch(&buffer[0], buffer_size);
				buffer_end = buffer_start + bytes;
				}

			/*
				INSTREAM_DOCUMENT_UNICOIL_JSON::~INSTREAM_DOCUMENT_UNICOIL_JSON()
				-----------------------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~instream_document_unicoil_json()
				{
				/* Nothing */
				}

			/*
				INSTREAM_DOCUMENT_UNICOIL_JSON::READ()
				--------------------------------------
			*/
			/*!
				@brief Read the next document from the source instream into document.
				@param buffer [out] The next document in the source instream.
			*/
			virtual void read(document &buffer);

			/*
				INSTREAM_DOCUMENT_UNICOIL_JSON::UNITTEST()
				------------------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		};
	}
