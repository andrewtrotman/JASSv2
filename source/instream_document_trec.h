/*
	INSTREAM_DOCUMENT_TREC.H
	------------------------
*/
#pragma once
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
		static const size_t buffer_size = (16 * 1024 * 1024);

	protected:
		char *document_start, *document_end;

		char *primary_buffer, *secondary_buffer;
		char **buffer_to_read_into, **buffer_to_read_from;
		long long *end_of_buffer, *end_of_second_buffer;
		long long position_of_end_of_buffer, position_of_end_of_second_buffer;

		long long primary_buffer_used, secondary_buffer_used;

		long auto_file_id;
	
		std::string document_start_tag;
		std::string document_end_tag;
		std::string primary_key_start_tag;
		std::string primary_key_end_tag;

	public:
		instream_document_trec(instream &source, const std::string &document_tag = "DOC", const std::string &document_primary_key_tag = "DOCNO");
		virtual ~instream_document_trec();

		virtual void read(document &buffer);
	} ;
}