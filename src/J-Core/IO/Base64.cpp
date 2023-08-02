#include <J-Core/IO/Base64.h>
#include <iostream>

namespace JCore {

    static const char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string base64Encode(const uint8_t* buf, uint32_t bufLen) {
        std::string out;

        int val = 0, valb = -6;
        for (size_t i = 0; i < bufLen; i++) {
            char c = buf[i];

            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(base64Chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }

    void base64Decode(const char* str, std::vector<uint8_t>& ret) {
        int strLen = strlen(str);

        int tBuf[256]{-1};
        std::fill_n(tBuf, 256, -1);

        for (int i = 0; i < 64; i++) tBuf[base64Chars[i]] = i;

        int val = 0, valb = -8;
        for (size_t i = 0; i < strLen; i++) {
            char c = str[i];
            if (tBuf[c] == -1) break;
            val = (val << 6) + tBuf[c];
            valb += 6;
            if (valb >= 0) {
                ret.push_back(uint8_t((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
    }
}