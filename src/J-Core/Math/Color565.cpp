#include <J-Core/Math/Color565.h>
#include <J-Core/Math/Color555.h>
#include <J-Core/Math/Color24.h>
#include <J-Core/Math/Color32.h>

namespace JCore {
    const Color565 Color565::White(0xFF, 0xFF, 0xFF);
    const Color565 Color565::Black(0, 0, 0);
    const Color565 Color565::Gray(0x80, 0x80, 0x80);
    const Color565 Color565::Red(0xFF, 0, 0);
    const Color565 Color565::Green(0, 0xFF, 0);
    const Color565 Color565::Blue(0, 0, 0xFF);
    const Color565 Color565::Magenta(0xFF, 0, 0xFF);
    const Color565 Color565::Yellow(0xFF, 0xFF, 0);
    const Color565 Color565::Cyan(0, 0xFF, 0xFF);

    Color565::Color565(uint8_t r, uint8_t g, uint8_t b) {
        data = ((b >> 3) << 11) | ((g >> 2) << 5) | (r >> 3);
    }
    Color565::Color565(const Color555& rgb) : data() {
        uint16_t g = (((rgb.data & GREEN_MASK_555) >> 5) * 255) / 31;
        data |= (rgb.data & RED_MASK_555) | (g << 5) | ((rgb.data & BLUE_MASK_555) << 1);
    }

    Color565::Color565(const Color24& rgb) : Color565(rgb.r, rgb.g, rgb.b) {}
    Color565::Color565(const Color32& rgba) : Color565(rgba.r, rgba.g, rgba.b) {}

    void Color565::flipRB() {
        uint16_t r = (data & RED_MASK_565);
        uint16_t b = (data & BLUE_MASK_565) >> 11;
        data = (data & GREEN_MASK_565) | (r << 11) | b;
    }
}