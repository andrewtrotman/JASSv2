/*
	EVALUATE.CPP
	------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include <cmath>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "file.h"
#include "asserts.h"
#include "evaluate.h"

namespace JASS
	{
	const evaluate::judgement_less_than evaluate::judgement_less_than_comparator;

	/*
		EVALUATE::DECODE_ASSESSMENTS_TREC_QRELS()
		-----------------------------------------
	*/
	void evaluate::decode_assessments_trec_qrels(std::string &assessment_file)
		{
		/*
			Turn into a list of individual assessments.
		*/
		std::vector<uint8_t *> line_list;
		file::buffer_to_list(line_list, assessment_file);

		/*
			Convert each assessment from the file into an assessment object
			Each line in the file looks like:

		 	1 0 AP880212-0161 0

			where the first column is the topic (or query) number, the second coumn is the "iteration" (and unused),
			the third column is the document primary key, and the fourth column is the relevance (1 for relevant, 0 for
			not relevant).  Unassessed documents are assumed to be non-relevant.
		*/
		for (auto &line : line_list)
			if (char *column_1 = reinterpret_cast<char *>(line))
				{
				while (isspace(*column_1))
					column_1++;

				if (char *column_2 = strchr(column_1, ' '))
					{
					*column_2++ = '\0';
					while (isspace(*column_2))
						column_2++;

					if (char *column_3 = strchr(column_2, ' '))
						{
						*column_3++ = '\0';
						while (isspace(*column_3))
							column_3++;

						if (char *column_4 = strchr(column_3, ' '))
							{
							*column_4++ = '\0';
							while (isspace(*column_4))
								column_4++;

							char *query_id = column_1;
							char *document_id = column_3;
							double relevance = atof(column_4);

							assessments.push_back(judgement(query_id, document_id, relevance));
							}
						}
					}
				}

		/*
			now sort the assessments
		*/
		std::sort(assessments.begin(), assessments.end());
		}

	/*
		EVALUATE::LOAD_ASSESSMENTS_TREC_QRELS()
		---------------------------------------
	*/
	void evaluate::load_assessments_trec_qrels(std::string &filename)
		{
		/*
			Read the assessments file.
		*/
		std::string assessment_file;
		file::read_entire_file(filename, assessment_file);

		/*
			decode the assessments file.
		*/
		decode_assessments_trec_qrels(assessment_file);
		}

	/*
		EVALUATE::UNITTEST()
		--------------------
	*/
	void evaluate::unittest(void)
		{
		/*
			Sample answer
		*/
		std::string correct_answer =
			"1 AP880216-0139 1\n"
			"1 AP880216-0169 0\n"
			"1 AP880217-0026 0\n"
			"1 AP880217-0030 0\n"
			"2 AP880212-0161 0\n";

		/*
			Load the sample data
		*/
		evaluate container;
		std::string copy = unittest_data::five_trec_assessments;
		container.decode_assessments_trec_qrels(copy);

		/*
			Construct the answer by concatinating the results of parsing
		*/
		std::ostringstream constructed_answer;
		for (const auto &judgement : container.assessments)
			constructed_answer << judgement.query_id << ' ' << judgement.document_id << ' ' << judgement.score << '\n';

		/*
			Make sure we're correct
		*/
		JASS_assert(constructed_answer.str() == correct_answer);

		puts("evaluate::PASSED");
		}
	}
