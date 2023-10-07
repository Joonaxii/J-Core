#pragma once
#include <cstdint>

namespace JCore {
    struct Color4444 {
        uint16_t data;

        constexpr Color4444() : data(0) {};
        constexpr Color4444(uint16_t r, uint16_t g, uint16_t b, uint16_t a) : data((r & 0xF) | ((g & 0xF) << 4) | ((b & 0xF) << 8) | ((a & 0xF) << 12)) {};
        constexpr Color4444(uint16_t r, uint16_t g, uint16_t b) : Color4444(r, g, b, 0xF) {};
        constexpr Color4444(uint16_t v) : Color4444(v, v, v, 0xF) {};
        constexpr Color4444(uint16_t v, uint16_t a) : Color4444(v, v, v, a) {};

        constexpr uint8_t getR() const { return uint8_t(data & 0xF); }
        constexpr uint8_t getG() const { return uint8_t(data & 0xF0) >> 4; }
        constexpr uint8_t getB() const { return uint8_t(data & 0xF00) >> 4; }
        constexpr uint8_t getA() const { return uint8_t(data & 0xF000) >> 4; }

        void setR(uint16_t value) { data = (data & ~0xF)    | ((value & 0xF)); }
        void setG(uint16_t value) { data = (data & ~0xF0)   | ((value & 0xF) << 4); }
        void setB(uint16_t value) { data = (data & ~0xF00)  | ((value & 0xF) << 8); }
        void setA(uint16_t value) { data = (data & ~0xF000) | ((value & 0xF) << 12); }

        void flipRB() {
            data = (data & 0xF0F0) | ((data & 0xF) << 8) | ((data & 0x0F00) >> 8);
        }
    };
}