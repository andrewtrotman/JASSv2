/*
	INSTREAM_DOCUMENT_WARC.CPP
	--------------------------
*/
#include <string.h>

#include <memory>

#include "ascii.h"
#include "instream_memory.h"
#include "instream_document_warc.h"

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_WARC::FIND_STRING()
		-------------------------------------
	*/
	const char *instream_document_warc::find_string(const std::string &string)
		{
		size_t string_length = string.size();

		/*
			Do a series of gets()'s until we find the given string
		*/
		buffer[string_length] = '\0';		// prevent early accidental termination (due to a ':' left over from a previous iteration).
		const char *end = buffer.c_str() + buffer.size() - 1;
		do
			do
				{
				/*
					Read a '\n' terminated string from the source.
				*/
				buffer[0] = '\0';
				char *into = const_cast<char *>(buffer.c_str() - 1);
				do
					{
					into++;
					if (source->fetch(into, 1) != 1)
						return nullptr;		// at EOF
					}
				while (*into != '\n' && into < end);
				*into = '\0';

				/*
					In the case of buffer overflow we read to end of line and start over.  This shouldn't happen very often,
					but it does happen because there are some very long WARC-Target-URI in the TREC ClueWeb09 collection
					in documents such as clueweb09-en0000-05-10880 which as a 1486 character WARC-Target-URI!  It also won't
					affect the string comparison because we're only comparing to the start of the string.
				*/
				if (into == end)
					{
					do
						if (source->fetch(into, 1) != 1)
							return nullptr;		// at EOF
					while (*into != '\n');
					continue;		// don't do the string compare because we're out of sync
					}
				}
			while (buffer[string_length] != ':');			// check for the ':' in the right place
		while (buffer.compare(0, string_length, string) != 0);	// now check for the TREC-ID

		return buffer.c_str() + string_length + 1;
		}

	/*
		INSTREAM_DOCUMENT_WARC::READ()
		------------------------------
	*/
	static const std::string warc_trec_id = "WARC-TREC-ID";				// initialise at program startup
	static const std::string content_length = "Content-Length"; 		// initialise at program startup
	
	void instream_document_warc::read(document &object)
		{
		const char *filename;

		/*
			Get and store the filename
		*/
		if ((filename = find_string(warc_trec_id)) == NULL)
			object.primary_key = object.contents = slice();
		else
			{
			/*
				Get the document primary key
			*/
			while (ascii::isspace(*filename))
				filename++;

			object.primary_key = slice(object.primary_key_allocator, filename, filename + strlen(filename));

			/*
				Get and store the document length
			*/
			const char *file_length;
			if ((file_length = find_string(content_length)) == NULL)
				object.primary_key = object.contents = slice();
			else
				{
				size_t length = atoll(file_length);
				char *document = reinterpret_cast<char *>(object.contents_allocator.malloc(length + 1));

				source->fetch(document, length);
				document[length] = '\0';
				object.contents = slice(document, document + length);
				}
			}
		}

	/*
		INSTREAM_DOCUMENT_WARC::UNITTEST()
		----------------------------------
	*/
	void instream_document_warc::unittest(void)
		{
		/*
			An example WARC file, a snippet from ClueWeb13B
		*/
		std::string example_file =
			"WARC/1.0\n"
			"WARC-Type: warcinfo\n"
			"WARC-Date: 2012-02-10T21:42:47Z\n"
			"WARC-Data-Type: twitter links\n"
			"WARC-File-Length: 72730302\n"
			"WARC-Filename: 0000tw-00.warc.gz\n"
			"WARC-Number-of-Documents: 1768\n"
			"WARC-Record-ID: <urn:uuid:5a67c755-09e8-41f8-b9f9-6e8fcf30f353>\n"
			"Content-Type: application/warc-fields\n"
			"Content-Length: 283\n"
			"\n"
			"software: Heritrix/3.1.1-SNAPSHOT-20120210.102032 http://crawler.archive.org\n"
			"format: WARC File Format 1.0\n"
			"conformsTo: http://bibnum.bnf.fr/WARC/WARC_ISO_28500_version1_latestdraft.pdf\n"
			"isPartOf: ClueWeb12\n"
			"description:  The Lemur Project's ClueWeb12 dataset (http://lemurproject.org/)\n"
			"\n"
			"\n"
			"WARC/1.0\n"
			"WARC-Type: response\n"
			"WARC-Date: 2012-02-10T21:51:20Z\n"
			"WARC-TREC-ID: clueweb12-0000tw-00-00013\n"
			"WARC-Payload-Digest: sha1:YZUOJNSUMFG3JVUKM6LBHMRMMHWLVNQ4\n"
			"WARC-IP-Address: 100.42.59.15\n"
			"WARC-Target-URI: http://cheapcosthealthinsurance.com/2012/01/25/what-is-hiv-aids/\n"
			"WARC-Record-ID: <urn:uuid:74edc71e-a881-4942-81fc-a40db4bf1fb9>\n"
			"Content-Type: application/http; msgtype=response\n"
			"Content-Length: 9\n"
			"\n"
			"HTTP/1.1\n"
			"\n"
			"\n"
			"WARC/1.0\n"
			"WARC-Type: response\n"
			"WARC-Date: 2012-02-10T21:49:12Z\n"
			"WARC-TREC-ID: clueweb12-0000tw-00-00027\n"
			"WARC-Payload-Digest: sha1:A2F6UD2MR7TRJY75VZMTZCX3UFOXUIK3\n"
			"WARC-IP-Address: 100.42.59.15\n"
			"WARC-Target-URI: http://cheapcosthealthinsurance.com/2012/02/06/united-healthcare/\n"
			"WARC-Record-ID: <urn:uuid:a95a43c5-cdce-4d90-aa8b-0b961ae447f9>\n"
			"Content-Type: application/http; msgtype=response\n"
			"Content-Length: 16\n"
			"\n"
			"HTTP/1.1 200 OK\n"
			"\n"
			"\n";
		/*
			The correct documents
		*/
		const char *first_answer = "\nHTTP/1.1";
		const char *first_key = "clueweb12-0000tw-00-00013";
		const char *second_answer = "\nHTTP/1.1 200 OK";
		const char *second_key = "clueweb12-0000tw-00-00027";

		/*
			set up a reader from memory
		*/
		std::shared_ptr<instream> source(new instream_memory(example_file.c_str(), example_file.size()));
		instream_document_warc getter(source);

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
		puts("instream_document_warc::PASSED");
		}
	}
