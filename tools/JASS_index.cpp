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
#include <filesystem>

#include "timer.h"
#include "parser.h"
#include "version.h"
#include "quantize.h"
#include "commandline.h"
#include "stem_porter.h"
#include "parser_fasta.h"
#include "serialise_ci.h"
#include "quantize_none.h"
#include "instream_file.h"
#include "instream_memory.h"
#include "instream_deflate.h"
#include "compress_integer.h"
#include "serialise_jass_v1.h"
#include "serialise_jass_v2.h"
#include "serialise_integers.h"
#include "parser_unicoil_json.h"
#include "instream_document_trec.h"
#include "instream_document_fasta.h"
#include "serialise_forward_index.h"
#include "index_manager_sequential.h"
#include "ranking_function_atire_bm25.h"
#include "instream_directory_iterator.h"
#include "instream_document_unicoil_json.h"

/*
	Declare the command line parameters
*/
bool parameter_jass_v1_index = false;
bool parameter_jass_v2_index = false;
bool parameter_compiled_index = false;
bool parameter_uint32_index = false;
bool parameter_forward_index = false;
std::string parameter_filename = "";
bool parameter_quiet = false;
bool parameter_help = false;
size_t parameter_report_every_n = (std::numeric_limits<size_t>::max)();
bool parameter_atire_similar = false;
size_t parameter_fasta_kmer_length = 0;

bool parameter_stem_porter = false;

bool parameter_document_format_trec = true;
bool parameter_document_format_JSON_uniCOIL = false;

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

	JASS::commandline::note("\nDOCUMENT FORMATS\n-------------"),
	JASS::commandline::parameter("-dt",  "--document_TREC", "TREC format: <DOC><DOCNO></DOCNO></DOC> formatted documents (default)", parameter_document_format_trec),
	JASS::commandline::parameter("-djc", "--document_JSON_uniCOIL", "JSON uniCOIL forward index format: {\"id\": \"0\", \"vector\": {\"term\": 94 }}", parameter_document_format_JSON_uniCOIL),

	JASS::commandline::note("\nCOMPATIBILITY\n-------------"),
	JASS::commandline::parameter("-A", "--atire", "ATIRE-like parsing (errors and all)", parameter_atire_similar),

	JASS::commandline::note("\nTERM PROCESSING\n---------------"),
	JASS::commandline::parameter("-tp", "--term_steming_porter", "Term stemming with Porter v1 (JASS implementation)", parameter_stem_porter),

	JASS::commandline::note("\nINDEX GENERATION\n----------------"),
	JASS::commandline::parameter("-I1", "--index_jass_v1", "Generate a JASS version 1 index.", parameter_jass_v1_index),
	JASS::commandline::parameter("-I2", "--index_jass_v2", "Generate a JASS version 2 index.", parameter_jass_v2_index),
	JASS::commandline::parameter("-Ib", "--index_binary", "Generate a binary dump of just the postings segments.", parameter_uint32_index),
	JASS::commandline::parameter("-Ic", "--index_compiled", "Generate a JASS compiled index.", parameter_compiled_index),
	JASS::commandline::parameter("-If", "--index_forward", "Generate a forward index.", parameter_forward_index),
	JASS::commandline::parameter("-IF", "--index_FASTA", "<k> Generate a k-mer index from FASTA documents.", parameter_fasta_kmer_length)
	);


/*
	ENUM DOCUMENT_FORMAT
	--------------------
*/
enum document_format
	{
	NONE,
	TREC,
	K_MER,
	JSON_uniCOIL
	};

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
	GET_DOCUMENT_FORMAT()
	---------------------
