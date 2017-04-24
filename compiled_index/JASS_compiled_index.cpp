
/*
	COMP"ILED_INDEX.CPP
	-------------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <algorithm>

#include "JASS_vocabulary.h"
#include "accumulator_2d.h"

JASS::accumulator_2d<uint16_t> accumulators(1024);

/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	std::sort(&dictionary[0], &dictionary[dictionary_length]);
	}

void add_rsv(size_t document_id, uint16_t weight)
	{
	accumulators[document_id] += weight;
	}
