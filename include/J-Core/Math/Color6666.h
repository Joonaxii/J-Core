#pragma once
#include <cstdint>

namespace JCore {
#pragma pack(push, 1)
    struct Color6666 {
    public:

        constexpr Color6666() :_data{0} {}

        Color6666(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        uint8_t getR() const;
        uint8_t getG() const;
        uint8_t getB() const;
        uint8_t getA() const;

    private:
        uint8_t _data[3];
    };
#pragma pack(pop)
}