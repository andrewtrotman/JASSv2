/*
	CHANNEL_SOCKET.H
	----------------
*/
#ifndef CHANNEL_SOCKET_H_
#define CHANNEL_SOCKET_H_

#include "channel.h"
#include "sockets.h"

/*
	class ANT_CHANNEL_SOCKET
	------------------------
*/
class ANT_channel_socket : public ANT_channel
{
private:
	long connected;
	ANT_socket *socket;
	char *address;
	unsigned short port;

private:
	void connect(void);
	void reset_socket(void);

protected:
	virtual long long block_write(char *source, long long length);
	virtual char *block_read(char *into, long long length);
	virtual char *getsz(char terminator = '\0');

public:
	ANT_channel_socket(unsigned short port, char *address = NULL);
	virtual ~ANT_channel_socket();
} ;

#endif /* CHANNEL_SOCKET_H_ */

