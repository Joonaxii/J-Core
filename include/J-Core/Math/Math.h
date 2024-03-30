#pragma once
#include <cstdint>
#include <limits>
#include <bitset>

static constexpr int32_t PAL_SIZE = 256;

constexpr float PI = 3.14159265359f;
constexpr float DEG_2_RAD = PI / 180.0f;
constexpr float RAD_2_DEG = 180.0f / PI;
constexpr float UINT8_TO_FLOAT = 1.0f / float(UINT8_MAX);
constexpr float UINT16_TO_FLOAT = 1.0f / float(UINT16_MAX);
constexpr float UINT32_TO_FLOAT = 1.0f / float(UINT32_MAX);
constexpr float UINT64_TO_FLOAT = 1.0f / float(UINT64_MAX);

namespace detail
{
    double constexpr sqrtNewtonRaphson(double x, double curr, double prev) {
        return curr == prev ? curr : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
    }

    float constexpr sqrtfNewtonRaphson(float x, float curr, float prev) {
        return curr == prev ? curr : sqrtfNewtonRaphson(x, 0.5f * (curr + x / curr), curr);
    }
}

namespace JCore::Math {
    constexpr uint8_t multUI8(uint32_t a, uint32_t b) {
        return uint8_t((a * b * 0x10101U + 0x800000U) >> 24);
    }

    constexpr uint8_t divUI8(uint32_t a, uint32_t b) {
        return uint8_t(b < 1 ? 0xFF : (a / float(b)) * 255.0f);
    }

    /// <summary>
    /// Assumes input is positive
    /// </summary>
    constexpr float roundf_P(float f) {
        return f - float(int64_t(f)) >= 0.5f ? int64_t(f) + 1.0f : float(int64_t(f));
    }

    namespace detail {
        static constexpr int32_t LOG2_TABLE_64[64] =
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

        struct Tables {
            uint8_t UI8_MUL[256 * 256];
            uint8_t UI8_DIV[256 * 256];

            Tables() : UI8_MUL(), UI8_DIV() {
                for (uint32_t i = 0, k = 0; i < 256; i++) {
                    for (uint32_t j = 0; j < 256; j++, k++)
                    {
                        UI8_MUL[k] = JCore::Math::multUI8(i, j);
                        UI8_DIV[k] = JCore::Math::divUI8(i, j);
                    }
                }
            }
        };

        struct TablesEXPR {
            uint8_t FROM_UI8[256 * 7];
            uint8_t TO_UI8[256];
            constexpr TablesEXPR() : FROM_UI8(), TO_UI8()
            {
                for (uint32_t i = 1, j = 0; i < 8; i++)
                {
                    uint32_t BIT_MASK = (1 << i) - 1;
                    for (uint32_t k = 0; k <= BIT_MASK; k++)
                    {
                        TO_UI8[j++] = uint8_t((k / float(BIT_MASK)) * 255.0f);
                    }
                }

                for (uint32_t i = 0, k = 0; i < 256; i++) {
                    float iF = i / 255.0f;
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 1) - 1)));
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 2) - 1)));
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 3) - 1)));
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 4) - 1)));
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 5) - 1)));
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 6) - 1)));
                    FROM_UI8[i] = uint8_t(roundf_P(iF * float((1 << 7) - 1)));
                }
            }
        };
        static constexpr TablesEXPR TABLES_EXPR = TablesEXPR();

        inline Tables& getTables() {
            static Tables tables{};
            return tables;
        }
    }

    template<uint8_t bits>
    inline constexpr uint8_t remapBitsToUI8(uint8_t value) {
        return bits >= 8 ? value : detail::TABLES_EXPR.TO_UI8[(value & ((1 << bits) - 1)) + ((1 << bits) - 1)];
    }

    template<uint8_t bits>
    inline constexpr uint8_t remapBitsFromUI8(uint8_t value) {
        return bits >= 8 ? value : detail::TABLES_EXPR.FROM_UI8[value + ((bits - 1) * 256)];
    }

    template<typename T, uint8_t bits>
    inline constexpr T remapBitsToUI8(uint8_t value) {
        return T(bits >= 8 ? value : detail::TABLES_EXPR.TO_UI8[(value & ((1 << bits) - 1)) + ((1 << bits) - 1)]);
    }

    template<typename T, uint8_t bits>
    inline constexpr T remapBitsFromUI8(uint8_t value) {
        return T(bits >= 8 ? value : detail::TABLES_EXPR.FROM_UI8[value + ((bits - 1) * 256)]);
    }

    inline uint8_t multUI8_LUT(uint32_t a, uint32_t b) {
        return detail::getTables().UI8_MUL[(a & 0xFF) | ((b & 0xFF) << 8)];
    }

    inline uint8_t divUI8_LUT(uint32_t a, uint32_t b) {
        return detail::getTables().UI8_DIV[(a & 0xFF) | ((b & 0xFF) << 8)];
    }

    template<typename T, typename P = float>
    struct InvLerp {
        static constexpr P inverseLerp(const T& a, const T& b, const T& v);
    };

    template<typename T, typename P>
    inline constexpr P InvLerp<T, P>::inverseLerp(const T& a, const T& b, const T& v) {
        return a == b ? P() : P((v - a) / (b - a));
    }

