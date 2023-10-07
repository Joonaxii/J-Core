#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Color24.h>
#include <J-Core/Math/Color555.h>
#include <J-Core/Math/Color565.h>
#include <J-Core/Math/Color4444.h>
#include <J-Core/IO/ImageUtils.h>

namespace JCore {
    const Color32 Color32::White(0xFF, 0xFF, 0xFF, 0xFF);
    const Color32 Color32::Black(0, 0, 0, 0xFF);
    const Color32 Color32::Gray(0x80, 0x80, 0x80, 0xFF);
    const Color32 Color32::Clear(0, 0, 0, 0);

    const Color32 Color32::Red(0xFF, 0, 0, 0xFF);
    const Color32 Color32::Green(0, 0xFF, 0, 0xFF);
    const Color32 Color32::Blue(0, 0, 0xFF, 0xFF);
    const Color32 Color32::Magenta(0xFF, 0, 0xFF, 0xFF);
    const Color32 Color32::Yellow(0xFF, 0xFF, 0, 0xFF);
    const Color32 Color32::Cyan(0, 0xFF, 0xFF, 0xFF);

    Color32::Color32(const Color24& rgb) : r(rgb.r), g(rgb.g), b(rgb.b), a(0xFF) { }
    Color32::Color32(const Color24& rgb, const uint8_t alpha) : r(rgb.r), g(rgb.g), b(rgb.b), a(alpha) { }

    Color32::Color32(const Color555& rgb) {
        unpackRGB555(rgb.data, r, g, b, a);
    }

    Color32::Color32(const Color555& rgb, const uint8_t alpha) {
        unpackRGB555(rgb.data, r, g, b, a);
        a = alpha;
    }

    Color32::Color32(const Color565& rgb) : Color32(rgb, 0xFF) { }
    Color32::Color32(const Color565& rgb, const uint8_t alpha) {
        unpackRGB565(rgb.data, r, g, b);
        a = alpha;
    }

    Color32::Color32(const Color4444& rgba) : Color32() {
        unpackRGB4444(rgba.data, r, g, b, a);
    }

    Color32::operator Color24() const { return Color24(r, g, b); }

    bool Color32::operator==(const Color24& other) const {
        const uint32_t rgb = static_cast<uint32_t>(other);
        return rgb == (static_cast<uint32_t>(*this) & 0xFFFFFF);
    }
    bool Color32::operator!=(const Color24& other) const { return !(*this == other); }

    bool Color32::operator>(const Color24& other) const {
        const uint32_t rgb = static_cast<uint32_t>(other);
        return rgb > (static_cast<uint32_t>(*this) & 0xFFFFFF);
    }
    bool Color32::operator<(const Color24& other) const { 
        const uint32_t rgb = static_cast<uint32_t>(other);
        return rgb < (static_cast<uint32_t>(*this) & 0xFFFFFF);
    }

    void Color32::flipRB() {
        uint8_t temp = r;
        r = b;
        b = temp;
    }
}