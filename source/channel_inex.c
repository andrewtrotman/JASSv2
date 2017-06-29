/*
	CHANNEL_INEX.C
	--------------
	This channel reads from another channel assuming that that channel
	is an INEX topic file.  This allows the search engine to directly
	read INEX topic files without pre-processing.
	
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
#include "channel_inex.h"

using namespace std;

/*
	ANT_CHANNEL_INEX::ANT_CHANNEL_INEX()
	------------------------------------
	taglist is a combination of 't','d','n' (title, desc, narr) as a '\0' terminated string.  Its a set
*/
ANT_channel_inex::ANT_channel_inex(ANT_channel *in, char *taglist)
{
tags = strnew(taglist);
in_channel = in;

at_eof = false;
}

/*
	ANT_CHANNEL_INEX::~ANT_CHANNEL_INEX()
	------------------------------------
*/
ANT_channel_inex::~ANT_channel_inex()
{
delete [] tags;
delete in_channel;
}

/*
	ANT_CHANNEL_INEX::BLOCK_READ()
	------------------------------
*/
char *ANT_channel_inex::block_read(char *into, long long length)
{
exit(printf("ANT_channel_inex::block_read not implemented (class only supports gets())"));
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}

/*
	ANT_CHANNEL_INEX::BLOCK_WRITE()
	-------------------------------
*/
long long ANT_channel_inex::block_write(char *source, long long length)
{
exit(printf("ANT_channel_inex::block_write not implemented (class only supports gets())"));
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}

/*
	ANT_CHANNEL_INEX::CLEAN_SPACES_INLINE()
	---------------------------------------
*/
char *ANT_channel_inex::clean_spaces_inline(char *title)
{
char *from, *to;

from = to = title;
while (*from != '\0')
	{
	if (*from == ' ')
		if (*(from + 1) == ' ')
			from++;
		else
			*to++ = *from++;
	else
		*to++ = *from++;
	}
*to = '\0';

return title;
}

/*
	ANT_CHANNEL_INEX::CAS_CLEAN()
	-----------------------------
*/
char *ANT_channel_inex::cas_clean(char *title)
{
char *pos;
long in_square = 0;
long in_round = 0;
long in_about = 0;
long comma = 0;

for (pos = title; *pos != '\0'; pos++)
	{
	if (*pos == '[')
		in_square++;
	else if (*pos == ']')
		in_square--;
	else if (*pos == '(')
		in_round++;
	else if (*pos == ')')
		{
		in_round--;
		if (comma != 0 && in_about != 0)
			in_about = 0;
		}
	else if (*pos == ',')
		comma++;

	if (in_square != 0 && in_about == 0)
		{
		if (strncmp(pos, "about", 5) == 0)
			{
			memset(pos, ' ', 5);
			pos += 5;
			in_about++;
			comma = 0;
			}
		else
			*pos = ' ';
		}

	if (in_square != 0 && in_about != 0 && comma == 0)
		{										// support elements
		if (ANT_isalnum(*pos) || *pos == '_')
			*pos = ANT_toupper(*pos);
		else
			*pos = ' ';
		}
	else if (in_square == 0)					// target elements
		{
		if (ANT_isalnum(*pos) || *pos == '_')
			*pos = ANT_toupper(*pos);
		else
			*pos = ' ';
		}
	else										// search terms
		{
		if (ANT_isupper(*pos))
			*pos = ANT_tolower(*pos);
		if (!ANT_isalnum(*pos))
			*pos = ' ';
		}
	}

return clean_spaces_inline(title); 
}

/*
	ANT_CHANNEL_INEX::CO_CLEAN()
	----------------------------
*/
char *ANT_channel_inex::co_clean(char *title)
{
char *pos;

for (pos = title; *pos != '\0'; pos++)
	{
	if (ANT_isupper(*pos))
		*pos = ANT_tolower(*pos);
	if (!ANT_isalnum(*pos))
		*pos = ' ';
	}

return clean_spaces_inline(title); 
}


/*
	ANT_CHANNEL_INEX::GET_TAG_CONTENTS()
	------------------------------------
*/
void ANT_channel_inex::get_tag_contents(ostringstream *query, char *buffer, char *start_tag, char *end_tag)
{
char *title_pos, *first_char, *more = NULL;

title_pos = strstr(buffer, start_tag);
first_char = title_pos + strlen(start_tag);

if ((title_pos = strstr(buffer, end_tag)) != NULL)
	query->write(first_char, title_pos - first_char);
else
	{
	*query << first_char;

	while ((more = in_channel->gets()) != NULL)
		{
		strip_end_punc(more);
		if ((title_pos = strstr(more, end_tag)) == NULL)
			*query << more << ' ';
		else
			{
			query->write(more, title_pos - more);
			*query << ' ';
			break;
			}
		delete  [] more;
		}
	}

delete [] more;
}

/*
	ANT_CHANNEL_INEX::GETSZ()
	-------------------------
*/
char *ANT_channel_inex::getsz(char terminator)
{
ostringstream query;
char *buffer, *title;
long long topic;

if (at_eof)
	return NULL;

while ((buffer = in_channel->gets()) != NULL)
	{
	strip_end_punc(buffer);
	if (strstr(buffer, "<topic") != NULL)
		{
		/*
			The topic id format for 2009 efficiency track is id="2009-Eff-228", otherwise we expect "<topic id="544" ct_no="6">"
		*/
		if (strstr(buffer, "-Eff-") == NULL)
			topic = atol(strchr(strstr(buffer, "id="), '"') + 1);
		else
			topic = atol(strstr(buffer, "id=\"2009-Eff-") + 13);

		query << topic << ' ';
		}
	if (strchr(tags, 't') != NULL && strstr(buffer, "<title>") != NULL)
		get_tag_contents(&query, buffer, "<title>", "</title>");
	if (strchr(tags, 'c') != NULL && strstr(buffer, "<castitle>") != NULL)
		get_tag_contents(&query, buffer, "<castitle>", "</castitle>");
	if (strchr(tags, 'd') != NULL && strstr(buffer, "<description>") != NULL)
		get_tag_contents(&query, buffer, "<description>", "</description>");
	if (strchr(tags, 'n') != NULL && strstr(buffer, "<narrative>") != NULL)
		get_tag_contents(&query, buffer, "<narrative>", "</narrative>");

	if (strstr(buffer, "</topic>") != NULL)
		{
		title = strnew(query.str().c_str());
		if (strchr(tags, 'c') != NULL)
			cas_clean(title);
		else
			co_clean(title);

		delete [] buffer;

		return title;
		}
	delete [] buffer;
	}

at_eof = true;

return NULL;
#pragma ANT_PRAGMA_UNUSED_PARAMETER
}

