/*
	CHANNEL_TREC.C
	--------------
	This channel reads from another channel assuming that that channel
	is a TREC topic file.  This allows the search engine to directly
	read TREC topic files without pre-processing.
	
	The out channel is what-ever is passed to the constructor of this object
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include "pragma.h"
#include "str.h"
#include "stop_word.h"
#include "ctypes.h"
#include "channel_trec.h"

using namespace std;

/*
	ANT_CHANNEL_TREC::ANT_CHANNEL_TREC()
	------------------------------------
	taglist is a combination of 't','d','n' (title, desc, narr) as a '\0' terminated string.  Its a set
*/
ANT_channel_trec::ANT_channel_trec(ANT_channel *in, char *taglist)
{
tag = strnew(taglist);
in_channel = in;

read = true;
number = -1;
at_eof = false;
}

/*
	ANT_CHANNEL_TREC::~ANT_CHANNEL_TREC()
	------------------------------------
*/
ANT_channel_trec::~ANT_channel_trec()
{
delete [] tag;
delete in_channel;
}

/*
	ANT_CHANNEL_TREC::BLOCK_READ()
	------------------------------
*/
char *ANT_channel_trec::block_read(char *into, long long length)
{
exit(printf("ANT_channel_trec::block_read not implemented (class only supports gets())"));
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}

/*
	ANT_CHANNEL_TREC::BLOCK_WRITE()
	-------------------------------
*/

long long ANT_channel_trec::block_write(char *source, long long length)
{
exit(printf("ANT_channel_trec::block_write not implemented (class only supports gets())"));
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}

/*
	ANT_CHANNEL_TREC::CLEAN()
	-------------------------
*/
char *ANT_channel_trec::clean(long number, ostringstream &raw_query)
{
char *from, *to, *destination;
ostringstream stopped_query;
string copy;

raw_query << ends;

to = destination = new char [raw_query.tellp() * 2 + 1];
copy = raw_query.str();
from = (char *)(copy.c_str());

while (*from != '\0')
	{
	if (ANT_isalpha(*from))
		{
		do
			*to++ = ANT_tolower(*from++);
		while (ANT_isalpha(*from));
		*to++ = ' ';
		}
	else if (ANT_isdigit(*from))
		{
		do
			*to++ = ANT_tolower(*from++);
		while (ANT_isdigit(*from));
		*to++ = ' ';
		}
	else
		{
		from++;
		while (!ANT_isalnum(*from) && *from != '\0')
			{
			*to++ = ' ';
			from++;
			}
		}
	}

*to = '\0';
strip_space_inplace(destination);

stopped_query << number << ' ' << destination << ends;
delete [] destination;

return strnew((char *)stopped_query.str().c_str());
}

/*
	ANT_CHANNEL_TREC::GETSZ()
	-------------------------
*/
char *ANT_channel_trec::getsz(char terminator)
{
long match, old_number;
ostringstream raw_query;
char *buffer_start, *buffer_end, *colon;

if (at_eof)
	return NULL;

for (;;)
	{
	if (read)
		if ((buffer = in_channel->gets()) == NULL)
			break;		// at end of input

	read = true;

	/*
		trim preceeding whitespace
	*/
	buffer_start = buffer;
	while (ANT_isspace(*buffer_start))
		buffer_start++;

	if (strncmp(buffer_start, "<num>", 5) == 0)
		{
		/*
			Pre-ClueWeb
		*/
		old_number = number;
		number = atol(strchr(buffer_start, ':') + 1);
		delete [] buffer;

		if (old_number >= 0)
			return clean(old_number, raw_query);
		}
	else if (strncmp(buffer_start, "<topic number=", 14) == 0)
		{
		/*
			ClueWeb
		*/
		old_number = number;
		number = atol(strchr(buffer_start, '"') + 1);
		delete [] buffer;

		if (old_number >= 0)
			return clean(old_number, raw_query);
		}
	else
		{
		match = false;

		if (*buffer_start == '<')
			{
			if ((strncmp(buffer_start + 1, "query", 5) == 0) && (strchr(tag, 'q') != NULL))
				match = true;			// ClueWeb
			if ((strncmp(buffer_start + 1, "title", 5) == 0) && (strchr(tag, 't') != NULL))
				match = true;
			if ((strncmp(buffer_start + 1, "desc",  4) == 0) && (strchr(tag, 'd') != NULL))
				match = true;
			if ((strncmp(buffer_start + 1, "narr",  4) == 0) && (strchr(tag, 'n') != NULL))
				match = true;
			}
		if (match)
			{
			if ((buffer_end = strchr(buffer_start + 1, '<')) != NULL)
				{
				/*
					the close tag and the open tag are on the same line (thank ClueWeb for this change)
				*/
				buffer_start = strchr(buffer_start, '>') + 1;
				raw_query.write(buffer_start, buffer_end - buffer_start);
				delete [] buffer;
				}
			else
				{
				if ((colon = strchr(buffer_start, ':')) != NULL)
					raw_query << colon + 1;			// character after the "<desc> Description:"
				else
					raw_query << strchr(buffer_start, '>') + 1;			// character after the "<desc>"
				delete [] buffer;
				while ((buffer = in_channel->gets()) != NULL)
					{
					if (*buffer == '<')
						{
						read = false;
						break;
						}
					strip_space_inplace(buffer);
					raw_query << ' ' << buffer;
					delete [] buffer;
					}
				}
			}
		else
			delete [] buffer;
		}
	}

at_eof = true;
return clean(number, raw_query);
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}