*/
document_format get_document_format()
	{
	if
		(
		parameter_fasta_kmer_length != 0 &&
		parameter_document_format_JSON_uniCOIL
		)
		return document_format::NONE;


	if (parameter_fasta_kmer_length != 0)
		return document_format::K_MER;
	else if (parameter_document_format_JSON_uniCOIL)
		return document_format::JSON_uniCOIL;
	else
		return document_format::TREC;
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

	document_format format = get_document_format();
	if (format == NONE)
		{
		std::cout << argv[0] << "only one input format at a time";
		exit(1);
		}

	/*
		Provide help if needed.
	*/
	if (parameter_filename == "" || parameter_help)
		exit(usage(argv[0]));

	/*
		If we're not in quiet mode then dump the copyright message
	*/
	if (!parameter_quiet)
		std::cout << JASS::version::build() << "\n";


	/*
		Check to make sure we'll actually be exporting the index
	*/
	if (!(parameter_jass_v2_index | parameter_jass_v1_index | parameter_uint32_index | parameter_compiled_index | parameter_forward_index | parameter_fasta_kmer_length))
		{
		std::cout << "You must specify an index file format or else no index will be generated\n";
		return 1;
		}

	/*
		Decode the input filename
	*/
	if (parameter_filename == "")
		std::cout << "filename needed";

	/*
		Set up the parser
	*/
	JASS::parser *parser;
	switch (format)
		{
		case TREC:
			parser = new JASS::parser();
			break;
		case K_MER:
			parser = new JASS::parser_fasta(parameter_fasta_kmer_length);
			break;
		case JSON_uniCOIL:
			parser = new JASS::parser_unicoil_json();
			break;
		default:
			std::cout << "Unknown parser type";
			exit(1);
			break;
		}

	/*
		Set up the input pipeline
	*/
	std::shared_ptr<JASS::instream> file(new JASS::instream_file(parameter_filename));
	JASS::instream *data_source;
	switch (format)
		{
		case TREC:
			data_source = new JASS::instream_document_trec(file);
			break;
		case K_MER:
			data_source = new JASS::instream_document_fasta(file);
			break;
		case JSON_uniCOIL:
			{
			if (std::filesystem::is_directory(std::filesystem::path(parameter_filename)))
				{
				std::shared_ptr<JASS::instream> source(new JASS::instream_directory_iterator(parameter_filename));
				data_source = new JASS::instream_document_unicoil_json(source);
				}
			else
				{
				std::shared_ptr<JASS::instream> deflater(new JASS::instream_deflate(file));
				data_source = new JASS::instream_document_unicoil_json(deflater);
				}
			break;
			}
		default:
			std::cout << "Unknown parser type";
			exit(1);
			break;
		}

	std::shared_ptr<JASS::instream> source(data_source);

	/*
		Set up the stemmer
	*/
	JASS::stem *stem = nullptr;
	if (parameter_stem_porter)
		stem = new JASS::stem_porter;

	/*
		Now call JASS
	*/
	JASS::index_manager_sequential index;
	JASS::document document;
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
		parser->set_document(document);
		index.begin_document(document.primary_key);

		/*
			Process each token
		*/
		bool finished = false;
		JASS::compress_integer::integer document_length = 0;				// measured in terms
		do
			{
			auto &token = const_cast<JASS::parser::token &>(parser->get_next_token());
//std::cout << "[" << token.lexeme << "," << token.count << "]\n";
			switch (token.type)
				{
				case JASS::parser::token::eof:
					finished = true;
					break;
				case JASS::parser::token::alpha:
					document_length++;
					if (stem != nullptr && token.lexeme.size() > 2)
						stem->tostem(token, token);
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

	JASS::quantize<JASS::ranking_function_atire_bm25> *quantizer;
	if (format == JSON_uniCOIL)
		quantizer = new JASS::quantize_none<JASS::ranking_function_atire_bm25>(total_documents, ranker);
	else
		{
		quantizer = new JASS::quantize<JASS::ranking_function_atire_bm25>(total_documents, ranker);
		index.iterate(*quantizer);
		}

	auto time_to_end_quantization = JASS::timer::stop(timer).nanoseconds();

	/*
		Decode the export formats and encode into a vector
	*/
	std::vector<std::unique_ptr<JASS::index_manager::delegate>> exporters;
	if (parameter_compiled_index)
		exporters.push_back(std::make_unique<JASS::serialise_ci>(index.get_highest_document_id()));
	if (parameter_jass_v1_index)
		exporters.push_back(std::make_unique<JASS::serialise_jass_v1>(index.get_highest_document_id()));
	if (parameter_jass_v2_index)
		exporters.push_back(std::make_unique<JASS::serialise_jass_v2>(index.get_highest_document_id()));
	if (parameter_uint32_index)
		exporters.push_back(std::make_unique<JASS::serialise_integers>(index.get_highest_document_id()));
	if (parameter_forward_index)
		exporters.push_back(std::make_unique<JASS::serialise_forward_index>(index.get_highest_document_id()));

	/*
		Write out the index in the desired formats.
	*/
	if (exporters.size() != 0)
		quantizer->serialise_index(index, exporters);

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

	delete parser;
	delete quantizer;

	/*
		Done.
	*/
	return 0;
	}
