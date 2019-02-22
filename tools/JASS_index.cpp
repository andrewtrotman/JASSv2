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

#include <vector>

#include "timer.h"
#include "parser.h"
#include "version.h"
#include "quantize.h"
#include "commandline.h"
#include "serialise_ci.h"
#include "instream_file.h"
#include "instream_memory.h"
#include "compress_integer.h"
#include "serialise_jass_v1.h"
#include "serialise_integers.h"
#include "instream_document_trec.h"
#include "index_manager_sequential.h"
#include "ranking_function_atire_bm25.h"

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
bool parameter_atire_similar = false;

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

	JASS::commandline::note("\nCOMPATIBILITY\n-------------"),
	JASS::commandline::parameter("-A", "--atire", "ATIRE-like parsing (errors and all)", parameter_atire_similar),

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
	auto timer = JASS::timer::start();			// elapsed time since start (excluding static initialisers)

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

	/*
		If we're not in quiet mode then dump the copyright message
	*/
	if (!parameter_quiet)
		std::cout << JASS::version::build() << "\n";

	/*
		Decode the input filename
	*/
	if (parameter_filename == "")
		std::cout << "filename needed";

	/*
		Provide help if needed.
	*/
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

	auto preamble_time = JASS::timer::stop(timer).nanoseconds();

	/*
		Parse the instream to get document (which are then indexed)
	*/
	uint64_t collection_length = 0;		// measured in terms
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
			{
			auto took = JASS::timer::stop(timer).nanoseconds();
			std::cout << "Documents:" << total_documents << " in:" << took << " ns" << "\n";
			}

		/*
			parse the current document
		*/
		parser.set_document(document);
		index.begin_document(document.primary_key);

		/*
			Process each token
		*/
		bool finished = false;
		JASS::compress_integer::integer document_length = 0;				// measured in terms
		do
			{
			const auto &token = parser.get_next_token();
			
			switch (token.type)
				{
				case JASS::parser::token::eof:
					finished = true;
					break;
				case JASS::parser::token::alpha:
					document_length++;
					index.term(token);
					break;
				case JASS::parser::token::numeric:
					document_length++;
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
		collection_length += document_length;

		/*
			ATIRE has a bug that results in the document length calculation being off by one (one too large in ATIRE)
		*/
		index.end_document(document_length + (parameter_atire_similar ? 1 : 0));
		}
	while (!document.isempty());

	auto time_to_end_parse = JASS::timer::stop(timer).nanoseconds();

	std::cout << "Documents:" << total_documents << '\n';
	std::cout << "Terms    :" << collection_length << '\n';

	/*
		quantize the index
	*/
	std::shared_ptr<JASS::ranking_function_atire_bm25> ranker(new JASS::ranking_function_atire_bm25(0.9, 0.4, index.get_document_length_vector()));
	JASS::quantize<JASS::ranking_function_atire_bm25> quantizer(total_documents, ranker);
	index.iterate(quantizer);
	auto time_to_end_quantization = JASS::timer::stop(timer).nanoseconds();

	/*
		Decode the export formats and encode into a vector
	*/
	std::vector<std::unique_ptr<JASS::index_manager::delegate>> exporters;
	if (parameter_compiled_index)
		exporters.push_back(std::make_unique<JASS::serialise_ci>(index.get_highest_document_id()));
	if (parameter_jass_v1_index)
		exporters.push_back(std::make_unique<JASS::serialise_jass_v1>(index.get_highest_document_id()));
	if (parameter_uint32_index)
		exporters.push_back(std::make_unique<JASS::serialise_integers>(index.get_highest_document_id()));

	/*
		Write out the index in the desired formats.
	*/
	if (exporters.size() != 0)
		quantizer.serialise_index(index, exporters);

	/*
		Dump the statistics to the console.
	*/
	auto time_to_end = JASS::timer::stop(timer).nanoseconds();
	auto parse_time = time_to_end_parse - preamble_time;
	auto quantization_time = time_to_end_quantization - time_to_end_parse;
	auto serialise_time = time_to_end - time_to_end_quantization;

	std::cout << "Preamble time    :" << preamble_time << "ns (" << preamble_time / 1000000000 << " seconds)\n";
	std::cout << "Parse time       :" << parse_time << "ns (" << parse_time / 1000000000 << " seconds)\n";
	std::cout << "Quantization time:" << quantization_time << "ns (" << quantization_time / 1000000000 << " seconds)\n";
	std::cout << "Serialise time   :" << serialise_time << "ns (" << serialise_time / 1000000000 << " seconds)\n";
	std::cout << "=================\n";
	std::cout << "Total time       :" << time_to_end << "ns (" << time_to_end / 1000000000 << " seconds)\n";

	/*
		Done.
	*/
	return 0;
	}
