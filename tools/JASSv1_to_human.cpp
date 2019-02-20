/*
	JASSV1_TO_HUMAN.CPP
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@brief Dump a human-readable version of a JASS v1 index to standard out.
*/

#include <stdio.h>
#include <stdint.h>

#include <iostream>

#include "file.h"
#include "decode_d0.h"
#include "decode_d1.h"
#include "deserialised_jass_v1.h"

/*
	CLASS PRINTER
	-------------
*/
/*!
	@brief Implementation of add_rsv() that prints an individual posting
*/
class printer
	{
	public:
		/*
			PRINTER::ADD_RSV()
			------------------
		*/
		/*!
			@brief print the posting
			@param document [in] The document identifier (docid) used internally.
			@param impact [in] The impact score of this term in this document.
		*/
		void add_rsv(uint64_t document, uint64_t impact)
			{
			std::cout << '[' << document << ',' << impact << ']';
			}
	} ;


/*
	WALK_INDEX
	----------
*/
/*!
	@brief Walk the index, term by term, and print each posting from each postings list.
	@param index [in] Reference to a JASS v1 deserialised index object.
	@param decompressor [in] reference to an object that can decompress a postings list segment.
*/
template <typename DECODER>
void walk_index(JASS::deserialised_jass_v1 &index, JASS::compress_integer &decompressor)
	{
	printer out_stream;
	DECODER decoder(index.document_count() + 4096);		// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

	/*
		Walk each term
	*/
	for (const auto &term : index)
		{
		std::cout << term.term;
		/*
			Walk each segment
		*/
		for (uint64_t current_segment = 0; current_segment < term.impacts; current_segment++)
			{
			uint64_t *postings = (uint64_t *)term.offset + current_segment;
			const JASS::deserialised_jass_v1::segment_header &header = *reinterpret_cast<const JASS::deserialised_jass_v1::segment_header *>(index.postings() + *postings);

			decoder.decode(decompressor, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);
			decoder.process(header.impact, out_stream);
			}
		std::cout << '\n';
		}
	}

/*
	MAIN()
	------
*/
/*!
	@brief Dump a human-readable version of a JASS v1 index to standard out.
*/
int main(void)
	{

	/*
		Move to using JASS library methods for processing the postings lists
	*/
	JASS::deserialised_jass_v1 index(true);
	index.read_index();

	/*
		Get the encoding scheme and the d-ness of the index
	*/
	std::string codex_name;
	index.codex(codex_name);
	JASS::compress_integer &decompressor = index.codex(codex_name);
	uint32_t d_ness = codex_name == "None" ? 0 : 1;			// d_ness of other than 0 or 1 is (currently) invalid

	std::cout << "\nPOSTINGS LISTS\n-------------\n";

	if (d_ness == 0)
		walk_index<JASS::decoder_d0>(index, decompressor);
	else
		walk_index<JASS::decoder_d1>(index, decompressor);

	std::cout << "\nPRIMARY KEY LIST\n----------------\n";
	for (const auto &key : index.primary_keys())
		std::cout << key << '\n';

	return 0;
	}
