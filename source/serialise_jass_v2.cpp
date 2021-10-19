/*
	SERIALISE_JASS_V2.CPP
	---------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include "reverse.h"
#include "checksum.h"
#include "allocator.h"
#include "serialise_jass_v2.h"
#include "compress_integer_all.h"
#include "index_manager_sequential.h"

namespace JASS
	{

	/*
		SERIALISE_JASS_V2::WRITE_POSTINGS()
		-----------------------------------
	*/
	size_t serialise_jass_v2::write_postings(const index_postings &postings_list, size_t &number_of_impacts, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
		{
		/*
			Clear the internal buffers.
		*/
		compressed_segments.clear();
		compressed_headers.clear();
		compressed_offsets.clear();

		/*x
			Keep a track of where the postings are stored on disk (we return this to the caller).
		*/
		size_t postings_location = postings.tell();

		/*
			Impact order the postings list.
		*/
		postings_list.impact_order(documents, impact_ordered, document_frequency, document_ids, term_frequencies);

		/*
			Compute the number of impact headers we're going to see (which we return to the caller).
		*/
		number_of_impacts = impact_ordered.impact_size();

		/*
			Compress each postings segment and store the offset to it.
			First D1 encode, then use the encoder to compress.
		*/
		uint8_t *compress_into = &compressed_buffer[0];
		auto compress_into_size = compressed_buffer.size();
//std::cout << "\nPOSTINGS SIZES\n";
		for (const auto &segment : impact_ordered)
			{
			compress_integer::d1_encode(segment.begin(), segment.begin(), segment.size());
			*segment.begin() -= 1;			// JASS v1 counts documents from 0.
			auto took = encoder->encode(compress_into, compress_into_size, segment.begin(), segment.size());
			if (took == 0)
				{
				std::cout <<  "Failed to compress postings list while serialising" << std::ends;
				exit(1);
				}

			/*
				Save the compressed data.
			*/
			compressed_segments.push_back(slice(compress_into, took));

			compress_into += took;
			compress_into_size -= took;
//std::cout << "[" << *segment.begin() << "]" << took << "\n";
			}

		/*
			Build all the headers
		*/
		size_t which = 0;
		uint64_t location = 0;
		uint64_t sum_of_header_sizes = 0;
		for (const auto &impact : impact_ordered)
			{
			uint8_t *start = compress_into;
			uint64_t header_size = 0;

			/*
				Impact score
			*/
			header_size += compress_integer_variable_byte::bytes_needed_for((uint32_t)impact.impact_score);
			compress_integer_variable_byte::compress_into(compress_into, (uint32_t)impact.impact_score);
//std::cout << "[I:" << impact.impact_score << "]";

			/*
				Start (offset from the end of this header)
			*/
			header_size += compress_integer_variable_byte::bytes_needed_for(location);
			compress_integer_variable_byte::compress_into(compress_into, location);
//std::cout << "[S:" << location << "]";

			/*
				Length (this was a pointer to the end, but is now the length)
			*/
			header_size += compress_integer_variable_byte::bytes_needed_for((uint64_t)compressed_segments[which].size());
			compress_integer_variable_byte::compress_into(compress_into, (uint64_t)compressed_segments[which].size());
//std::cout << "[L:" << compressed_segments[which].size() << "]";

			/*
				Number of documents in this impact
			*/
			header_size += compress_integer_variable_byte::bytes_needed_for(impact.size());
			compress_integer_variable_byte::compress_into(compress_into, impact.size());
//std::cout << "[N:" << impact.size() << "] ";

			/*
				Save the size and compute the sizes used so far, ready for the next time around the loop;
			*/
			compressed_headers.push_back(slice(start, header_size));

			sum_of_header_sizes += header_size;
			location += compressed_segments[which].size() + header_size;
			which++;
			}

		/*
			Now write out the pointers to the headers (also compressed)
			Iterate from largest impact to smallest impact
		*/
//std::cout << "\nHEADER SIZES\n";
//for (const auto &header : compressed_headers)
//std::cout << header.size() << '\n';
//
//std::cout << "HEADER OFFSETS\n";
		uint64_t offset = 0;
		for (const auto &header : reverse(compressed_headers))
			{
//std::cout << offset << '\n';
			uint8_t *start = compress_into;
			uint64_t size = compress_integer_variable_byte::bytes_needed_for(offset);
			compress_integer_variable_byte::compress_into(compress_into, offset);
			compressed_offsets.push_back(slice(start, size));
			offset += header.size() + size;
			}

		/*
			Write out each offset
		*/
//std::cout << "Sequence-> ";
		for (const auto &offset : reverse(compressed_offsets))
			{
			postings.write(offset.address(), offset.size());
//for (size_t byte = 0; byte < offset.size(); byte++)
//printf("%02X ", *((uint8_t *)offset.address() + byte));
//std::cout << offset.size() << " ";
			}

		/*
			Write out each postings list header.
		*/
//std::cout << ": ";
		for (const auto &header : reverse(compressed_headers))
			{
			postings.write(header.address(), header.size());
//for (size_t byte = 0; byte < header.size(); byte++)
//printf("%02X ", *((uint8_t *)header.address() + byte));
//std::cout << header.size() << " ";
			}

		/*
			Write out each postings list segment.
		*/
//std::cout << ": ";
		for (const auto &segment : compressed_segments)
			{
			postings.write(segment.address(), segment.size());

//for (size_t byte = 0; byte < segment.size(); byte++)
//printf("%02X ", *((uint8_t *)segment.address() + byte));
			}

//std::cout << "\n\n";
		/*
			Return the location of the postings list on disk
		*/
		return postings_location;
		}

	/*
		SERIALISE_JASS_V2::OPERATOR()()
		-------------------------------
	*/
	void serialise_jass_v2::operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
		{
//std::cout << term;
		/*
			Write the postings list to disk and keep a track of where it is.
		*/
		size_t number_of_impact_scores;

		size_t postings_location;
		postings_location = write_postings(postings, number_of_impact_scores, document_frequency, document_ids, term_frequencies);

		/*
			Find out where we are in the vocabulary strings file - which will be the start of the term before we write it.
		*/
		uint64_t term_offset = vocabulary_strings.tell();

		/*
			Write the vocabulary term to CIvocab_terms.bin
		*/
		vocabulary_strings.write(term.address(), term.size());
		vocabulary_strings.write("\0", 1);
		
		/*
			Keep a copy of the term and the detals of the postings list for later sorting and writing to CIvocab.bin
		*/
		index_key.push_back(vocab_tripple(term, term_offset, postings_location, number_of_impact_scores));
		}

	/*
		SERIALISE_JASS_V2::UNITTEST()
		-----------------------------
	*/
	void serialise_jass_v2::unittest(void)
		{
		/*
			Build an index.
		*/
		index_manager_sequential index;
		index_manager_sequential::unittest_build_index(index, unittest_data::ten_documents);
		
		/*
			Serialise the index.
		*/
		{
		serialise_jass_v1 serialiser(index.get_highest_document_id(), jass_v1_codex::qmx, 16);
		index.iterate(serialiser);
		}

		/*
			Checksum the index to make sure its correct.
		*/
		auto checksum = checksum::fletcher_16_file("CIvocab.bin");
//std::cout << "CIvocab.bin checksum:" << checksum << "\n";
		JASS_assert(checksum == 10231);

		checksum = checksum::fletcher_16_file("CIvocab_terms.bin");
//std::cout << "CIvocab_terms.bin checksum:" << checksum << "\n";
		JASS_assert(checksum == 25057);

		checksum = checksum::fletcher_16_file("CIpostings.bin");
//std::cout << "CIpostings.bin checksum:" << checksum << "\n";
		JASS_assert(checksum == 43058);

		checksum = checksum::fletcher_16_file("CIdoclist.bin");
//std::cout << "CIdoclist.bin checksum:" << checksum << "\n";
		JASS_assert(checksum == 3045);

		puts("serialise_jass_v2::PASSED");
		}
	}
