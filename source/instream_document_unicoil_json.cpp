/*
	INSTREAM_DOCUMENT_UNICOIL_JSON.CPP
	----------------------------------
*/
#include <string.h>

#include <memory>
#include <algorithm>

#include "ascii.h"
#include "instream_memory.h"
#include "instream_document_unicoil_json.h"

static const char *start_of_document = "{\"id\":";
static const char *document_id_marker = "\"id\": \"";
static const char *vector_marker = "\"vector\": {";
static const char *end_of_document = "}}\n";

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_UNICOIL_JSON::READ()
		--------------------------------------
	*/
	void instream_document_unicoil_json::read(document &object)
		{
		const char *doc_start = nullptr;
		const char *docid_start = nullptr;
		const char *docid_end = nullptr;
		const char *doc_end = nullptr;

		/*
			Check for EOF
		*/
		if (buffer_end < buffer_start)
			{
			object.primary_key = object.contents = slice();
			return;
			}

		/*
			Get the document start and end
		*/
		do
			{
			doc_start = std::search(buffer_start, buffer_end, start_of_document, start_of_document + 6);
			doc_end = std::search(buffer_start, buffer_end, end_of_document, end_of_document + 3);

			if (doc_end == buffer_end)
				{
				/*
					Two possibilities, one is that the buffer is too small, the other is that we've reached the end of the buffer and so need to read more.
				*/
				if (buffer_start == &buffer[0])
					{
					/*
						The buffer is too small so extend it
					*/
					size_t bytes_in_use = buffer_end - buffer_start;
					size_t bytes_remaining = buffer_size - bytes_in_use;

					if (bytes_in_use != 0)
						{
						/*
							Double the size of the buffer if its got data in it - because its too small.
						*/
						size_t gap = doc_start - buffer_start;
						buffer_size += buffer_size;
						buffer.resize(buffer_size);		// create space and move the old contents into the new space
						buffer_start = &buffer[0];
						buffer_end = &buffer[0] + bytes_in_use;
						doc_start = &buffer[0] + gap;

						bytes_remaining += buffer_size;
						}

					size_t bytes = source->fetch((void *)buffer_end, bytes_remaining);
					doc_end = buffer_end += bytes;

					/*
						At EOF
					*/
					if (bytes == 0)
						{
						if (doc_start != nullptr)
							{
							doc_end = buffer_end;			// the last document might not end in a '\n'.
							if (doc_end != doc_start)
								break;							// the last document isn't empty
							}

						object.primary_key = object.contents = slice();
						return;
						}
					}
				else
					{
					/*
						We're not at the start of the buffer so move the remaining data and go to the start of the buffer
					*/
					doc_start -= buffer_start - &buffer[0];
					memmove(&buffer[0], buffer_start, buffer_end - buffer_start);
					size_t bytes_in_use = buffer_end - buffer_start;
					size_t bytes_remaining = buffer_size - bytes_in_use;
					buffer_start = &buffer[0];
					doc_end = buffer_end = buffer_start + bytes_in_use;

					/*
						Now fill the remainder of the buffer
					*/
					size_t bytes = source->fetch((void *)buffer_end, bytes_remaining);
					doc_end = buffer_end += bytes;
					}
				}
			}
		while (doc_end == buffer_end);

		/*
			Get the document primary key
		*/
		docid_start = std::search(doc_start, doc_end, document_id_marker, document_id_marker + 7);
		if (docid_start != nullptr)
			if ((docid_start = strchr(docid_start + 6, '"')) != nullptr)
				docid_end = strchr(++docid_start, '"');

		if (docid_end == buffer_end)
			{
			static const char *none = "AnonymousDocument";
			object.primary_key = slice(object.primary_key_allocator, none, none + strlen(none));
			}
		else
			object.primary_key = slice(object.primary_key_allocator, docid_start, docid_end);

		/*
			Get and store the document. In this case we're only looking for the data marked "vector"
		*/
		doc_start = std::search(doc_start, doc_end, vector_marker, vector_marker + 11) + 11;
		object.contents = slice(object.contents_allocator, doc_start, doc_end);
		
		buffer_start = doc_end +2;
		}

	/*
		INSTREAM_DOCUMENT_UNICOIL_JSON::UNITTEST()
		------------------------------------------
	*/
	void instream_document_unicoil_json::unittest(void)
		{
		/*
			An example JSON file
		*/
		std::string example_file =
			"{\"id\": \"0\", \"contents\": \"The.\", \"vector\": {\"the\": 94}}\n\n"
			"{\"id\": \"1\", \"contents\": \"id\", \"vector\": {\"id\": 101}}";

		/*
			The correct documents
		*/
		const char *first_answer = "\"the\": 94";
		const char *first_key = "0";
		const char *second_answer = "\"id\": 101";
		const char *second_key = "1";

		/*
			Set up a reader from memory
		*/
		std::shared_ptr<instream> source(new instream_memory(example_file.c_str(), example_file.size()));
		instream_document_unicoil_json getter(source);

		/*
			Extract 2 documents to make sure we get the right answers
		*/
		document doc;
		getter.read(doc);
		JASS_assert(strncmp(reinterpret_cast<char *>(doc.primary_key.address()), first_key, strlen(first_key)) == 0);
		JASS_assert(strncmp(reinterpret_cast<char *>(doc.contents.address()), first_answer, strlen(first_answer)) == 0);
		getter.read(doc);
		JASS_assert(strncmp(reinterpret_cast<char *>(doc.primary_key.address()), second_key, strlen(second_key)) == 0);
		JASS_assert(strncmp(reinterpret_cast<char *>(doc.contents.address()), second_answer, strlen(second_answer)) == 0);

		/*
			Make sure we can mark EOF correctly
		*/
		getter.read(doc);
		JASS_assert(doc.isempty());

		/*
			Success
		*/
		puts("instream_document_unicoil_json::PASSED");
		}
	}
