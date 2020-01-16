/*
	CHANNEL_TREC.CPP
	----------------
*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include "file.h"
#include "parser.h"
#include "channel_trec.h"
#include "channel_file.h"

namespace JASS
	{
	/*
		CHANNEL_TREC::CLEAN()
		---------------------
	*/
	void channel_trec::clean(std::string &clean_query, size_t number, const std::string &&raw_query)
		{
		parser parser;
		std::ostringstream answer;

		answer << number << ' ';
		parser.set_document(raw_query);
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
				case JASS::parser::token::numeric:
					answer.write(reinterpret_cast<char *>(token.get().address()), token.get().size());
					break;
				case JASS::parser::token::xml_start_tag:
				case JASS::parser::token::xml_end_tag:
				default:
					break;
				}
			}
		while (!finished);

		clean_query = answer.str();
		}

	/*
		CHANNEL_TREC::GETS()
		--------------------
	*/
	void channel_trec::gets(std::string &into)
		{
		size_t old_number;
		std::ostringstream raw_query;
		parser parser;
		std::ostringstream answer;

		if (at_eof)
			{
			into.clear();
			return;
			}

		for (;;)
			{
			if (read)
				{
				in_channel.gets(buffer);
				if (buffer.size() == 0)
					break;		// at end of input
				}
			parser.set_document(buffer);

			bool finished = false;
			const char *token_start;
			bool match = false;
			do
				{
				const auto &token = parser.get_next_token();

				switch (token.type)
					{
					case JASS::parser::token::eof:
						finished = true;
						break;
					case JASS::parser::token::alpha:
					case JASS::parser::token::numeric:
						if (match)
							{
							answer.write(reinterpret_cast<char *>(token.get().address()), token.get().size());
							answer << ' ';
							}
						break;
					case JASS::parser::token::xml_start_tag:
						match = false;
						token_start = reinterpret_cast<const char *>(token.get().address());
						if (strcmp(token_start, "num") == 0)
							{
							/*
								Pre-ClueWeb
							*/
							parser::token digits;
							do
								digits = parser.get_next_token();
							while (digits.type != JASS::parser::token::numeric);
							old_number = number;
							number = atol(reinterpret_cast<char *>(digits.get().address()));

							if (old_number >= 0)
								return clean(into, old_number, answer.str());
							}
						else if (strncmp(token_start, "topic number=", 13) == 0)
							{
							/*
								ClueWeb
							*/
							const auto &digits = parser.get_next_token();			// 700

							old_number = number;
							number = atol(reinterpret_cast<char *>(digits.get().address()));

							if (old_number >= 0)
								return clean(into, old_number, raw_query.str());
							}
						else if ((strncmp(token_start, "query", 5) == 0) && (tag.find('q') != std::string::npos))
								match = true;			// ClueWeb
						else if ((strncmp(token_start, "title", 5) == 0) && (tag.find('t') != std::string::npos))
								match = true;
						else if ((strncmp(token_start, "desc",  4) == 0) && (tag.find('d') != std::string::npos))
								match = true;
						else if ((strncmp(token_start, "narr",  4) == 0) && (tag.find('n') != std::string::npos))
								match = true;
						break;
					default:
						match = false;
						break;
					}
				}
			while (!finished);
			}

		at_eof = true;
		return clean(into, number, answer.str());
		}

		/*
			CHANNEL_TREC::UNITTEST()
			------------------------
		*/
		void channel_trec::unittest(void)
			{
			std::string example_file = "\n\
			<top>\n\
			<num> Number: 698\n\
\n\
			<title>\n\
			literacy rates Africa\n\
\n\
			<desc>\n\
			What are literacy rates in African countries?\n\
\n\
			<narr>\n\
			A relevant document will contain information about the\n\
			literacy rate in an African country.\n\
			General education levels that do not specifically include literacy rates\n\
			are not relevant.\n\
			</top>\n\
\n\
\n\
			<top>\n\
			<num> Number: 699\n\
\n\
			<title>\n\
			term limits\n\
\n\
			<desc>\n\
			What are the pros and cons of term limits?\n\
\n\
			<narr>\n\
			Relevant documents reflect an opinion on the value of term limits\n\
			with accompanying reason(s).  Documents that cite the status of term\n\
			limit legislation or opinions on the issue sans reasons for the opinion\n\
			are not relevant.\n\
			</top>\n\
\n\
\n\
			<top>\n\
			<num> Number: 700\n\
\n\
			<title>\n\
			gasoline tax U.S.\n\
\n\
			<desc>\n\
			What are the arguments for and against an increase in gasoline\n\
			taxes in the U.S.?\n\
\n\
			<narr>\n\
			Relevant documents present reasons for or against raising gasoline taxes\n\
			in the U.S.  Documents discussing rises or decreases in the price of\n\
			gasoline are not relevant.\n\
			</top>\n\
			";

			/*
				Generate a file with the data in it.
			*/
			auto filename = file::mkstemp("jass");
			file::write_entire_file(filename, example_file);

			channel_file infile(filename);
			channel_trec query_reader(infile, "t");

			std::string into;
			do
				{
				query_reader.gets(into);
				std::cout << into << "\n";
				}
			while (into.size() != 0);

			exit(0);
			}
	}
