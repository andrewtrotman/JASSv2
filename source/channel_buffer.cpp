/*
	CHANNEL_BUFFER.CPP
	------------------
	Copyright (c) 2020 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include <memory>
#include <iostream>

#include "channel_buffer.h"

namespace JASS
	{
	/*
		CHANNEL_BUFFER::UNITTEST()
		--------------------------
	*/
	void channel_buffer::unittest(void)
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
			</top>";

		/*
			Generate a file with the data in it.
		*/
		std::shared_ptr<char> source(new char[::strlen(example_file) + 1], [](char *pointer) { delete [] pointer; });
		strcpy(source.get(), example_file);

		channel_buffer reader(source);

		/*
			Extract the queries one at a time.
		*/
		std::string into;
		std::ostringstream answer;
		do
			{
			reader.gets(into);
			answer << into;
//			std::cout << into;
			}
		while (into.size() != 0);

		/*
			Compare to the known correct answer.
		*/

		JASS_assert(answer.str() == example_file);

		/*
			We passed!
		*/
		::puts("channel_buffer::PASSED");
		}
	}
