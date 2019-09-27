/*
	INSTREAM_DOCUMENT_FASTA.H
	-------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Child class of instream for creating documents FASTA genome sequence files.
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/

#pragma once

#include <stdint.h>

#include <string>

#include "slice.h"
#include "instream.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DOCUMENT_FASTA
		-----------------------------
	*/
	/*!
		@brief Child class of instream for creating documents from FASTA genome sequence files.
		@details See the Wikipedia article on the FASTA file format here: https://en.wikipedia.org/wiki/FASTA_format
	*/
	class instream_document_fasta : public instream
		{
		protected:
			size_t buffer_size;								///< Size of the disk read buffer.  Normally 16MB

		protected:
			uint8_t *buffer;									///< Pointer to the interal buffer from which documents are extracted.  Filled by calling source.read()
			uint8_t *buffer_end;								///< Pointer to the end of the buffer (used to prevent read past EOF).
			size_t buffer_used;								///< The number of bytes of buffer that have already been used from buffer (buffer + buffer_used is a pointer to the unused data in buffer)

		protected:
			/*
				INSTREAM_DOCUMENT_FASTA::INSTREAM_DOCUMENT_FASTA()
				--------------------------------------------------
			*/
			/*!
				@brief Protected constructor used to set the size of the internal buffer in the unittest.
				@param source [in] The instream responsible for providing data to this class.
				@param buffer_size [in] The size of the internal buffer filled from source.
			*/
			instream_document_fasta(std::shared_ptr<instream> &source, size_t buffer_size);
			
			/*
				INSTREAM_DOCUMENT_FASTA::FETCH()
				--------------------------------
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
				INSTREAM_DOCUMENT_FASTA::INSTREAM_DOCUMENT_FASTA()
				--------------------------------------------------
			*/
			/*!
				@brief Copy constructor (not available).
				@param previous [in] The instance to copy.
			*/
			instream_document_fasta(const instream_document_fasta &previous) = delete;

			/*
				INSTREAM_DOCUMENT_FASTA::INSTREAM_DOCUMENT_FASTA()
				--------------------------------------------------
			*/
			/*!
				@brief Constructor
				@param source [in] The instream responsible for providing data to this class.
			*/
			instream_document_fasta(std::shared_ptr<instream> &source);
			
			/*
				INSTREAM_DOCUMENT_FASTA::INSTREAM_DOCUMENT_FASTA()
				--------------------------------------------------
			*/
			/*!
				@brief Destructor
			*/
			virtual ~instream_document_fasta();

			/*
				INSTREAM_DOCUMENT_FASTA::READ()
				-------------------------------
			*/
			/*!
				@brief Read the next document from the source instream into document.
				@param buffer [out] The next document in the source instream.
			*/
			virtual void read(document &buffer);
			
			/*
				INSTREAM_DOCUMENT_FASTA::UNITTEST()
				-----------------------------------
			*/
			/*!
				@brief Unit test this class
			*/
			static void unittest(void);
		} ;
}
