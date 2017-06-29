/*
	CHANNEL_TREC.H
	--------------
*/
#ifndef CHANNEL_TREC_H_
#define CHANNEL_TREC_H_

#include "stop_word.h"
#include "channel.h"
#include <sstream>

/*
	class ANT_CHANNEL_TREC
	----------------------
*/
class ANT_channel_trec : public ANT_channel
{
private:
	ANT_channel *in_channel;
	char *buffer;
	long read, at_eof;
	long number;
	char *tag;

private:
	char *clean(long number, 
std::ostringstream &raw_query);

protected:
	virtual long long block_write(char *source, long long length);
	virtual char *block_read(char *into, long long length);
	virtual char *getsz(char terminator = '\0');

public:
	ANT_channel_trec(ANT_channel *in, char *taglist);
	virtual ~ANT_channel_trec();
} ;

#endif
