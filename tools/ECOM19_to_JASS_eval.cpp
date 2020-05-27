/*
	ECOM19_TO_JASS_EVAL.CPP
	-----------------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

*/
/*!
	@file
	@brief Read the eBay released assessments (which is a table) and convert into JASS_eval format (a superset of trec_eval format)
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman
*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <map>
#include <string>
#include <vector>

#include "file.h"

/*
	USAGE()
	-------
*/
int usage(const char *exename)
	{
	printf("Usage:%s <docfile> <assessmentsfile>\n", exename);
	return 1;
	}

/*
	MAIN()
	------
*/
int main(int argc, const char *argv[])
	{
	if (argc != 3)
		exit(usage(argv[0]));

	/*
		Load the documents
	*/
	std::string documents;
	JASS::file::read_entire_file(argv[1], documents);
	if (documents == "")
		exit(printf("Cannot read documents from file:%s", argv[1]));

	/*
		Load the assessments
	*/
	std::string assessments;
	JASS::file::read_entire_file(argv[2], assessments);
		if (assessments == "")
			exit(printf("Cannot read documents from file:%s", argv[1]));

	/*
		Break the documents file into individual documents
	*/
	std::vector<uint8_t *> document_lines;
	JASS::file::buffer_to_list(document_lines, documents);

	/*
		Parse each document and output the id and the price (column 1 and column 2)
	*/
	for(uint8_t *line : document_lines)
		{
		if (line == *document_lines.begin())
			continue;
		long document_id = atol((char *)line);
		char *space = strchr((char *)line, '\t');
		while (isspace(*space))
			space++;
		double price = atof(space);
		printf("PRICE 0 %ld %2.2f\n", document_id, price);
		}

	/*
		Break the assessments file into individual assessments
	*/
	std::vector<uint8_t *> assessment_lines;
	JASS::file::buffer_to_list(assessment_lines, assessments);

	/*
		Parse each document and output the id and the price (column 1 and column 2)
		Since the assessments are a table where the columns are the query numbers, we need to flip them in order to
		get trec_eval compatible assessments.  To do this we store them hashed first by query_id then by document_id
	*/
	std::map<int, std::map<int, int>> assessment_table;

	std::vector<long> query_id;
	for(uint8_t *line : assessment_lines)
		{
		/*
			Extract the query numbers
		*/
		if (line == *assessment_lines.begin())
			{
			for (char *column = strchr((char *)line, '\t'); column != NULL; column = strchr(column, '\t'))
				{
				while (isspace(*column))
					column++;
				query_id.push_back(atol(column));
				}
			continue;
			}

		/*
			Extract the assessments for this document
		*/
		size_t column_number = 0;
		long document_id = atol((char *)line);

		for (char *column = strchr((char *)line, '\t'); column != NULL; column = strchr(column, '\t'))
			{
			while (isspace(*column))
				column++;
			long assessment = atol(column);
			if (assessment != 0)
				assessment_table[query_id[column_number]][document_id] = assessment == 1 ? 1 : 0;

			column_number++;
			}
		}
		
	/*
		Output the sorted assessments
	*/
	for (const auto &[qid, document_list] : assessment_table)
		for (const auto &[document_id, assessment] : document_list)
			printf("%d 0 %d %d\n", qid, document_id, assessment);

	return 0;
	}
