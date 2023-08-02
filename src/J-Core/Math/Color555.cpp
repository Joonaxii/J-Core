#include <J-Core/Math/Color555.h>
#include <J-Core/Math/Color565.h>
#include <J-Core/Math/Color24.h>
#include <J-Core/Math/Color32.h>

namespace JCore {
    const Color555 Color555::Clear(0x00, 0x00, 0x00, 0x00);
    const Color555 Color555::White(0xFF, 0xFF, 0xFF);
    const Color555 Color555::Black(0, 0, 0);
    const Color555 Color555::Gray(0x80, 0x80, 0x80);
    const Color555 Color555::Red(0xFF, 0, 0);
    const Color555 Color555::Green(0, 0xFF, 0);
    const Color555 Color555::Blue(0, 0, 0xFF);
    const Color555 Color555::Magenta(0xFF, 0, 0xFF);
    const Color555 Color555::Yellow(0xFF, 0xFF, 0);
    const Color555 Color555::Cyan(0, 0xFF, 0xFF);


    Color555::Color555(uint8_t r, uint8_t g, uint8_t b) : Color555(r, g, b, 0xFF) { }
    Color555::Color555(uint8_t r, uint8_t g, uint8_t b, uint8_t alpha) : data() {
        data |= (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | (alpha >= 127 ? ALPHA_MASK_555 : 0);
    }

    Color555::Color555(const Color565& rgb) : data() {
        uint16_t g = (((rgb.data & GREEN_MASK_565) >> 5) * 255) / 63;
        data |= (rgb.data & RED_MASK_565) | (((g >> 3) << 5)) | ((rgb.data & BLUE_MASK_565) >> 1) | ALPHA_MASK_555;
    }

    Color555::Color555(const Color24& rgb) : Color555(rgb.r, rgb.g, rgb.b, 0xFF) {}
    Color555::Color555(const Color32& rgba) : Color555(rgba.r, rgba.g, rgba.b, rgba.a) {}

    void Color555::flipRB() {
        uint16_t r = (data & RED_MASK_555);
        uint16_t b = (data & BLUE_MASK_555) >> 10;
        data = (data & (GREEN_MASK_555 | ALPHA_MASK_555)) | (r << 10) | b;
    }
}