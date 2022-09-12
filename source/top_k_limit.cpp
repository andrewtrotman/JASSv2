/*
	TOP_K_LIMIT.CPP
	---------------
	Copyright (c) 2021 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdlib.h>
#include <string.h>

#include "file.h"
#include "top_k_limit.h"

namespace JASS
	{
	/*
		TOP_K_LIMIT::TOP_K_LIMIT()
		--------------------------
	*/
	top_k_limit::top_k_limit(std::string &filename)
		{
		std::string file_contents;
		std::vector<uint8_t *> line_list;

		/*
			If we have no filename then do no work
		*/
		if (filename.empty())
			return;

		/*
			Read the file and break it into lines
		*/
		file::read_entire_file(filename, file_contents);
		file::buffer_to_list(line_list, file_contents);

		for (auto &line : line_list)
			{
			/*
				The line starts with the query id
			*/
			uint8_t *query_id = line;

			/*
				Then has a space
			*/
			uint8_t *rsv_position = (uint8_t *)strchr((char *)query_id, ' ');
			*rsv_position++ = '\0';
			while (isspace(*rsv_position))
				rsv_position++;

			/*
				Then an rsv
			*/
			uint32_t rsv = strtoul((char *)rsv_position, nullptr, 10);
			table[(char *)query_id] = rsv;
			}

		}

	}
