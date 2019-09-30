/*
	INSTREAM_DOCUMENT_FASTA.CPP
	---------------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>
#include <string.h>

#include <new>
#include <memory>
#include <algorithm>

#include "assert.h"
#include "instream_memory.h"
#include "instream_document_fasta.h"

namespace JASS
	{
	/*
		INSTREAM_DOCUMENT_FASTA::INSTREAM_DOCUMENT_FASTA()
		--------------------------------------------------
	*/
	instream_document_fasta::instream_document_fasta(std::shared_ptr<instream> &source, size_t buffer_size) :
		instream(source),
		buffer_size(buffer_size)
		{
		/*
			Allocate the internal buffer and keep a pointer to its end.
		*/
		buffer = new uint8_t[buffer_size + 1];
		buffer_end = buffer;
		buffer_used = 0;
		}


	/*
		INSTREAM_DOCUMENT_FASTA::INSTREAM_DOCUMENT_FASTA()
		--------------------------------------------------
	*/
	instream_document_fasta::instream_document_fasta(std::shared_ptr<instream> &source) :
		instream_document_fasta(source, 16 * 1024 * 1024)
		{
		/*
			Nothing - all managed by the protected constructor
		*/
		}

	/*
		INSTREAM_DOCUMENT_FASTA::~INSTREAM_DOCUMENT_FASTA()
		---------------------------------------------------
	*/
	instream_document_fasta::~instream_document_fasta()
		{
		delete [] buffer;
		}

	/*
		INSTREAM_DOCUMENT_FASTA::READ()
		-------------------------------
	*/
	void instream_document_fasta::read(document &object)
		{
		uint8_t *unread_data = buffer + buffer_used;

		/*
			Find the start of the document
		*/
		uint8_t *document_start;
		if ((document_start = std::find(unread_data, buffer_end, '>')) == buffer_end)
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
		
			if ((document_start = std::find(unread_data, buffer_end, '>')) == buffer_end)
				{
				/*
					Most probably end of file.
				*/
				object.primary_key = object.contents = slice();
				return;
				}
			buffer_used = 1;
			}

		/*
			Find the end tag
		*/
		uint8_t *document_end;
		static const char *end_of_document_marker = "\n>";
		if ((document_end = std::search(document_start + 1, buffer_end, end_of_document_marker, end_of_document_marker + 2)) == buffer_end)
			{
			/*
				This happens when we move find the start tag in the buffer, but the end tag is not in memory.  We play the 
				same game as above and shift the start tag to the start of the buffer.
			*/
			memmove(buffer, document_start, (buffer_end - buffer) - (document_start - buffer));
			buffer_end -= document_start - buffer;
			fetch(buffer_end, buffer_size - (buffer_end - buffer));
			document_start = buffer;
			document_end = std::search(document_start + 1, buffer_end, end_of_document_marker, end_of_document_marker + 2);
			if (document_end == buffer_end)
				document_end--;
			}

		buffer_used = document_end - buffer;		// skip to end of the document
		document_end++;		// because the end of line marker matters (believe it or not!).

		/*
			Extract the document's primary key.
		*/
		uint8_t *document_id_end = document_end;
		uint8_t *document_id_start = std::find(document_start, document_end, '>');
		if (document_id_start != document_end)
			{
			document_id_end = std::find(document_id_start + 1, document_end, '\n') + 1;
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
		INSTREAM_DOCUMENT_FASTA::UNITTEST()
		-----------------------------------
	*/
	void instream_document_fasta::unittest(void)
		{
		static const std::string example_1_key = ">NR_118889.1";
		static const std::string example_1 = "GG\nTCT NATA\nCCGGATATAACAAC TCATGGCATGGTTGGTAGTG GAAAGCTCCG GCGGT\n";
		static const std::string example_2_key = ">NR_118899.1";
		static const std::string example_2 = "GGGTGAGTAACACGTGAGTAACCTGCCCCNNACTTCTGGATAACCGCTTGAAGGGTNG\n";
		static const std::string example_3_key = ">NR_074334.1 >NR_118873.1 >NR_119237.1";
		static const std::string example_3 = "ATTCTGGTTGATCCTGCCAGAGGCCGCTGCTATCCGGCTGGGACTAAGCCATGAGTCA\n";
		static const std::string example_4_key = ">NR_118890.1";
		static const std::string example_4 = "TACTTTGGGATAAGCCTGGGAAACTGGGTCTNATACCGGATATGACAACTGAGCATGG\n";
		static const std::string example_5_key = ">NR_044838.1";
		static const std::string example_5 = "TTGAACGGAGAGTTCGANCCTGGCTCAGGATGAACGCTGGCGGCGCGCCTAAATGCAA\n";

		static const std::string example_data = example_1_key + "\n" + example_1 + example_2_key + "\n" + example_2 + example_3_key + "\n" + example_3 + example_4_key + "\n" + example_4 + example_5_key + "\n" + example_5;
		/*
			Set up a pipeline that is an instream_document_trec reading from a instream_document_trec reading from a instream_memory reading from the unittest_data::ten_documents string.
		*/
		std::shared_ptr<instream> buffer(new class instream_memory((uint8_t *)example_data.c_str(), example_data.size()));
		std::shared_ptr<instream> first_slice(new class instream_document_fasta(buffer, 1024 * 1024));				// call the protected constructor and tell it to use an unusually small buffer
		instream_document_fasta slicer(first_slice, 1024 * 1024);				// call the protected constructor and tell it to use an unusually small buffer
//instream_document_fasta slicer(buffer, 1024 * 1024);				// call the protected constructor and tell it to use an unusually small buffer

		document indexable_object;

		/*
			Check each read to make sure it worked.
		*/
		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == example_1_key + "\n" + example_1);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == example_1_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == example_2_key + "\n" + example_2);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == example_2_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == example_3_key + "\n" + example_3);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == example_3_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == example_4_key + "\n" + example_4);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == example_4_key);

		slicer.read(indexable_object);
		JASS_assert(std::string((char *)&indexable_object.contents[0], indexable_object.contents.size()) == example_5_key + "\n" + example_5);
		JASS_assert(std::string((char *)&indexable_object.primary_key[0], indexable_object.primary_key.size()) == example_5_key);

		slicer.read(indexable_object);
		JASS_assert(indexable_object.contents.size() == 0);

		/*
			Success
		*/	
		puts("instream_document_fasta::PASSED");
		}
	}