#undef max
#undef min
    template<typename T, typename P = double>
    inline constexpr P normalize(T value) {
        return value < 0 ? (value / P(-std::numeric_limits<T>::min())) : (value / P(std::numeric_limits<T>::max()));
    }

    template<typename T>
    inline float normalize_T(T value) {
        return 0.0f;
    }

    template<>
    inline float normalize_T(uint8_t value) {
        static float LUT[UINT8_MAX + 1]{ 1.0f };
        if (LUT[0] != 1.0f) {
            for (size_t i = 0; i <= UINT8_MAX; i++) {
                LUT[i] = float(i / float(UINT8_MAX));
            }
        }
        return LUT[value];
    }

    template<>
    inline float normalize_T(uint16_t value) {
        static float LUT[UINT16_MAX + 1]{ 1.0f };
        if (LUT[0] != 1.0f) {
            for (size_t i = 0; i <= UINT16_MAX; i++) {
                LUT[i] = float(i / float(UINT16_MAX));
            }
        }
        return LUT[value];
    }

    template<typename T>
    inline constexpr bool isPowerOf2(T value) { return (value & (value - 1)) == 0; }

    template<> inline constexpr bool isPowerOf2<double>(const double value) { return isPowerOf2<int64_t>(int64_t(value)); }
    template<> inline constexpr bool isPowerOf2<float>(const float value) { return isPowerOf2<int64_t>(int64_t(value)); }

    template<typename T>
    constexpr T sqrt(T x) {
        static_assert("Not Implemented for given type!");
        return T{};
    }

    template<>
    inline constexpr double sqrt<double>(double x) {
        return (x >= 0 && x < std::numeric_limits<double>::infinity()
            ? ::detail::sqrtNewtonRaphson(x, x, 0)
            : std::numeric_limits<double>::quiet_NaN());
    }

    template<>
    inline constexpr float sqrt<float>(float x) {
        return (x >= 0 && x < std::numeric_limits<float>::infinity()
            ? ::detail::sqrtfNewtonRaphson(x, x, 0)
            : std::numeric_limits<float>::quiet_NaN());
    }

    inline constexpr uint8_t multByte(uint8_t a, uint8_t b) {
        return uint8_t((uint32_t(a) * b * 0x10101U + 0x800000U) >> 24);
    }

    float easeInOutQuart(float t);
    float easeInOutCubic(float t);
    float easeOutCubic(float t);

    inline constexpr int32_t log2(uint64_t value) {
        if (value == 0) { return 0; }
        value |= value >> 1;
        value |= value >> 2;
        value |= value >> 4;
        value |= value >> 8;
        value |= value >> 16;
        value |= value >> 32;
        return detail::LOG2_TABLE_64[((uint64_t)((value - (value >> 1)) * 0x07EDD5E59A4E28C2)) >> 58];
    }

    inline constexpr int32_t findFirstLSB(uint64_t value) {
        return log2(value ^ (value & (value - 1)));
    }

    template<typename T>
    size_t countBits(T value) {
        return std::bitset<sizeof(T) << 3>(value).count();
    }

    template<typename T, typename P = T>
    constexpr P sign(T value, T threshold) {
        return P(value > threshold ? 1 : value < -threshold ? -1 : 0);
    }

    constexpr inline int32_t bytesRequired(int32_t bits) {
        int32_t bitsShift = bits >> 3;
        return ((bitsShift << 3) == bits) ? bitsShift : bitsShift + 1;
    }

    template<typename T, size_t size>
    inline constexpr T nextDivByPowOf2(T input) {
        static_assert(isPowerOf2(size) && "Size not a power of 2!");
        return (input + (size - 1)) & ~(size - 1);
    }

    template<typename T>
    int32_t potToIndex(T value) {
        return value != T(0) && isPowerOf2<T>(value) ? log2(uint64_t(value)) : -1;
    }

    template<> inline int32_t potToIndex<double>(double value) { return potToIndex<int64_t>(int64_t(value)); }
    template<> inline int32_t potToIndex<float>(float value) { return potToIndex<int64_t>(int64_t(value)); }

    template<typename T>
    inline constexpr T min(T lhs, T rhs) {
        return rhs < lhs ? rhs : lhs;
    }

    template<typename T>
    inline constexpr T max(T lhs, T rhs) {
        return rhs > lhs ? rhs : lhs;
    }

    template<typename T>
    inline constexpr T sign(T val) {
        return T(val > 0 ? 1 : val < 0 ? -1 : 0);
    }

    template<>
    inline constexpr int8_t sign(int8_t val) {
        return val & 0x7F;
    }

    template<>
    inline constexpr int16_t sign(int16_t val) {
        return val & 0x7FFF;
    }

    template<>
    inline constexpr int32_t sign(int32_t val) {
        return val & 0x7FFFFFFF;
    }

    template<>
    inline constexpr int64_t sign(int64_t val) {
        return val & 0x7FFFFFFFFFFFFFFFLL;
    }

    template<>
    inline constexpr uint8_t sign(uint8_t val) {
        return val;
    }

    template<>
    inline constexpr uint16_t sign(uint16_t val) {
        return val;
    }

    template<>
    inline constexpr uint32_t sign(uint32_t val) {
        return val;
    }

    template<>
    constexpr uint64_t sign(uint64_t val) {
        return val;
    }

    template<typename T>
    constexpr T clamp01(T value) {
        return T(value < 0 ? 0 : value > 1 ? 1 : value);
    }

    template<typename T>
    constexpr T clamp(T value, T min, T max) {
        return T(value < min ? min : value > max ? max : value);
    }

    template<typename T>
    constexpr T abs(T value) {
        return T(value < 0 ? -value : value);
    }

    template<typename T>
    T repeat(T t, T length) {
        static_assert("Not Implemented!");
    }

    template<>
    inline double repeat(double t, double length) {
        return clamp<double>(t - floor(t / length) * length, 0.0, length);
    }

    template<>
    inline float repeat(float t, float length) {
        return clamp<float>(t - floorf(t / length) * length, 0.0f, length);
    }

    template<typename T, typename P = float>
    constexpr T lerp(const T& a, const T& b, P t) {
        return T((P(1) - t) * a + b * t);
    }
    template<typename T, typename P = float>
    constexpr P inverseLerp(const T& a, const T& b, const T& v) {
        return InvLerp<T, P>::inverseLerp(a, b, v);
    }

    template<typename T, typename P = float>
    constexpr T remap(const T& a, const T& b, const T& aT, const T& bT, P v) {
        return lerp<T, P>(aT, bT, InvLerp<T, P>::inverseLerp(a, b, v));
    }

    template<typename T>
    constexpr inline T nextDivBy(T input, T div) {
        const int32_t divB = input % div;
        return divB ? input + (div - divB) : input;
    }

    template<typename T, typename P = float>
    constexpr T deNormalize(P value) {
        return T(value < 0 ? (value * -std::numeric_limits<T>::min()) : (value * std::numeric_limits<T>::max()));
    }

    template<typename T, typename P = double>
    inline constexpr T scalarToUInt(P input) {
        return T(clamp01<P>(input) * std::numeric_limits<T>::max());
    }

    template<typename T, typename P = double>
    inline constexpr P uintToScalar(T input) {
        return P(input) * (P(1.0) / P(std::numeric_limits<T>::max()));
    }

    static constexpr int32_t alignToPalette(int32_t size) {
        return size <= PAL_SIZE ? size : size >= PAL_SIZE * PAL_SIZE ? PAL_SIZE * PAL_SIZE : (size + PAL_SIZE - 1) & -PAL_SIZE;
    }

    static constexpr bool isAlignedToPalette(int32_t size) {
        return size <= PAL_SIZE || (size & (PAL_SIZE - 1)) == 0;
    }

    template<typename T>
    constexpr bool isInRangeExc(T input, T min, T max) {
        return input >= min && input < max;
    }

    template<typename T>
    constexpr bool isInRangeInc(T input, T min, T max) {
        return input >= min && input <= max;
    }
}
