/*
	SERIALISE_CI.CPP
	----------------
	Copyright (c) 2017 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <ostream>

#include "slice.h"
#include "checksum.h"
#include "serialise_ci.h"
#include "index_postings.h"
#include "index_manager_sequential.h"

namespace JASS
	{
	/*
		SERIALISE_CI::SERIALISE_CI()
		----------------------------
	*/
	serialise_ci::serialise_ci() :
		postings_file("JASS_postings.cpp", "w+b"),
		postings_header_file("JASS_postings.h", "w+b"),
		vocab_file("JASS_vocabulary.cpp", "w+b"),
		terms(0)
		{
		vocab_file.write("#include <stdint.h>\n\n");
		vocab_file.write("#include\"JASS_postings.h\"\n");
		vocab_file.write("#include\"JASS_vocabulary.h\"\n");
		vocab_file.write("JASS_ci_vocab dictionary[] = {\n");

		postings_file.write("#include <stddef.h>\n");
		postings_file.write("#include <stdint.h>\n\n");
		postings_file.write("extern void add_rsv(size_t document_id, uint16_t impact);\n\n");
		}

	/*
		SERIALISE_CI::~SERIALISE_CI()
		-----------------------------
	*/
	serialise_ci::~serialise_ci()
		{
		vocab_file.write("};\n");

		std::ostringstream length;

		length << "uint64_t dictionary_length = " << terms << ";\n";
		vocab_file.write(length.str());
		}

	/*
		SERIALISE_CI::OPERATOR()()
		--------------------------
	*/
	void serialise_ci::operator()(const slice &term, const index_postings &postings)
		{
		std::ostringstream code;
		uint64_t previous_document_id = std::numeric_limits<uint64_t>::max();

		code << "void T_" << term << "()\n";
		code << "{\n";
		for (const auto &posting : postings)
			{
			if (std::get<0>(posting) != previous_document_id)
				{
				code << "add_rsv(" << std::get<0>(posting) << ',' << std::get<1>(posting) << ");\n";
				previous_document_id = std::get<0>(posting);
				}
			}
		code << "}\n";

		postings_file.write(code.str());
		vocab_file.write("{\"");
		vocab_file.write(term.address(), term.size());
		vocab_file.write("\",T_");
		vocab_file.write(term.address(), term.size());
		vocab_file.write("},\n");

		postings_header_file.write("void T_");
		postings_header_file.write(term.address(), term.size());
		postings_header_file.write("();\n");

		terms++;
		}

	/*
		SERIALISE_CI::UNITTEST()
		------------------------
	*/
	void serialise_ci::unittest(void)
		{
		/*
			Build an index.
		*/
		index_manager_sequential index;
		index_manager_sequential::unittest_build_index(index);
		
		/*
			Serialise the index.
		*/
		{
		serialise_ci serialiser;
		index.iterate(serialiser);
		}

		/*
			Checksum the inde to make sure its correct.
		*/
		std::cout << "=====\n";
		auto checksum = checksum::fletcher_16_file("JASS_postings.cpp");
		std::cout << "JASS_postings.c:" << checksum << '\n';

		checksum = checksum::fletcher_16_file("JASS_postings.h");
		std::cout << "JASS_postings.h:" << checksum << '\n';

		checksum = checksum::fletcher_16_file("JASS_vocabulary.cpp");
		std::cout << "JASS_vocabulary.c:" << checksum << '\n';
		std::cout << "=====\n";
		}
	}
