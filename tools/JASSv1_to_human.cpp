/*
	JASSV1_TO_HUMAN.CPP
	-------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/

#include <stdio.h>

#include "file.h"

struct vocab_tripple
	{
	uint64_t term;
	uint64_t offset;
	uint64_t impacts;
	};

/*
	MAIN()
	------
*/
int main(void)
	{
	std::string vocab_buffer;
	JASS::file::read_entire_file("CIvocab.bin", vocab_buffer);
	const vocab_tripple *vocab = reinterpret_cast<const vocab_tripple *>(vocab_buffer.c_str());
	auto vocab_length = vocab_buffer.size() / sizeof(vocab_tripple);


	std::string strings;
	JASS::file::read_entire_file("CIvocab_terms.bin", strings);

	for (auto term = 0; term < vocab_length; term++)
		{
		puts(strings.c_str() + vocab[term].term);
		}
	}
