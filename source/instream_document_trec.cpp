/*
	INSTREAM_DOCUMENT_TREC.CPP
	--------------------------
*/
#include <new>

#include <stdio.h>
#include <string.h>

#include "instream_document_trec.h"

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
		------------------------------------------------
	*/
	instream_document_trec::instream_document_trec(instream &source, const std::string &document_tag, const std::string &document_primary_key_tag)
		{
		buffer = new uint8_t[buffer_size + 1];
		buffer_end = buffer + buffer_size;
		buffer_used = 0;
		*buffer = buffer[buffer_size] = '\0';

		set_tags(document_tag, document_primary_key_tag);
		}

	/*
		INSTREAM_DOCUMENT_TREC::~INSTREAM_DOCUMENT_TREC()
		-------------------------------------------------
	*/
	instream_document_trec::~instream_document_trec()
		{
		delete [] buffer;
		}

	/*
		INSTREAM_DOCUMENT_TREC::SET_TAGS()
		----------------------------------
	*/
	void instream_document_trec::set_tags(const std::string &document_tag, const std::string &primary_key_tag)
		{
		document_start_tag = "<" + document_tag + ">";
		document_end_tag = "</" + document_tag + ">";
		primary_key_start_tag = "<" + primary_key_tag + ">";
		primary_key_end_tag = "</" + primary_key_tag + ">";
		}

	/*
		INSTREAM_DOCUMENT_TREC::READ()
		------------------------------
	*/
	void instream_document_trec::read(document &object)
		{
		char *unread_data = (char *)buffer + buffer_used;

		/*
			Find the start tag
		*/
		char *document_start;
		if ((document_start = strstr(unread_data, document_start_tag.c_str())) == NULL)
			{
			/*
				We might be at the end of a buffer and half-way through a tag so we copy the remainder of the file to the
				start of the buffer and fill the remainder.
			*/
			memmove(buffer, buffer + buffer_used, buffer_size - buffer_used);
			fetch(buffer + buffer_used, buffer_size - buffer_used);
			buffer_used = 0;

			if ((document_start = strstr(unread_data, document_start_tag.c_str())) == NULL)
				return NULL;		// Most probably end of file.
			}

		/*
			Find the end tag
		*/
		char *document_end;
		if ((document_end = strstr(document_start, document_end_tag.c_str())) == NULL)
			{
			/*
				This happens when we move find the start tag in the buffer, but the end tag is not in memory.  We play the 
				same game as above and shift the start tag to the start of the buffer.
			*/
			memmove(buffer, document_start, buffer_size - ((char *)buffer - document_start));
			fetch(buffer + buffer_size - buffer_used, document_start - buffer);
			document_start = (char *)buffer;
			if ((document_end = strstr(document_start, document_end_tag.c_str())) == NULL)
				return NULL;		// We are either at end of file of have a document that is too large to index (so pretend EOF)
			}

		buffer_used = document_end + document_end_tag.size() - (char *)buffer;		// skip to end of end tag

		/*
			Extract the document's primary key.
		*/
		char *document_id_end = NULL;
		char *document_id_start = strstr(document_start, primary_key_start_tag.c_str());
		if (document_id_start != NULL)
			{
			document_id_start += primary_key_start_tag.size();
			document_id_end = strstr(document_id_start, primary_key_end_tag.c_str());
			}

		/*
			Copy the id into the document object and get the document
		*/
		object.contents = slice(object.allocator, document_start, document_end);
		if (document_id_end == NULL)
			object.primary_key = slice(object.allocator, "Unknown");
		else
			object.primary_key = slice(object.allocator, document_id_start, document_id_end);
		}
}
