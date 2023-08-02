#pragma once
#include <cstdint>
#include <type_traits>

namespace JCore {
    struct Color32;
    struct Color555;
    struct Color565;
    struct Color24 {
        static const Color24 White;
        static const Color24 Black;
        static const Color24 Gray;
        static const Color24 Red;
        static const Color24 Green;
        static const Color24 Blue;
        static const Color24 Magenta;
        static const Color24 Yellow;
        static const Color24 Cyan;

        uint8_t r;
        uint8_t g;
        uint8_t b;

        Color24();
        Color24(const uint8_t gray);
        Color24(const uint8_t r, const uint8_t g, const uint8_t b);
        Color24(const Color32& rgba);
        Color24(const Color555& rgb);
        Color24(const Color565& rgb);

        operator uint32_t() const { return uint32_t(r) | (g << 8) | (b << 16); }
        operator Color24() const;

        bool operator==(const Color24& other) const { return r == other.r && g == other.g && b == other.b; };
        bool operator!=(const Color24& other) const { return !(*this == other); };

        bool operator<(const Color24& other) const  {
            return static_cast<const uint32_t>(*this) < static_cast<const uint32_t>(other);
        }
        bool operator>(const Color24& other) const {
            return static_cast<const uint32_t>(*this) > static_cast<const uint32_t>(other);
        }

        bool operator==(const Color32& other) const;
        bool operator!=(const Color32& other) const;

        bool operator<(const Color32& other) const;
        bool operator>(const Color32& other) const;

        void flipRB();
    };
}

template<>
struct std::hash<JCore::Color24> {
    size_t operator()(const JCore::Color24& color) const {
        return uint32_t(color);
    }
};