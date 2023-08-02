#pragma once 
#include <cstdint>
#include <type_traits>

static constexpr uint16_t RED_MASK_565 = 0x1F;
static constexpr uint16_t GREEN_MASK_565 = 0x7E0;
static constexpr uint16_t BLUE_MASK_565 = 0xF800;

namespace JCore {
    struct Color24;
    struct Color555;
    struct Color32;
    struct Color565 {
        static const Color565 White;
        static const Color565 Black;
        static const Color565 Gray;
        static const Color565 Red;
        static const Color565 Green;
        static const Color565 Blue;
        static const Color565 Magenta;
        static const Color565 Yellow;
        static const Color565 Cyan;

        uint16_t data{ 0 };

        Color565() : data(0) {}
        Color565(uint16_t value) : data(value) {}
        Color565(uint8_t r, uint8_t g, uint8_t b);

        Color565(const Color555& rgb);
        Color565(const Color24& rgb);
        Color565(const Color32& rgba);

        bool operator <(const Color565& other) const { return data < other.data; }
        bool operator >(const Color565& other) const { return data > other.data; }

        void flipRB();
    };
}

template<>
struct std::hash<JCore::Color565> {
    size_t operator()(const JCore::Color565& color) const {
        return uint32_t(color.data);
    }
};