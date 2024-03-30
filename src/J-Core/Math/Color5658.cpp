#include <J-Core/Math/Color5658.h>
#include <J-Core/IO/ImageUtils.h>

namespace JCore {
    Color5658::Color5658(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        _rgb =
            uint16_t(remapBitsFromUI8<5>(r)) |
            (uint16_t(remapBitsFromUI8<6>(g)) << 5) |
            (uint16_t(remapBitsFromUI8<5>(b)) << 11);
        _a = a;
    }

    uint8_t Color5658::getR() const {
        return uint8_t(_rgb & 0x1F);
    }

    uint8_t Color5658::getG() const {
        return uint8_t((_rgb >> 5) & 0x3F);
    }

    uint8_t Color5658::getB() const {
        return uint8_t((_rgb >> 11) & 0x1F);
    }

    uint8_t Color5658::getA() const {
        return _a;
    }
}