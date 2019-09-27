/*
	INSTREAM_DOCUMENT_TREC.H
	------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Child class of instream for creating documents from TREC pre-web (i.e. news articles) data.
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include <stdint.h>

#include <string>

#include "slice.h"
#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DOCUMENT_TREC
		----------------------------
	*/
	/*!
		@brief Child class of instream for creating documents from TREC pre-web (i.e. news articles) data.
		@details Connect an object of this class to an input stream and it will return TREC new-article formatted documents
		one per read.  This is done by looking for \<DOC> and \</DOC> tags in the source stream.  Document primary keys are
		assumed to be between \<DOCNO> and \</DOCNO> tags.
	*/
	class instream_document_trec : public instream
		{
		protected:
			size_t buffer_size;								///< Size of the disk read buffer.  Normally 16MB

		protected:
			uint8_t *buffer;									///< Pointer to the interal buffer from which documents are extracted.  Filled by calling source.read()
			uint8_t *buffer_end;								///< Pointer to the end of the buffer (used to prevent read past EOF).
			size_t buffer_used;								///< The number of bytes of buffer that have already been used from buffer (buffer + buffer_used is a pointer to the unused data in buffer)
		
			std::string document_start_tag;				///< The start tag used to delineate documents ("<DOC>" be default)
			std::string document_end_tag;					///< The end tag used to mark the end of a document ("</DOC>" by defaut)
			std::string primary_key_start_tag;			///< The primary key's start tag ("<DOCNO>" by default)
			std::string primary_key_end_tag;				///< The primary key's end tag ("</DOCNO>" by default)

		protected:
			/*
				INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
				------------------------------------------------
			*/
			/*!
				@brief Protected constructor used to set the size of the internal buffer in the unittest.
				@param source [in] The instream responsible for providing data to this class.
				@param buffer_size [in] The size of the internal buffer filled from source.
				@param document_tag [in] The name of the tag used to delineate docments.
				@param document_primary_key_tag [in] The name of the element that contans the document's primary key.
			*/
			instream_document_trec(std::shared_ptr<instream> &source, size_t buffer_size, const std::string &document_tag, const std::string &document_primary_key_tag);
			
			/*
				INSTREAM_DOCUMENT_TREC::SET_TAGS()
				----------------------------------
			*/
			/*!
				@brief Register the document tag and the primary key tag.  Used to set up internal data structures.
				@param document_tag [in] The name of the tag used to delineate docments.
				@param primary_key_tag [in] The name of the element that contans the document's primary key.
			*/
			void set_tags(const std::string &document_tag, const std::string &primary_key_tag);

			/*
				INSTREAM_DOCUMENT_TREC::FETCH()
				-------------------------------
			*/
			/*!
				@brief Fetch another block of data from the source.
				@param buffer [out] Write bytes amount of data into this memory location.
				@param bytes [in] Read this amount of data from the source.
			*/
			void fetch(void *buffer, size_t bytes)
				{
				buffer_end = (uint8_t *)buffer + source->fetch(buffer, bytes);
				}

		public:
			/*
				INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
				------------------------------------------------
			*/
			/*!
				@brief Copy constructor (not available).
				@param previous [in] The instance to copy.
			*/
			instream_document_trec(const instream_document_trec &previous) = delete;

			/*
				INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
				------------------------------------------------
			*/
			/*!
				@brief Constructor
				@param source [in] The instream responsible for providing data to this class.
				@param document_tag [in] The name of the tag used to delineate docments (default = "DOC").
				@param document_primary_key_tag [in] The name of the element that contans the document's primary key (default = "DOCNO").
			*/
			instream_document_trec(std::shared_ptr<instream> &source, const std::string &document_tag = "DOC", const std::string &document_primary_key_tag = "DOCNO");
			
			/*
				INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
				------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~instream_document_trec();

			/*
				INSTREAM_DOCUMENT_TREC::READ()
				------------------------------
			*/
			/*!
				@brief Read the next document from the source instream into document.
				@param buffer [out] The next document in the source instream.
			*/
			virtual void read(document &buffer);
			
			/*
				INSTREAM_DOCUMENT_TREC::UNITTEST()
				----------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
}
