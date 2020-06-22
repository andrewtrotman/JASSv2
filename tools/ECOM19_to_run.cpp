/*
	ECOM19_TO_RUN.CPP
	-----------------
	Copyright (c) 2019 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)

*/
/*!
	@file
	@brief Read a run from the ECOM19 eBay data challenge and convert into trec_eval format
	@author Andrew Trotman
	@copyright 2019 Andrew Trotman

*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "file.h"

/*
	CLASS QUERY_DOCUMENT_PRICE
	--------------------------
*/
class query_document_price
	{
	public:
		size_t query;
		size_t document;
		double price;

	public:

		/*
			QUERY_DOCUMENT_PRICE::QUERY_DOCUMENT_PRICE()
			--------------------------------------------
		*/
		query_document_price(size_t query, size_t document, double price) :
			query(query),
			document(document),
			price(price)
			{
			/* Nothing */
			}

		/*
			QUERY_DOCUMENT_PRICE::OPERATOR<()
			---------------------------------
		*/
		bool operator<(const query_document_price &second) const
			{
			if (query < second.query)
				return true;
			else if (query > second.query)
				return false;

			if (price < second.price)
				return true;
			else if (price > second.price)
				return false;

			if (document < second.document)
				return true;
			else
				return false;
			}
	};

/*
	READ_MATRIX()
	-------------
*/
void read_matrix(std::string &data, std::unordered_map<int, std::unordered_map<int, int>> &table, int include_minus_1 = true)
	{
	size_t document_id = 0;

	std::vector<uint8_t *> file_lines;
	JASS::file::buffer_to_list(file_lines, data);

	size_t row_number = 0;
	for (auto line : file_lines)
		{
		size_t column_number = 0;
		uint8_t *column = line;
		bool break_at_end = false;
		while (!break_at_end)
			{
			while (isspace(*column))
				column++;
			uint8_t *start_of_data = column;
			while (*column != '\0' && !isspace(*column))
				column++;
			if (*column == '\0')
				break_at_end = true;
			*column++ = '\0';
			if (row_number == 0 || column_number == 0)
				{
				table[row_number][column_number] = atol((char *)start_of_data);
				if (column_number == 0)
					document_id = atol((char *)start_of_data);
				}
			else
				{
				size_t query_id = table[0][column_number];
				if (*start_of_data == '1')
					table[document_id][query_id] = 1;
				if (include_minus_1 && *start_of_data == '-')
					table[document_id][query_id] = -1;
				}
			column_number++;
			}
		row_number++;
		
		if (row_number % 100000 == 0)
			fprintf(stderr, "%d\n", (int)row_number);
		}
	}

/*
	USAGE()
	-------
*/
int usage(const char *exename)
	{
	printf("Usage:%s <docfile> <trainingdatafile> <runfile> <runname>\n", exename);
	return 1;
	}

/*
	MAIN_EVENT()
	------------
*/
int main_event(int argc, const char *argv[])
	{
	if (argc != 5)
		exit(usage(argv[0]));

	/*
		Load the documents
	*/
fprintf(stderr, "Load Documents\n");
	std::string documents;
	JASS::file::read_entire_file(argv[1], documents);
	if (documents == "")
		exit(printf("Cannot read documents from file:%s", argv[1]));

	/*
		Break the documents file into individual documents
	*/
	std::vector<uint8_t *> document_lines;
	JASS::file::buffer_to_list(document_lines, documents);

	/*
		Parse each document and output the id and the price (column 1 and column 2)
	*/
	std::unordered_map<int, double> prices;
	for(uint8_t *line : document_lines)
		{
		if (line == *document_lines.begin())
			continue;
		long document_id = atol((char *)line);
		char *space = strchr((char *)line, '\t');
		while (isspace(*space))
			space++;
		double price = atof(space);
		prices[document_id] = price;
		}

	/*
		Load the training data (which must be removed from a run)
	*/
fprintf(stderr, "Load training data");
	std::string training_data_text;
	JASS::file::read_entire_file(argv[2], training_data_text);
		if (training_data_text == "")
			exit(printf("Cannot read documents from file:%s", argv[2]));

	std::unordered_map<int, std::unordered_map<int, int>> training_data;
	read_matrix(training_data_text, training_data, true);

	/*
		Load the run
	*/
fprintf(stderr, "Load Run\n");
	std::string run_text;
	JASS::file::read_entire_file(argv[3], run_text);
		if (run_text == "")
			exit(printf("Cannot read documents from file:%s", argv[3]));

fprintf(stderr, "Matrix the Run\n");
	std::unordered_map<int, std::unordered_map<int, int>> run;
	read_matrix(run_text, run, false);

	/*
		Remove the training data from the run and convert the run into trec_eval format
	*/
fprintf(stderr, "Dump the Run\n");
	//coverity[format_changed]
	std::cout << std::setprecision(2) << std::fixed;
	for (size_t column = 1; column < run[0].size(); column++)
		{
		std::vector<query_document_price> answer;
		for (size_t row = 1; row < run.size(); row++)
			{
			size_t query_id = run[0][column];
			size_t document_id = run[row][0];

			if (run[document_id][query_id] == 1 && training_data[document_id][query_id] == 0)
				answer.push_back(query_document_price(query_id, document_id, prices[document_id]));
			}
		std::sort(answer.begin(), answer.end());
		size_t rank = 0;
		size_t results_list_length = answer.size();
		for (const auto &[query, document, price] : answer)
			{
			(void)price;		// unused, supress warning.
			rank++;
			std::cout << query << " Q0 " << document << ' ' << rank << ' ' << results_list_length - rank + 1 << ' ' << argv[4] << '\n';
			}
		}

	return 0;
	}

/*
	MAIN_EVENT()
	------------
*/
int main(int argc, const char *argv[])
	{
	try
		{
		return main_event(argc, argv);
		}
	catch (...)
		{
		puts("Unexpected exception");
		return 1;
		}
	}
