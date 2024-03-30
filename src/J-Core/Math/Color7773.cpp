#include <J-Core/Math/Color7773.h>
#include <J-Core/IO/ImageUtils.h>

namespace JCore {
    Color7773::Color7773(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        uint32_t temp = 0;
        temp =
            uint32_t(remapBitsFromUI8<7>(r)) |
            (uint32_t(remapBitsFromUI8<7>(g)) << 7) |
            (uint32_t(remapBitsFromUI8<7>(b)) << 14) |
            (uint32_t(remapBitsFromUI8<3>(a)) << 21);
        memcpy(_data, &temp, 3);
    }

    uint8_t Color7773::getR() const {
        return uint8_t(_data[0] & 0x7F);
    }

    uint8_t Color7773::getG() const {
        return uint8_t((_data[0] >> 7) | ((_data[1] & 0x3F) << 1));
    }

    uint8_t Color7773::getB() const {
        return uint8_t((_data[1] >> 6) | ((_data[2] & 0x1F) << 2));
    }

    uint8_t Color7773::getA() const {
        return _data[2] >> 5;
    }
}