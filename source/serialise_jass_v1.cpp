/*
	SERIALISE_JASS_V1.CPP
	---------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include "checksum.h"
#include "serialise_jass_v1.h"
#include "index_manager_sequential.h"

namespace JASS
	{
	/*
		SERIALISE_JASS_V1::~SERIALISE_JASS_V1()
		---------------------------------------
	*/
	serialise_jass_v1::~serialise_jass_v1()
		{
		/*
			Sort then serialise the contents of the CIvocab.bin file.
		*/
		std::sort(index_key.begin(), index_key.end());
		
		/*
			Serialise the contents of CIvocab.bin
		*/
		for (const auto line : index_key)
			{
			vocabulary.write(&line.term, sizeof(line.term));
			vocabulary.write(&line.offset, sizeof(line.offset));
			vocabulary.write(&line.impacts, sizeof(line.impacts));
			}

		/*
			Serialise the primary key offsets and the numnber of documents in the collection.  This all goes into the primary key file CIdoclist.bin.
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
	size_t serialise_jass_v1::write_postings(const index_postings &postings_list, size_t &number_of_impacts)
		{
		/*
			Keep a track of where the postings are stored on disk.
		*/
		size_t postings_locaton = postings.tell();

		/*
			Impact order the postings list.
		*/
		const auto &impact_ordered = postings_list.impact_order(memory);

		/*
			Write out the number of impact headers we're going to see.
		*/
		uint64_t impact_count = number_of_impacts = impact_ordered.impact_size();
		postings.write(&impact_count, sizeof(impact_count));

		/*
			Write out each pointer to an impact header.
		*/
		uint64_t offset = postings_locaton + sizeof(impact_count) + impact_count * sizeof(uint64_t);
		uint64_t impact_header_size = sizeof(uint16_t) + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint32_t);
		for (size_t which = 0; which < impact_count; which++)
			{
			postings.write(&offset, sizeof(offset));
			offset += impact_header_size;
			}

		/*
			Write out each impact header.
		*/
		for (const auto &header : impact_ordered)
			{
			/*
				impact score (uint16_t).
			*/
			uint16_t score = header.impact_score;
			postings.write(&score, sizeof(score));

			/*
				start loction on disk (uint64_t).
			*/
			uint64_t start_location = offset;
			postings.write(&start_location, sizeof(start_location));

			/*
				This is where comnoression happens - but since we're not initially compressing no work is necessary.
			*/
			offset += header.size();

			/*
				end location on disk (uint64_t).
			*/
			uint64_t finish_location = offset;
			postings.write(&finish_location, sizeof(finish_location));

			/*
				the number of document ids with this impact score (length of the impact segment measured in doc_ids).
			*/
			uint32_t frequency = header.size();
			postings.write(&frequency, sizeof(frequency));
			}

		/*
			write out each postings list segment.
		*/
		for (const auto &header : impact_ordered)
			{
			postings.write(header.begin(), header.size());				// Since we're not compressed, just write them out as a single chunk.
			}

		/*
			return the location of the postings list on disk
		*/
		return postings_locaton;
		}

	/*
		SERIALISE_JASS_V1::OPERATOR()()
		-------------------------------
	*/
	void serialise_jass_v1::operator()(const slice &term, const index_postings &postings)
		{
		/*
			write the postings list to disk and keep a track of where it is.
		*/
		memory.rewind();
		size_t number_of_impact_scores;
		size_t postings_location = write_postings(postings, number_of_impact_scores);

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
			Keep a copy of the term and the detals of the postings list for later sorting and writeing to CIvocab.bin
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
		SERIALISE_JASS_V1::UNITTEST()
		-----------------------------
	*/
	void serialise_jass_v1::unittest(void)
		{
		/*
			Build an index.
		*/
		index_manager_sequential index;
		index_manager_sequential::unittest_build_index(index);
		
		/*
			Serialise the index.
		*/
		{
		serialise_jass_v1 serialiser;
		index.iterate(serialiser);
		}

		/*
			Checksum the inde to make sure its correct.
		*/
		std::cout << "=====";

		auto checksum = checksum::fletcher_16_file("CIvocab.bin");
		std::cout << "\tCIvocab.bin = " << checksum << '\n';

		checksum = checksum::fletcher_16_file("CIvocab_terms.bin");
		std::cout << "\tCIvocab_terms.bin = " << checksum << '\n';

		checksum = checksum::fletcher_16_file("CIpostings.bin");
		std::cout << "\tCIpostings.bin = " << checksum << '\n';

		checksum = checksum::fletcher_16_file("CIdoclist.bin");
		std::cout << "\tCIdoclist.bin = " << checksum << '\n';

		std::cout << "=====";

		puts("\tserialise_jass_v1::INCOMPLETE - not all implemented");
		}
	}
