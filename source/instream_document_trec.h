/*
	INSTREAM_DOCUMENT_TREC.H
	------------------------
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
	class instream_document_trec : public instream
	{
	private:
		static const size_t buffer_size = (16 * 1024 * 1024);			///< Size of the disk read buffer

	protected:
		uint8_t *buffer;
		uint8_t *buffer_end;
		size_t buffer_used;

		long long primary_buffer_used;

		long auto_file_id;
	
		std::string document_start_tag;
		std::string document_end_tag;
		std::string primary_key_start_tag;
		std::string primary_key_end_tag;

	private:
		void set_tags(const std::string &document_tag, const std::string &primary_key_tag);

	public:
		instream_document_trec(instream &source, const std::string &document_tag = "DOC", const std::string &document_primary_key_tag = "DOCNO");
		virtual ~instream_document_trec();

		virtual void read(document &buffer);
		

	} ;
}