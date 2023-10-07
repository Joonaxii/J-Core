#pragma once
#include <cstdint>
#include <functional>

namespace JCore {
    struct Color24;
    struct Color555;
    struct Color565;
    struct Color4444;
    struct Color32 {
        static const Color32 White;
        static const Color32 Black;
        static const Color32 Gray;
        static const Color32 Clear;
        static const Color32 Red;
        static const Color32 Green;
        static const Color32 Blue;
        static const Color32 Magenta;
        static const Color32 Yellow;
        static const Color32 Cyan;

        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;

        inline constexpr Color32() :r(), g(),b(),a() {}
        inline constexpr Color32(const uint32_t rgba) : r(rgba & 0xFF), b((rgba >> 8) & 0xFF), g((rgba >> 16) & 0xFF), a((rgba >> 24) & 0xFF) { }
        inline constexpr Color32(const uint8_t gray) : r(gray), g(gray), b(gray), a(0xFF) { }
        inline constexpr Color32(const uint8_t r, const uint8_t g, const uint8_t b) : r(r), g(g), b(b), a(0xFF) { }
        inline constexpr Color32(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a) : r(r), g(g), b(b), a(a) { }

        Color32(const Color24& rgb);
        Color32(const Color24& rgb, const uint8_t alpha);

        Color32(const Color555& rgb);
        Color32(const Color555& rgb, const uint8_t alpha);
        Color32(const Color565& rgb);
        Color32(const Color565& rgb, const uint8_t alpha);
        Color32(const Color4444& rgba);

        operator uint32_t() const { return *reinterpret_cast<const uint32_t*>(this); }
        operator Color24() const;

        bool operator==(const Color32& other) const { 
            return *reinterpret_cast<const uint32_t*>(this) == *reinterpret_cast<const uint32_t*>(&other); 
        };
        bool operator!=(const Color32& other) const { 
            return *reinterpret_cast<const uint32_t*>(this) != *reinterpret_cast<const uint32_t*>(&other); 
        };

        bool operator<(const Color32& other) const { 
            return static_cast<const uint32_t>(*this) < static_cast<const uint32_t>(other); 
        };
        bool operator>(const Color32& other) const {
            return static_cast<const uint32_t>(*this) > static_cast<const uint32_t>(other);
        };

        bool operator==(const Color24& other) const;
        bool operator!=(const Color24& other) const;

        bool operator<(const Color24& other) const;
        bool operator>(const Color24& other) const;

        void flipRB();
    };
}

template<>
struct std::hash<JCore::Color32> {
    size_t operator()(const JCore::Color32& color) const {
        return reinterpret_cast<const uint32_t&>(color);
    }
};