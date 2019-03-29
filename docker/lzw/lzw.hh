// Lempel-Ziv-Welch codec
// ----------------------
// This program is released under the GPL license.
// -----------------------------------------------

#ifndef LEMPEL_ZIV_WELCH_HH
#define LEMPEL_ZIV_WELCH_HH

#include <vector>

class LZWCodec
{
 public:
    // The encoder will test all max bitsizes from maxBits1 to maxBits2
    // if maxBits1 < maxBits2. In order to specify only one max bitsize
    // leave maxBits2 to 0.
    LZWCodec(unsigned maxBits1=16, unsigned maxBits2=0,
             bool verbose=false, bool printTime = false);

    void encode(const std::vector<unsigned char>& input,
                std::vector<unsigned char>& encoded);

    void decode(const std::vector<unsigned char>& encoded,
                std::vector<unsigned char>& decoded);



 private:
    bool verbose, printTime;
    unsigned maxBits1, maxBits2;
};

#endif
