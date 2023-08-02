#pragma once
#include <cstdint>
#include <type_traits>

static constexpr uint16_t RED_MASK_555 = 0x1F;
static constexpr uint16_t GREEN_MASK_555 = 0x3E0;
static constexpr uint16_t BLUE_MASK_555 = 0x7C00;
static constexpr uint16_t ALPHA_MASK_555 = 0x8000;

namespace JCore {

    struct Color565;
    struct Color24;
    struct Color32;
    struct Color555 {
        static const Color555 Clear;
        static const Color555 White;
        static const Color555 Black;
        static const Color555 Gray;
        static const Color555 Red;
        static const Color555 Green;
        static const Color555 Blue;
        static const Color555 Magenta;
        static const Color555 Yellow;
        static const Color555 Cyan;

        uint16_t data{ 0 };

        Color555() : data(0) {}
        Color555(uint16_t value) : data(value) {}
        Color555(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha);
        Color555(uint8_t r, uint8_t g, uint8_t b);

        Color555(const Color565& rgb);
        Color555(const Color24& rgb);
        Color555(const Color32& rgba);

        bool operator <(const Color555& other) const { return data < other.data; }
        bool operator >(const Color555& other) const { return data > other.data; }

        void flipRB();
    };
}

template<>
struct std::hash<JCore::Color555> {
    size_t operator()(const JCore::Color555& color) const {
        return uint32_t(color.data);
    }
};