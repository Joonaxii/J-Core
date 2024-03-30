#pragma once
#include <cstdint>

namespace JCore {
#pragma pack(push, 1)
    struct Color5658 {
    public:

        constexpr Color5658() : _rgb{ 0 }, _a{0} {}
        constexpr Color5658(uint16_t rgb, uint8_t a) : _rgb{ rgb }, _a{a} {}

        Color5658(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

        uint8_t getR() const;
        uint8_t getG() const;
        uint8_t getB() const;
        uint8_t getA() const;

    private:
        uint16_t _rgb;
        uint8_t _a; 
    };
#pragma pack(pop)
}