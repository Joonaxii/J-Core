#include <J-Core/Math/Color6666.h>
#include <J-Core/IO/ImageUtils.h>

namespace JCore {
    Color6666::Color6666(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        uint32_t temp = 0;
        temp =
            uint32_t(remapBitsFromUI8<6>(r)) |
            (uint32_t(remapBitsFromUI8<6>(g)) << 6) |
            (uint32_t(remapBitsFromUI8<6>(b)) << 12) |
            (uint32_t(remapBitsFromUI8<6>(a)) << 18);
        memcpy(_data, &temp, 3);
    }

    uint8_t Color6666::getR() const {
        return uint8_t(_data[0] & 0x3F);
    }

    uint8_t Color6666::getG() const {
        return uint8_t((_data[0] >> 6) | ((_data[1] & 0xF) << 2));
    }

    uint8_t Color6666::getB() const {
        return uint8_t((_data[1] >> 4) | ((_data[2] & 0x3) << 4));
    }

    uint8_t Color6666::getA() const {
        return _data[2] >> 2;
    }
}