// This program is released under the GPL license.
// -----------------------------------------------

#ifndef STREAM_IO_HH
#define STREAM_IO_HH

#include <vector>

class StreamWriter
{
    typedef unsigned char byte;
    std::vector<byte>& stream;
    unsigned curBitIndex;

 public:
    inline StreamWriter(std::vector<byte>& s);

    inline void write(unsigned value, unsigned bits);
};

class StreamReader
{
    typedef unsigned char byte;
    const std::vector<byte>& stream;
    unsigned curBitIndex;
    size_t curByteIndex;

 public:
    inline StreamReader(const std::vector<byte>& s, size_t startIndex);

    inline unsigned read(unsigned bits);
};




inline StreamWriter::StreamWriter(std::vector<byte>& s):
    stream(s), curBitIndex(0) {}

inline void StreamWriter::write(unsigned value, unsigned bits)
{
    while(bits > 0)
    {
        if(curBitIndex == 0)
        {
            stream.push_back(byte(value));
            if(bits < 8)
            {
                curBitIndex += bits;
                bits = 0;
            }
            else
            {
                bits -= 8;
                value >>= 8;
            }
        }
        else
        {
            const byte b = byte(value << curBitIndex);
            stream.back() |= b;
            const unsigned bitsWritten = 8-curBitIndex;
            if(bits < bitsWritten)
            {
                curBitIndex += bits;
                bits = 0;
            }
            else
            {
                bits -= bitsWritten;
                value >>= bitsWritten;
                curBitIndex = 0;
            }
        }
    }
}

inline StreamReader::StreamReader(const std::vector<byte>& s,
                                  size_t startIndex):
    stream(s), curBitIndex(0), curByteIndex(startIndex) {}

inline unsigned StreamReader::read(unsigned bits)
{
    unsigned res = 0;
    unsigned resBitIndex = 0;

    while(bits > 0)
    {
        unsigned iValue = stream[curByteIndex];

        if(curBitIndex == 0)
        {
            if(bits < 8)
            {
                const unsigned mask = ~(~0U << bits);
                res |= (iValue & mask) << resBitIndex;
                curBitIndex += bits;
                bits = 0;
            }
            else
            {
                res |= iValue << resBitIndex;
                resBitIndex += 8;
                bits -= 8;
                ++curByteIndex;
            }
        }
        else
        {
            const unsigned mask = ~(~0U << bits);
            res |= ((iValue>>curBitIndex) & mask) << resBitIndex;
            const unsigned bitsLeft = 8-curBitIndex;
            if(bits < bitsLeft)
            {
                curBitIndex += bits;
                bits = 0;
            }
            else
            {
                curBitIndex = 0;
                ++curByteIndex;
                bits -= bitsLeft;
                resBitIndex += bitsLeft;
            }
        }
    }

    return res;
}

#endif
