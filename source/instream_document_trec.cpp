/*
	INSTREAM_DOCUMENT_TREC.CPP
	--------------------------
*/
#include <new>
#include <algorithm>

#include <stdio.h>
#include <string.h>

#include "unittest_data.h"
#include "instream_memory.h"
#include "instream_document_trec.h"

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
		------------------------------------------------
	*/
	instream_document_trec::instream_document_trec(instream &source, size_t buffer_size, const std::string &document_tag, const std::string &document_primary_key_tag) :
		instream(nullptr, &source),
		buffer_size(buffer_size)
		{
		buffer = new uint8_t[buffer_size + 1];
		buffer_end = buffer;
		buffer_used = 0;
		*buffer = buffer[buffer_size] = '\0';

		set_tags(document_tag, document_primary_key_tag);
		}


	/*
		INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
		------------------------------------------------
	*/
	instream_document_trec::instream_document_trec(instream &source, const std::string &document_tag, const std::string &document_primary_key_tag) :
		instream_document_trec(source, 16 * 1024 * 1024, document_tag, document_primary_key_tag)
		{
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
		INSTREAM_DOCUMENT_TREC::FETCH()
		-------------------------------
	*/
	void instream_document_trec::fetch(void *buffer, size_t bytes)
		{
		document into;
		
		into.contents = slice(buffer, bytes);
		source->read(into);
		buffer_end = (uint8_t *)buffer + into.contents.size();
		}

	/*
		INSTREAM_DOCUMENT_TREC::READ()
		------------------------------
	*/
	void instream_document_trec::read(document &object)
		{
		uint8_t *unread_data = buffer + buffer_used;

		/*
			Find the start tag
		*/
		uint8_t *document_start;
		if ((document_start = std::search(unread_data, buffer_end, document_start_tag.c_str(), document_start_tag.c_str() + document_start_tag.size())) == buffer_end)
			{
			/*
				We might be at the end of a buffer and half-way through a tag so we copy the remainder of the file to the
				start of the buffer and fill the remainder.
			*/
			memmove(buffer, buffer + buffer_used, buffer_size - buffer_used);
			fetch(buffer + buffer_used, buffer_size - buffer_used);
			buffer_used = document_start_tag.size();

			if ((document_start = std::search(unread_data, buffer_end, document_start_tag.c_str(), document_start_tag.c_str() + document_start_tag.size())) == buffer_end)
				{
				/*
					Most probably end of file.
				*/
				object.primary_key = object.contents = slice();
				return;
				}
			}

		/*
			Find the end tag
		*/
		uint8_t *document_end;
		if ((document_end = std::search(document_start, buffer_end, document_end_tag.c_str(), document_end_tag.c_str() + document_end_tag.size())) == buffer_end)
			{
			/*
				This happens when we move find the start tag in the buffer, but the end tag is not in memory.  We play the 
				same game as above and shift the start tag to the start of the buffer.
			*/
			memmove(buffer, document_start, buffer_size - (buffer - document_start));
			fetch(buffer + buffer_size - buffer_used, document_start - buffer);
			document_start = buffer;
			if ((document_end = std::search(document_start, buffer_end, document_end_tag.c_str(), document_end_tag.c_str() + document_end_tag.size())) == buffer_end)
				{
				/*
					We are either at end of file of have a document that is too large to index (so pretend EOF)
				*/
				object.primary_key = object.contents = slice();
				return;
				}
			}
		document_end += document_end_tag.size();
		buffer_used = document_end - buffer;		// skip to end of end tag

		/*
			Extract the document's primary key.
		*/
		uint8_t *document_id_end = NULL;
		uint8_t *document_id_start = std::search(document_start, document_end, primary_key_start_tag.c_str(), primary_key_start_tag.c_str() + primary_key_start_tag.size());
		if (document_id_start != NULL)
			{
			document_id_start += primary_key_start_tag.size();
			document_id_end = std::search(document_id_start, document_end, primary_key_end_tag.c_str(), primary_key_end_tag.c_str() + primary_key_end_tag.size());
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
		
	/*
		INSTREAM_DOCUMENT_TREC::UNITTEST()
		----------------------------------
	*/
	void instream_document_trec::unittest(void)
		{
		instream_memory *buffer = new instream_memory((uint8_t *)unittest_data_ten_documents.c_str(), unittest_data_ten_documents.size());
		instream_document_trec slicer(*buffer, 80);				// call the private constructor and tell it to use an unusually small buffer
		document indexable_object;

		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_1);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_2);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_3);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_4);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_5);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_6);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_7);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_8);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_9);
		
		slicer.read(indexable_object);
		assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data_ten_document_10);
		
		slicer.read(indexable_object);
		assert(indexable_object.contents.size() == 0);
		
		puts("instream_document_trec::PASSED");
		}

}
