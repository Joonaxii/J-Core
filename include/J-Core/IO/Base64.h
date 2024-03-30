#pragma once
#include <vector>
#include <string>

namespace JCore {
    namespace {
        static constexpr const char BASE_64_CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    }

    template<typename T>
    void base64Encode(const uint8_t* buf, uint32_t bufLen, T& output) {
        int32_t val = 0, valb = -6;
        for (size_t i = 0; i < bufLen; i++) {
            char c = buf[i];

            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                output.push_back(BASE_64_CHARS[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) {
            out.push_back(BASE_64_CHARS[((val << 8) >> (valb + 8)) & 0x3F]);
        }
        while (out.size() & 0x3) {
            output.push_back('=');
        }
        return out;
    }

    void base64Decode(std::string_view str, std::vector<uint8_t>& ret) {
        int32_t tBuf[256]{ -1 };
        std::fill_n(tBuf, 256, -1);

        for (int i = 0; i < 64; i++) tBuf[BASE_64_CHARS[i]] = i;

        int32_t val = 0, valb = -8;
        for (size_t i = 0; i < str.size(); i++) {
            char c = str[i];
            if (tBuf[c] == -1) {
                break;
            }
            val = (val << 6) + tBuf[c];
            valb += 6;
            if (valb >= 0) {
                ret.push_back(uint8_t((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
    }
}