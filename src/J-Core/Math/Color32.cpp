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

    Color32::Color32(const Color555& rgb) :
        r(remapUI8Bits<5>(rgb.data & 0x1F)),
        g(remapUI8Bits<5>((rgb.data >> 5) & 0x3F)),
        b(remapUI8Bits<5>((rgb.data >> 10) & 0x1F)),
        a(remapUI8Bits<1>((rgb.data >> 15) & 0x1))
    {  }


    Color32::Color32(const Color555& rgb, const uint8_t alpha) :
        r(remapUI8Bits<5>(rgb.data & 0x1F)),
        g(remapUI8Bits<5>((rgb.data >> 5) & 0x3F)),
        b(remapUI8Bits<5>((rgb.data >> 10) & 0x1F)),
        a(alpha)
    {  }

    Color32::Color32(const Color565& rgb) : Color32(rgb, 0xFF) { }
    Color32::Color32(const Color565& rgb, const uint8_t alpha) :
        r(remapUI8Bits<5>(rgb.data & 0x1F)),
        g(remapUI8Bits<6>((rgb.data >> 5)& 0x3F)),
        b(remapUI8Bits<5>((rgb.data >> 11) & 0x1F)),
        a(alpha)
    {  }

    Color32::Color32(const Color4444& rgba) : 
        r(remapUI8Bits<4>(rgba.getR())),
        g(remapUI8Bits<4>(rgba.getG())),
        b(remapUI8Bits<4>(rgba.getB())),
        a(remapUI8Bits<4>(rgba.getA()))
    {  }

    Color8887::Color8887(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
        r(r), g(g), b(b),
        a(remapBitsFromUI8<7>(a)) { }

    Color8887::Color8887(const Color32& rgba) : Color8887(rgba.r, rgba.g, rgba.b, rgba.a) {}

    Color32::Color32(const Color8887& rgba) :
        r(rgba.r), g(rgba.g), b(rgba.b),
        a(remapUI8Bits<7>(rgba.a)) {}


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