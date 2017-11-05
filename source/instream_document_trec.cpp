/*
	INSTREAM_DOCUMENT_TREC.CPP
	--------------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>
#include <string.h>

#include <new>
#include <memory>
#include <algorithm>

#include "assert.h"
#include "unittest_data.h"
#include "instream_memory.h"
#include "instream_document_trec.h"

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
		------------------------------------------------
	*/
	instream_document_trec::instream_document_trec(std::shared_ptr<instream> &source, size_t buffer_size, const std::string &document_tag, const std::string &document_primary_key_tag) :
		instream(source),
		buffer_size(buffer_size)
		{
		/*
			Allocate the internal buffer and keep a pointer to its end.
		*/
		buffer = new uint8_t[buffer_size + 1];
		buffer_end = buffer;
		buffer_used = 0;

		/*
			Set up the internal housekeeping for the tags
		*/
		set_tags(document_tag, document_primary_key_tag);
		}


	/*
		INSTREAM_DOCUMENT_TREC::INSTREAM_DOCUMENT_TREC()
		------------------------------------------------
	*/
	instream_document_trec::instream_document_trec(std::shared_ptr<instream> &source, const std::string &document_tag, const std::string &document_primary_key_tag) :
		instream_document_trec(source, 16 * 1024 * 1024, document_tag, document_primary_key_tag)
		{
		/*
			Nothing - all managed by the protected constructor
		*/
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
		/*
			Generate the search patterns for the document seperator and the primary key
		*/
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
			memmove(buffer, unread_data, buffer_size - buffer_used);
			buffer_end -= buffer_used;
			buffer_used = 0;
			
			fetch(buffer_end, buffer + buffer_size - buffer_end);
			unread_data = buffer;
		
			if ((document_start = std::search(unread_data, buffer_end, document_start_tag.c_str(), document_start_tag.c_str() + document_start_tag.size())) == buffer_end)
				{
				/*
					Most probably end of file.
				*/
				object.primary_key = object.contents = slice();
				return;
				}
			buffer_used = document_start_tag.size();
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
			memmove(buffer, document_start, (buffer_end - buffer) - (document_start - buffer));
			buffer_end -= document_start - buffer;
			fetch(buffer_end, buffer_size - (buffer_end - buffer));
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
		uint8_t *document_id_end = document_end;
		uint8_t *document_id_start = std::search(document_start, document_end, primary_key_start_tag.c_str(), primary_key_start_tag.c_str() + primary_key_start_tag.size());
		if (document_id_start != document_end)
			{
			document_id_start += primary_key_start_tag.size();
			document_id_end = std::search(document_id_start, document_end, primary_key_end_tag.c_str(), primary_key_end_tag.c_str() + primary_key_end_tag.size());

			/*
				Trim whitespace from the start and end of the primary key.
			*/
			while (isspace(*document_id_start) && document_id_start < document_end)			// trim early whitespace
				document_id_start++;
			while (isspace(*(document_id_end - 1)) && document_id_end > document_start)				// trip trailing whitespace
				document_id_end--;
			}

		/*
			Copy the id into the document object and get the document
		*/
		object.contents = slice(object.contents_allocator, document_start, document_end);
		if (document_id_end == document_end)
			object.primary_key = slice(object.primary_key_allocator, "Unknown");
		else
			object.primary_key = slice(object.primary_key_allocator, document_id_start, document_id_end);
		}
		
	/*
		INSTREAM_DOCUMENT_TREC::UNITTEST()
		----------------------------------
	*/
	void instream_document_trec::unittest(void)
		{
		/*
			Set up a pipeline that is an instream_document_trec reading from a instream_document_trec reading from a instream_memory reading from the unittest_data::ten_documents string.
		*/
		std::shared_ptr<instream> buffer(new class instream_memory((uint8_t *)unittest_data::ten_documents.c_str(), unittest_data::ten_documents.size()));
		std::shared_ptr<instream> first_slice(new class instream_document_trec(buffer, 80, "DOC", "DOCNO"));				// call the protected constructor and tell it to use an unusually small buffer
		instream_document_trec slicer(first_slice, 80, "DOC", "DOCNO");				// call the protected constructor and tell it to use an unusually small buffer
		document indexable_object;

		/*
			Check each read to make sure it worked.
		*/
		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_1);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_1_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_2);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_2_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_3);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_3_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_4);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_4_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_5);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_5_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_6);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_6_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_7);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_7_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_8);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_8_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_9);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_9_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == unittest_data::ten_document_10);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == unittest_data::ten_document_10_key);

		slicer.read(indexable_object);
		JASS_assert(indexable_object.contents.size() == 0);


		/*
			Now check the failure states
		*/
		/*
			Missing </DOC>
		*/
		buffer.reset(new instream_memory((uint8_t *)unittest_data::ten_document_11_broken.c_str(), unittest_data::ten_document_11_broken.size()));
		first_slice.reset(new instream_document_trec(buffer, 128, "DOC", "DOCNO"));
		instream_document_trec slicer_11(first_slice, 128, "DOC", "DOCNO");
		slicer_11.read(indexable_object);
		JASS_assert(indexable_object.contents.size() == 0);

		/*
			Missing </DOCNO>
		*/
		buffer.reset(new instream_memory((uint8_t *)unittest_data::ten_document_13_broken.c_str(), unittest_data::ten_document_13_broken.size()));
		first_slice.reset(new instream_document_trec(buffer, 128, "DOC", "DOCNO"));
		instream_document_trec slicer_13(first_slice, 128, "DOC", "DOCNO");
		slicer_13.read(indexable_object);
		JASS_assert(indexable_object.contents.size() == 92);

		/*
			Success
		*/	
		puts("instream_document_trec::PASSED");
		}

	}
