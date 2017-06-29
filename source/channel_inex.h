/*
	CHANNEL_INEX.H
	--------------
*/
#ifndef CHANNEL_INEX_H_
#define CHANNEL_INEX_H_

#include <sstream>
#include "stop_word.h"
#include "channel.h"

/*
	class ANT_CHANNEL_INEX
	----------------------
*/
class ANT_channel_inex : public ANT_channel
{
private:
	ANT_channel *in_channel;
	long at_eof;
	char *tags;

protected:
	char *clean_spaces_inline(char *title);
	char *cas_clean(char *title);
	char *co_clean(char *title);
	void get_tag_contents(std::ostringstream *query, char *buffer, char *start_tag, char *end_tag);

	virtual long long block_write(char *source, long long length);
	virtual char *block_read(char *into, long long length);
	virtual char *getsz(char terminator = '\0');

public:
	ANT_channel_inex(ANT_channel *in, char *taglist);
	virtual ~ANT_channel_inex();
} ;



#endif /* CHANNEL_INEX_H_ */
