#pragma once
#include <vector>
#include <string>

namespace JCore {
    std::string base64Encode(const uint8_t* buf, uint32_t bufLen);
    void base64Decode(const char* str, std::vector<uint8_t>& data);
}