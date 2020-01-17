/*
	CHANNEL_TREC.CPP
	----------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <sstream>

#include "file.h"
#include "parser.h"
#include "channel_trec.h"
#include "channel_buffer.h"

namespace JASS
	{
	/*
		CHANNEL_TREC::CONSTRUCT()
		-------------------------
	*/
	void channel_trec::construct(std::string &query, size_t number, const std::string &&text)
		{
		query = std::to_string(number) + " " + text;
		}

	/*
		CHANNEL_TREC::GETS()
		--------------------
	*/
	void channel_trec::gets(std::string &into)
		{
		int64_t old_number;
		std::ostringstream raw_query;
		parser parser;
		std::ostringstream answer;

		if (at_eof)
			{
			into.clear();
			return;
			}

		bool match = false;
		for (;;)
			{
			if (read)
				{
				in_channel->gets(buffer);
				if (buffer.size() == 0)
					break;		// at end of input
				}
			parser.set_document(buffer);

			bool finished = false;
			const char *token_start;
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
						if (token.get().size() == 3 && strncmp(token_start, "num", 3) == 0)
							{
							/*
								Pre-ClueWeb
							*/
							parser::token digits;
							do
								digits = parser.get_next_token();
							while (digits.type != JASS::parser::token::numeric && digits.type != JASS::parser::token::eof);
							old_number = number;
							number = atol(reinterpret_cast<char *>(digits.get().address()));

							if (old_number >= 0)
								return construct(into, old_number, answer.str());
							}
						else if (token.get().size() == 5 && strncmp(token_start, "topic", 5) == 0)
							{
							/*
								ClueWeb:  The format is <topic numner="251" type="single">.  We've got a <topic> element so we can now look for the number in what we read (buffer).
							*/
							old_number = number;
							number = 0;
							for (const char *character = buffer.c_str(); *character != '\0'; character++)
								if (isdigit(*character))
									{
									number = atol(character);
									break;
									}

							if (old_number >= 0)
								return construct(into, old_number, answer.str());
							}
						else if (token.get().size() == 5 && (strncmp(token_start, "query", 5) == 0) && (tag.find('q') != std::string::npos))
								match = true;			// ClueWeb
						else if (token.get().size() == 5 && (strncmp(token_start, "title", 5) == 0) && (tag.find('t') != std::string::npos))
								match = true;
						else if (token.get().size() >= 4 && (strncmp(token_start, "desc",  4) == 0) && (tag.find('d') != std::string::npos))
								match = true;
						else if (token.get().size() == 4 && (strncmp(token_start, "narr",  4) == 0) && (tag.find('n') != std::string::npos))
								match = true;
						break;
					default:
						break;
					}
				}
			while (!finished);
			}

		at_eof = true;
		return construct(into, number, answer.str());
		}

	/*
		CHANNEL_TREC::UNITTEST()
		------------------------
	*/
	void channel_trec::unittest(void)
		{
		/*
			Use genuine TREC Robust04 topics as a test case.
		*/
		static const char *example_file =
			"\n\
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

		std::string correct_answer = "698 literacy rates africa 699 term limits 700 gasoline tax u s ";

		std::shared_ptr<char> data(new char [strlen(example_file) + 1], [](char *pointer) {delete [] pointer;});
		strcpy(data.get(), example_file);

		std::unique_ptr<channel>infile(new channel_buffer(data));
		channel_trec query_reader(infile, "t");

		/*
			Extract the queries one at a time.
		*/
		std::string into;
		std::ostringstream answer;
		do
			{
			query_reader.gets(into);
			answer << into;
			}
		while (into.size() != 0);

		/*
			Compare to the known correct answer.
		*/

		JASS_assert(answer.str() == correct_answer);

		/*
			We passed!
		*/
		::puts("channel_trec::PASSED");
		}
	}
