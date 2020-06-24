/*
	TREC_TO_QUERY.CPP
	-----------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

	Read a TREC topic file and turn into a JASS query file.
*/

#include "channel_file.h"
#include "channel_trec.h"
/*
	USAGE()
	-------
*/
int usage(char *exename)
	{
	std::cout << "Usage:" << exename << " <TRECTopicFile>\n";
	return 1;
	}
	
/*
	MAIN()
	------
*/
int main(int argc, char *argv[])
	{
	try
		{
		if (argc != 2)
			exit(usage(argv[0]));

		std::unique_ptr<JASS::channel> source(new JASS::channel_file(argv[1]));
		JASS::channel_trec infile(source, "t");

		std::string query;
		do
			{
			infile.gets(query);
			std::cout << query << '\n';
			}
		while (query.size() != 0);
		}
	catch (...)
		{
		puts("Unhandled Exception");
		return 1;
		}

	return 0;
	}
