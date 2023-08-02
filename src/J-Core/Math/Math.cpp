#include <J-Core/Math/Math.h>
#include <math.h>

namespace Math {
    const int32_t LOG2_TABLE_64[64] =
    {
        63,  0, 58,  1, 59, 47, 53,  2,
        60, 39, 48, 27, 54, 33, 42,  3,
        61, 51, 37, 40, 49, 18, 28, 20,
        55, 30, 34, 11, 43, 14, 22,  4,
        62, 57, 46, 52, 38, 26, 32, 41,
        50, 36, 17, 19, 29, 10, 13, 21,
        56, 45, 25, 31, 35, 16,  9, 12,
        44, 24, 15,  8, 23,  7,  6,  5
    };

    int32_t log2(uint64_t value) {
        if (value == 0) { return 0; }
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        return LOG2_TABLE_64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
    }

    int32_t findFirstLSB(const uint64_t value) {
        const uint64_t val = value ^ (value & (value - 1));
        return log2(val);
    }

    float easeInOutQuart(float t) {
        if (t <= 0) { return 0; }
        if (t >= 1) { return 1; }

        if (t < 0.5f) {
            return powf(2.0f, 20.0f * t - 10.0f) * 0.5f;
        }
        return (2.0f - powf(2.0f, -20.0f * t + 10.0f)) * 0.5f;
    }

    float easeInOutCubic(float t) {
        if (t < 0.5f) {
            return (4 * t * t * t);
        }
        return 1.0f - powf(-2 * t + 2, 3) * 0.5f;
    }

    float easeOutCubic(float t) {
        return 1.0f - powf(1.0f - t, 3);
    }
}