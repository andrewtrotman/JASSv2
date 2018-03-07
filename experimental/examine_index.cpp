/*
	EXAMINE_INDEX.CPP
	-----------------
	Written bu Andrew Trotman.

	Pass over an index and compute statistis for it.
*/
#include <iostream>

#include "decode_d0.h"
#include "decode_d1.h"
#include "deserialised_jass_v1.h"

template <typename DECODER>
void process(JASS::deserialised_jass_v1 &index, JASS::compress_integer &decompressor)
	{
	auto decoder = new DECODER(index.document_count() + 4096);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

	/*
		Iterate over the vocabularay
	*/
	for (const auto &metadata : index)
		{
		std::cout << metadata.term << "->";

		/*
			Iterate over each segment of the postings list
		*/
		for (size_t impact = 0; impact < metadata.impacts; impact++)
			{
			const uint64_t *current_impact = (reinterpret_cast<const uint64_t *>(metadata.offset) + impact);
			const JASS::deserialised_jass_v1::segment_header &header = *reinterpret_cast<const JASS::deserialised_jass_v1::segment_header *>(index.postings() + *current_impact);
			std::cout << " " << header.impact;

			/*
				Decompress and iterate over the postings
			*/
			char seperator = ':';
			decoder->decode(decompressor, header.segment_frequency, index.postings() + header.offset, header.end - header.offset);
			for (const auto docid : *decoder)
				{
				std::cout << seperator << docid;
				seperator = ',';
				}
			std::cout << '\n';
			}
		}
	}

int main(void)
	{
	/*
		Read the index
	*/
	JASS::deserialised_jass_v1 index(true);
	index.read_index();

	std::string codex_name;
	JASS::compress_integer &decompressor = index.codex(codex_name);
	uint32_t d_ness = codex_name == "None" ? 0 : 1;
	std::cout << codex_name << "\n";

	if (d_ness == 0)
		process<JASS::decoder_d0>(index, decompressor);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow
	else
		process<JASS::decoder_d1>(index, decompressor);				// Some decoders write past the end of the output buffer (e.g. GroupVarInt) so we allocate enough space for the overflow

	return 0;
	}
