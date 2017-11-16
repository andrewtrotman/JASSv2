/*
	JASS_INDEX.CPP
	--------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
/*!
	@file
	@brief The JASS indexer
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman
*/
#include <string.h>

#include "parser.h"
#include "version.h"
#include "commandline.h"
#include "serialise_ci.h"
#include "instream_file.h"
#include "instream_memory.h"
#include "serialise_jass_v1.h"
#include "serialise_integers.h"
#include "instream_document_trec.h"
#include "index_manager_sequential.h"

/*
	Declare the command line parameters
*/
bool parameter_jass_v1_index = false;
bool parameter_compiled_index = false;
bool parameter_uint32_index = false;
std::string parameter_filename = "";
bool parameter_quiet = false;
bool parameter_help = false;
size_t parameter_report_every_n = (std::numeric_limits<size_t>::max)();

auto command_line_parameters = std::make_tuple
	(
	JASS::commandline::note("\nMISCELLANEOUS\n-------------"),
	JASS::commandline::parameter("-q", "--nologo", "Suppress the banner.", parameter_quiet),
	JASS::commandline::parameter("-?", "--help", "Print this help.", parameter_help),
	JASS::commandline::parameter("-h", "--help", "Print this help.", parameter_help),
	JASS::commandline::parameter("-H", "--help", "Print this help.", parameter_help),

	JASS::commandline::note("\nREPORTING\n---------"),
	JASS::commandline::parameter("-N", "--report-every", "<n> Report time and memory every <n> documents.", parameter_report_every_n),

	JASS::commandline::note("\nFILE HANDLING\n-------------"),
	JASS::commandline::parameter("-f", "--filename", "<filename> Filename to index.", parameter_filename),

	JASS::commandline::note("\nINDEX GENERATION\n----------------"),
	JASS::commandline::parameter("-I1", "--index_jass_v1", "Generate a JASS version 1 index.", parameter_jass_v1_index),
	JASS::commandline::parameter("-Ic", "--index_compiled", "Generate a JASS compiled index.", parameter_compiled_index),
	JASS::commandline::parameter("-Ib", "--index_binary", "Generate a binary dump of just the postings segments.", parameter_uint32_index)
	);

/*
	USAGE()
	-------
*/
uint8_t usage(const std::string &exename)
	{
	std::cout << JASS::commandline::usage(exename, command_line_parameters) << "\n";
	return 1;
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	/*
		Do the command line parsing.
	*/
	std::string error;
	auto success = JASS::commandline::parse(argc, argv, command_line_parameters, error);
	if (!success)
		{
		std::cout << error;
		exit(1);
		}

	if (!parameter_quiet)
		std::cout << JASS::version::build() << "\n";

	if (parameter_filename == "")
		std::cout << "filename needed";

	if (parameter_filename == "" || parameter_help)
		exit(usage(argv[0]));

	/*
		Now call JASS
	*/
	JASS::parser parser;
	JASS::document document;
	std::shared_ptr<JASS::instream> file(new JASS::instream_file(parameter_filename));
	std::shared_ptr<JASS::instream> source(new JASS::instream_document_trec(file));
	JASS::index_manager_sequential index;

	size_t total_documents = 0;

	/*
		Parse the instream to get document (which are then indexed)
	*/
	do
		{
		/*
			Reuse memory from before
		*/
		document.rewind();

		/*
			get the next document
		*/
		source->read(document);
		if (document.isempty())
			break;
		total_documents++;
		if (total_documents % parameter_report_every_n == 0)
			std::cout << "Documents:" << total_documents << '\n';

		/*
			parse the current document
		*/
		parser.set_document(document);
		index.begin_document(document.primary_key);

		/*
			Process each token
		*/
		bool finished = false;
		do
			{
			const auto &token = parser.get_next_token();
			
			switch (token.type)
				{
				case JASS::parser::token::eof:
					finished = true;
					break;
				case JASS::parser::token::alpha:
					index.term(token);
					break;
				case JASS::parser::token::numeric:
					index.term(token);
					break;
				case JASS::parser::token::xml_start_tag:
					break;
				case JASS::parser::token::xml_end_tag:
					break;
				default:
					break;
				}
			}
		while (!finished);
		index.end_document();
		}
	while (!document.isempty());
	
	std::cout << "Documents:" << total_documents << '\n';

	/*
		Do we need to generate a compiled index?
	*/
	if (parameter_compiled_index)
		{
		JASS::serialise_ci serialiser;
		index.iterate(serialiser);
		}

	/*
		Do we need to generate a JASS v1 index?
	*/
	if (parameter_jass_v1_index)
		{
		JASS::serialise_jass_v1 serialiser;
		index.iterate(serialiser);
		}

	/*
		Do we need to generate a binary (uint32_t) dump of just the postings lists?
	*/
	if (parameter_uint32_index)
		{
		JASS::serialise_integers serialiser;
		index.iterate(serialiser);
		}

	return 0;
	}
