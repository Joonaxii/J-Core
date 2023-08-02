#pragma once
#include <cstdint>
#include <bitset>

static inline constexpr float PI = 3.14159265359f;
static inline constexpr float DEG_2_RAD = PI / 180.0f;
static inline constexpr float RAD_2_DEG = 180.0f / PI;

namespace Math {
    static constexpr int32_t PAL_SIZE = 256;
    int32_t log2(uint64_t value);
    int32_t findFirstLSB(const uint64_t value);

    float easeInOutQuart(float t);
    float easeInOutCubic(float t);
    float easeOutCubic(float t);

    template<typename T>
    size_t countBits(T value) {
        return std::bitset<sizeof(T) << 3>(value).count();
    }

    template<typename T, typename P = float>
    T lerp(T a, T b, P t) {
        return T((P(1) - t) * a + b * t);
    }

    template<typename T, typename P = float>
    P inverseLerp(T a, T b, P v) {
        if (a == b) { return P(); }
        return (P(v) - P(a)) / (P(b) - P(a));
    }

    template<typename T, typename P = float>
    P remap(T a, T b, T aT, T bT, P v) {
        return lerp<T, P>(aT, bT, inverseLerp<T, P>(a, b, v));
    }

    template<typename T>
    T clamp(const T a, const T min, const T max) {
        return a < min ? min : a > max ? max : a;
    }

    template<typename T>
    T sign(const T& value) {
        return T(value > 0 ? 1 : value < 0 ? -1 : 0);
    }

    template<typename T>
    constexpr inline T nextDivBy(T input, T div) {
        const int32_t divB = input % div;
        return divB ? input + (div - divB) : input;
    }

    template<typename T, size_t size>
    constexpr inline T nextDivByPowOf2(T input) {
        return (input + (size - 1)) & ~(size - 1);
    }

    template<typename T>
    bool isPowerOf2(const T value) { return (value & (value - 1)) == 0; }

    template<> inline bool isPowerOf2<double>(const double value) { return isPowerOf2<int64_t>(int64_t(value)); }
    template<> inline bool isPowerOf2<float>(const float value) { return isPowerOf2<int64_t>(int64_t(value)); }

    template<typename T>
    int32_t potToIndex(const T value) {
        return value != T(0) && isPowerOf2<T>(value) ? log2(uint64_t(value)) : -1;
    }

    template<> inline int32_t potToIndex<double>(const double value) { return potToIndex<int64_t>(int64_t(value)); }
    template<> inline int32_t potToIndex<float>(const float value) { return potToIndex<int64_t>(int64_t(value)); }

    static constexpr int32_t alignToPalette(const int32_t size) {
        return size <= PAL_SIZE ? size : size >= PAL_SIZE * PAL_SIZE ? PAL_SIZE * PAL_SIZE : (size + PAL_SIZE - 1) & -PAL_SIZE;
    }

    static constexpr bool isAlignedToPalette(const int32_t size) {
        return size <= PAL_SIZE || (size & (PAL_SIZE - 1)) == 0;
    }

    template<typename T>
    bool isInRangeExc(const T input, const T min, const T max) {
        return input >= min && input < max;
    }

    template<typename T>
    bool isInRangeInc(const T input, const T min, const T max) {
        return input >= min && input <= max;
    }
}