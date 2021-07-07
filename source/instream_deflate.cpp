/*
	INSTREAM_DEFLATE.CPP
	--------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
*/
#include <string.h>

#include "assert.h"
#include "instream_deflate.h"
#include "compress_general_zlib.h"

namespace JASS
	{
	/*
		INSTREAM_DEFLATE::INSTREAM_DEFLATE()
		------------------------------------
	*/
	instream_deflate::instream_deflate(std::shared_ptr<instream> &source) :
		instream(source)
		{
		stream.zalloc = Z_NULL;
		stream.zfree = Z_NULL;
		stream.opaque = Z_NULL;
		stream.avail_in = 0;
		stream.next_in = Z_NULL;

		bytes_read = 0;
		buffer = NULL;
		}

	/*
		INSTREAM_DEFLATE::READ()
		------------------------
	*/
	void instream_deflate::read(document &document)
	{
	size_t got;
	int state;

	if (buffer == NULL)
		{
		buffer = new uint8_t [buffer_length];
		if (inflateInit2(&stream, 15 + 32) != Z_OK)		// 2^15 window with zlib/gzip header detection
			{
			document.contents = slice();
			return;										// error, deflateInit2() failed!
			}
		}

	stream.avail_out = (uInt)document.contents.size();
	stream.next_out = &document.contents[0];

	do
		{
		if (stream.avail_in <= 0)
			{
			if ((got = source->fetch(buffer, buffer_length)) < 0)
				{
				document.contents = slice();
				return;										// error, deflateInit2() failed!
				}
			stream.avail_in = (uInt)got;
			stream.next_in = buffer;
			}

		state = inflate(&stream, Z_NO_FLUSH);

		if (state == Z_STREAM_END)
			{
			got = document.contents.size() - stream.avail_out;		// number of bytes that were decompressed
			document.contents.resize(got);
			bytes_read += got;
			return;										// at EOF
			}

		if (stream.avail_out == 0)
			{
			bytes_read += document.contents.size();
			return;			// filled the output buffer and so return bytes read
			}
		}
	while (state == Z_OK);

	printf("JASS::instream_deflate::read failure trying to decompress (zlib reports:%d)\n", state);
	document.contents = slice();
	}

	/*
		INSTREAM_FILE::UNITTEST()
		-------------------------
	*/
	void instream_deflate::unittest(void)
		{
		/*
			Yay, we passed!
		*/
		puts("instream_deflate::PASSED");
		}
	}

