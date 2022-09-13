/*
	SERIALISE_JASS_V1.CPP
	---------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include "reverse.h"
#include "checksum.h"
#include "allocator.h"
#include "serialise_jass_v1.h"
#include "compress_integer_all.h"
#include "index_manager_sequential.h"

namespace JASS
	{
	/*
		SERIALISE_JASS_V1::FINISH()
		---------------------------
	*/
	void serialise_jass_v1::finish(void)
		{
		/*
			Serialise the pointers into the vacabulary (the CIvocab.bin file)
		*/
		serialise_vocabulary_pointers();

		/*
			Serialise the primary key offsets and the number of documents in the collection.
		*/
		serialise_primary_keys();
		}

	/*
		SERIALISE_JASS_V1::SERIALISE_VOCABULARY_POINTERS()
		--------------------------------------------------
	*/
	void serialise_jass_v1::serialise_vocabulary_pointers(void)
		{
		/*
			Sort
		*/
		std::sort(index_key.begin(), index_key.end());

		/*
			Serialise the contents of CIvocab.bin
		*/
		for (const auto &line : index_key)
			{
			vocabulary.write(&line.term, sizeof(line.term));
			vocabulary.write(&line.offset, sizeof(line.offset));
			vocabulary.write(&line.impacts, sizeof(line.impacts));
			}
		}

	/*
		SERIALISE_JASS_V1::SERIALISE_PRIMARY_KEYS()
		-------------------------------------------
	*/
	void serialise_jass_v1::serialise_primary_keys(void)
		{
		/*
			Serialise the primary key offsets and the number of documents in the collection.  This all goes into the primary key file CIdoclist.bin.
			As JASS v2 counts from 1 but JASS v1 counts from 0, we have to drop the first (blank) element and subtract 1 from the count
		*/
		uint64_t document_count = primary_key_offsets.size() - 1;
		primary_keys.write(&primary_key_offsets[1], sizeof(primary_key_offsets[1]) * document_count);
		primary_keys.write(&document_count, sizeof(document_count));
		}

	/*
		SERIALISE_JASS_V1::WRITE_POSTINGS()
		-----------------------------------
	*/
	size_t serialise_jass_v1::write_postings(const index_postings &postings_list, size_t &number_of_impacts, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
		{
		/*
			Keep a track of where the postings are stored on disk.
		*/
		size_t postings_location = postings.tell();

		/*
			Impact order the postings list.
		*/
		postings_list.impact_order(documents, impact_ordered, document_frequency, document_ids, term_frequencies);

		/*
			Compute the number of impact headers we're going to see.
		*/
		number_of_impacts = impact_ordered.impact_size();

		/*
			Write out each pointer to an impact header.
		*/
		uint64_t offset = postings_location + number_of_impacts * sizeof(offset);
		uint64_t impact_header_size = sizeof(uint16_t) + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t);
		for (size_t which = 0; which < number_of_impacts; which++)
			{
			postings.write(&offset, sizeof(offset));
			offset += impact_header_size;
			}

		/*
			Write out each impact header, and compress as we go so we know where the data will be stored.
		*/
		size_t start_of_postings = offset + impact_header_size;									// +1 because there's a 0 terminator at the end
		auto wastage = allocator::realign(start_of_postings, alignment);						// Pad the start of the postings to be on a word boundary
		start_of_postings += wastage;

		uint8_t *compress_into = &compressed_buffer[0];
		auto compress_into_size = compressed_buffer.size();
		compressed_segments.clear();
		for (auto &header : reverse(impact_ordered))
			{
			/*
				Impact score (uint16_t).
			*/
			uint16_t score = static_cast<uint16_t>(header.impact_score);
			postings.write(&score, sizeof(score));

			/*
				Start loction on disk (uint64_t).
			*/
			uint64_t start_location = start_of_postings;

			postings.write(&start_location, sizeof(start_location));

			/*
				This is where compression happens.
				First D1 encode, then use the encoder to compress.
			*/
			compress_integer::d1_encode(header.begin(), header.begin(), header.size());
			*header.begin() -= 1;			// JASS v1 counts documents from 0.
			auto took = encoder->encode(compress_into, compress_into_size, header.begin(), header.size());
			if (took == 0)
				{
				/*
					Compression failed - exit
				*/
				std::cout <<  "Failed to compress postings list while serialising" << std::ends;
				exit(1);
				}
				
			/*
				Round up to the next word-aligned boundary
			*/
			auto padding = allocator::realign(took, alignment);

			/*
				Store it for writing out later
			*/
			compressed_segments.push_back(slice(compress_into, took + padding));
			compress_into += took;
			compress_into_size -= took;

			/*
				End location on disk (uint64_t).
			*/
			uint64_t finish_location = start_of_postings + took;
			postings.write(&finish_location, sizeof(finish_location));

			/*
				The number of document ids with this impact score (length of the impact segment measured in doc_ids).
			*/
			uint32_t frequency = static_cast<uint32_t>(header.size());
			postings.write(&frequency, sizeof(frequency));

			start_of_postings = finish_location + padding;
			}

		/*
			Write out a "blank" impact header
		*/
		uint8_t zero[] = {0, 0,  0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0};
		postings.write(&zero, sizeof(zero));

		/*
			Pad so that the postings are on a word boundary
		*/
		postings.write(zero, wastage);			

		/*
			Write out each postings list segment.
		*/
		for (auto &header : compressed_segments)
			postings.write(header.address(), header.size());

		/*
			Return the location of the postings list on disk
		*/
		return postings_location;
		}

	/*
		SERIALISE_JASS_V1::OPERATOR()()
		-------------------------------
	*/
	void serialise_jass_v1::operator()(const slice &term, const index_postings &postings, compress_integer::integer document_frequency, compress_integer::integer *document_ids, index_postings_impact::impact_type *term_frequencies)
		{
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
		SERIALISE_JASS_V1::DELEGATE::OPERATOR()()
		-----------------------------------------
	*/
	void serialise_jass_v1::operator()(size_t document_id, const slice &primary_key)
		{
		primary_key_offsets.push_back(primary_keys.tell());
		primary_keys.write(primary_key.address(), primary_key.size());
		primary_keys.write("\0", 1);
		}

	/*
		SERIALISE_JASS_V1::GET_COMPRESSOR()
		-----------------------------------
	*/
	std::unique_ptr<compress_integer> serialise_jass_v1::get_compressor(jass_v1_codex codex, std::string &name, int32_t &d_ness)
		{
		d_ness = 1;
		switch (codex)
			{
			case elias_gamma_simd_vb:
				name = "Group Elias Gamma SIMD with Variable Byte";
				break;
			case serialise_jass_v1::jass_v1_codex::elias_delta_simd:
				name = "Group Elias Delta SIMD";
				break;
			case serialise_jass_v1::jass_v1_codex::elias_gamma_simd:
				name = "Group Elias Gamma SIMD";
				break;
			case serialise_jass_v1::jass_v1_codex::qmx:
				name = "QMX JASS v1";
				break;
			case serialise_jass_v1::jass_v1_codex::uncompressed:
				name = "None";
				d_ness = 0;
				break;
			default:
				exit(printf("Unknown index format\n"));
			}

		return compress_integer_all::get_by_name(name);
		}

	/*
		SERIALISE_JASS_V1::UNITTEST()
		-----------------------------
	*/
	void serialise_jass_v1::unittest(void)
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
		serialiser.finish();
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

		puts("serialise_jass_v1::PASSED");
		}
	}
