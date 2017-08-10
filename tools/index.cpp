/*
	INDEX.CPP
	---------
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
#include "instream_document_trec.h"
#include "index_manager_sequential.h"

/*
	Declare the command line parameters
*/
bool parameter_jass_v1_index = false;
auto parameter_jass_v1_index_command = JASS::commandline::parameter("-I1", "--index_jass_v1", "Generate a JASS version 1 index.", parameter_jass_v1_index);

bool parameter_compiled_index = false;
auto parameter_compiled_index_command = JASS::commandline::parameter("-Ic", "--index_compiled", "Generate a JASS compiled index.", parameter_compiled_index);

std::string parameter_filename = "";
auto parameter_filename_command = JASS::commandline::parameter("-f", "--filename", "Filename to index.", parameter_filename);

auto command_line_parameters = std::make_tuple
	(
	parameter_filename_command,
	parameter_jass_v1_index_command,
	parameter_compiled_index_command
	);

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	std::cout << JASS::version::build() << "\n";
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

	JASS::parser parser;
	JASS::document document;
	std::shared_ptr<JASS::instream> file(new JASS::instream_file(parameter_filename));
	std::shared_ptr<JASS::instream> source(new JASS::instream_document_trec(file));
	JASS::index_manager_sequential index;

	size_t total_documents = 0;
	
	do
		{
		document.rewind();
		source->read(document);
		if (document.isempty())
			break;
		total_documents++;
		if (total_documents % 10000 == 0)
			printf("Documents:%lld\n", (long long)total_documents);
		parser.set_document(document);
		index.begin_document(document.primary_key);

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
	
	printf("Documents:%lld\n", (long long)total_documents);

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

	return 0;
	}

