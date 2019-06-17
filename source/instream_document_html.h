/*
	INSTREAM_DOCUMENT_HTML.H
	------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief Child class of instream for creating documents from files of html documents
	@author Andrew Trotman and Kat Lilly
	@copyright 2016 Andrew Trotman
*/

#pragma once

#include "instream_document_trec.h"

namespace JASS
	{
	/*
		CLASS INSTREAM_DOCUMENT_HTML
		----------------------------
	*/
	/*!
		@brief Child class of instream_document_trec for creating documents from html data.
		@details Document are found by looking for \<html  and \</html> tags in the source stream.  Document primary keys are
		assumed to be between \<title> and \</title> tags.
	*/
	class instream_document_html : public instream_document_trec
		{
		public :
			instream_document_html() = delete;

		/*
			INSTREAM_DOCUMENT_HTML::INSTREAM_DOCUMENT_HTML()
			------------------------------------------------
		*/
		/*!
			@brief Constructor
			@param source [in] The instream responsible for providing data to this class.
			@param document_tag [in] The name of the tag used to delineate docments (default = "DOC").
			@param document_primary_key_tag [in] The name of the element that contans the document's primary key (default = "DOCNO").
			@details Calls the parent constructor and then resets the key start and end tags for the document and the primary key appropriately.
		*/
		instream_document_html(std::shared_ptr<instream> &source, const std::string &document_tag = "html", const std::string &document_primary_key_tag = "title") : instream_document_trec(source, document_tag, document_primary_key_tag)
			{
			set_tags(document_tag, document_primary_key_tag);
			}

		/*
			INSTREAM_DOCUMENT_HTML::INSTREAM_DOCUMENT_HTML()
			------------------------------------------------
		*/
		/*!
			@brief Destructor
		*/
		virtual ~instream_document_html()
			{
			// nothing
			}

		/*
			INSTREAM_DOCUMENT_HTML::SET_TAGS()
			----------------------------------
		*/
		/*!
			@brief Register the document tag and the primary key tag.  Used to set up internal data structures.
			@param document_tag [in] The name of the tag used to delineate docments.
			@param primary_key_tag [in] The name of the element that contans the document's primary key.
			@details Calls the parent funtion, and then removes the \> from the end of the tag that begins a document
		*/
		virtual void set_tags(const std::string &document_tag, const std::string &primary_key_tag)
			{
			instream_document_trec::set_tags(document_tag, primary_key_tag);
			document_start_tag = "<" + document_tag;
			}
		};

	}
