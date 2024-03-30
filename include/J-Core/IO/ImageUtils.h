#pragma once
#include <cstdint>
#include <functional>
#include <J-Core/Math/Color24.h>
#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Color555.h>
#include <J-Core/Math/Color565.h>
#include <J-Core/Math/Color4444.h>
#include <J-Core/Math/Math.h>
#include <unordered_map>
#include <glm.hpp>
#include <algorithm>

namespace JCore {
    template<typename T>
    using PixelConvert = std::function<void(const uint8_t*, const uint8_t*, T*)>;

    enum class TextureFormat : uint8_t {
        Unknown,

        R8,
        RGB24,
        RGB48,
        RGBA32,
        RGBA64,
        Indexed8,
        Indexed16,

        RGBA4444,

        Count,
    };

    static constexpr int32_t getBitsPerPixel(const TextureFormat format) {
        switch (format) {
            default: return 0;

            case TextureFormat::R8:        return 8;
            case TextureFormat::Indexed8:  return 8;
            case TextureFormat::Indexed16: return 16;
            case TextureFormat::RGB24:     return 24;
            case TextureFormat::RGBA32:    return 32;

            case TextureFormat::RGB48:     return 48;
            case TextureFormat::RGBA64:    return 64;
            case TextureFormat::RGBA4444:  return 16;
        }
    }
    static constexpr int32_t getBitDepth(const TextureFormat format) {
        switch (format) {
            default: return 0;

            case TextureFormat::R8:        return 8;
            case TextureFormat::Indexed8:  return 8;
            case TextureFormat::Indexed16: return 16;
            case TextureFormat::RGB24:     return 8;
            case TextureFormat::RGBA32:    return 8;

            case TextureFormat::RGB48:     return 16;
            case TextureFormat::RGBA64:    return 16;

            case TextureFormat::RGBA4444:  return 4;
        }
    }
    static constexpr const char* getTextureFormatName(const TextureFormat format) {
        switch (format) {
            default:						return "Unknown";

            case TextureFormat::R8:	        return "R8";

            case TextureFormat::Indexed8:	return "Indexed8";
            case TextureFormat::Indexed16:	return "Indexed16";

            case TextureFormat::RGB24:		return "RGB24";
            case TextureFormat::RGBA32:		return "RGBA32";

            case TextureFormat::RGB48:		return "RGB48";
            case TextureFormat::RGBA64:		return "RGBA64";

            case TextureFormat::RGBA4444:	return "RGBA4444";
        }
    }

    bool hasAlpha(const uint8_t* data, int32_t width, int32_t height, TextureFormat format, int32_t paletteSize = -1);
    int32_t findInPalette(const Color32 color, const Color32* palette, uint32_t size);

    bool tryBuildPalette(Color32 pixel, int32_t& colors, TextureFormat& format, uint8_t* newPalette, std::unordered_map<uint32_t, int32_t>& paletteLut, int32_t alphaClip = -1, int32_t absoluteMax = -1);

    bool tryBuildPalette8(Color32 pixel, int32_t& colors, TextureFormat& format, uint8_t* newPalette, Color32 buffer[32], uint32_t& bufSize, int32_t alphaClip = -1);
    bool tryBuildPalette16(Color32 pixel, int32_t& colors, TextureFormat& format, uint8_t* newPalette, Color32 buffer[256], uint32_t& bufSize, int32_t alphaClip = -1, int32_t maxSize = 256 * 256);

    bool tryBuildPalette(const uint8_t* pixelData, int32_t start, int32_t length, int32_t& colors, TextureFormat& format, int32_t bpp, uint8_t* newPalette, int32_t alphaClip = -1);
    void applyPalette(const uint8_t* pixelData, int32_t width, int32_t height, int32_t colors, TextureFormat format, uint8_t* targetPixels, TextureFormat paletteFmt, int32_t alphaClip = -1);

    bool tryBuildPalette(const uint8_t* pixelData, int32_t start, int32_t length, int32_t& colors, int32_t bpp, uint8_t* newTexture, int32_t alphaClip = -1);
    int32_t tryBuildPalette(const uint8_t* pixelData, uint32_t width, uint32_t height, TextureFormat format, uint8_t* newTexture, int32_t alphaClip = -1);
    
    uint8_t quantizeUI8(uint8_t value, int32_t bits);

    uint8_t remapUI16ToUI8(uint16_t value);

    template<uint8_t bits>
    inline uint8_t remapUI8Bits(uint8_t value) {
        if (bits >= 8 || bits < 1) { return value; }
        static constexpr uint8_t BIT_MASK = (1 << bits) - 1;

        static uint8_t TABLE[1 << bits]{ 0xFF };
        if (TABLE[0] == 0xFF) {
            for (size_t i = 0; i <= BIT_MASK; i++) {
                TABLE[i] = uint8_t((i / float(BIT_MASK)) * 255.0f);
            }
        }
        return TABLE[value & BIT_MASK];
    }

    template<uint8_t bits>
    inline uint8_t remapBitsFromUI8(uint8_t value) {
        if (bits >= 8 || bits < 1) { return value; }
        static constexpr uint8_t BIT_MASK = (1 << bits) - 1;

        static uint8_t TABLE[256]{ 0xFF };
        if (TABLE[0] == 0xFF) {
            for (size_t i = 0; i < 256; i++) {
                TABLE[i] = uint8_t(std::roundf(i / 255.0f * float(BIT_MASK)));
            }
        }
        return TABLE[value];
    }

    constexpr uint8_t multUI8(uint32_t a, uint32_t b) {
        return uint8_t((a * b * 0x10101U + 0x800000U) >> 24);
    }

    inline uint8_t divUI8(int32_t a, int32_t b) {
        static bool init{ false };
        static uint8_t LUT[256 * 256]{0};
        if (!init) {
            for (int32_t i = 0; i < 256; i++) {
                for (int32_t j = 0; j < 256; j++) {
                    LUT[i | (j << 8)] = uint8_t(j < 1 ? 0xFF : (i / float(j)) * 255.0f);
                }
            }
            init = true;
        }
        return LUT[(a & 0xFF) | ((b & 0xFF) << 8)];
    }
    
    inline uint8_t getLuminance(uint8_t r, uint8_t g, uint8_t b) {
        return uint8_t(std::clamp<float>(0.2126f * float(r) + 0.7152f * float(g) + 0.0722f * float(b), 0.0f, 255.0f));
    }

    inline bool getThresholdLUM(uint8_t r, uint8_t g, uint8_t b, int32_t min, int32_t max) {
        int32_t lum = getLuminance(r, g, b);
        return lum > min && lum < max;
    }

    inline bool getThresholdRGB(uint8_t r, uint8_t g, uint8_t b, int32_t minR, int32_t maxR, int32_t minG, int32_t maxG, int32_t minB, int32_t maxB) {
        return (r >= minR && r <= maxR) && (g >= minG && g <= maxG) && (b >= minB && b <= maxB);
    }

    template<typename T, typename P = int32_t>
    inline P channelDistance(const T& a, const T& b) {
        return (std::abs(P(a.r) - P(b.r)) + std::abs(P(a.g) - P(b.g)) + std::abs(P(a.b) - P(b.b))) / P(3);
    }

    template<typename T, typename P = int32_t>
    inline P channelDistance(const T& a, P bR, P bG, P bB) {
        return (std::abs(P(a.r) - bR) + std::abs(P(a.g) - bG) + std::abs(P(a.b) - bB)) / P(3);
    }

    template<typename T>
    inline float channelDistanceSqrt(const T& a, float bR, float bG, float bB) {
        float r = float(a.r) - bR, g = float(a.g) - bG, b = float(a.b) - bB;
        r *= r;
        g *= g;
        b *= b;
        return sqrtf((r + g + b) / 3);
    }

    static inline void colorToAlpha(Color32& pix, float r1, float r2, float r3, float mA, float mX) { 
        static constexpr float BYTE_TO_FLOAT = 1.0f / 255.0f;
        float rgba[4] { 
            float(pix.r) * BYTE_TO_FLOAT, 
            float(pix.g) * BYTE_TO_FLOAT, 
            float(pix.b) * BYTE_TO_FLOAT, 
            float(pix.a) 
        };
        float cOut[4] { 
            0, 0, 0, 0
        };
        float dist = Math::max<float>(Math::max<float>(Math::abs(rgba[0] - r1), Math::abs(rgba[1] - r2)), Math::abs(rgba[2] - r3));

        if (dist <= mA) {
            pix.a = 0;
            return;
        }

        if (dist >= mX) {
            return;
        }

        float tDiff = mX - mA;
        float alpha = Math::clamp((dist - tDiff) / tDiff, 0.0f, 1.0f);

        float oProp = (dist / mX);
        pix.r = uint8_t(((rgba[0] - r1) / oProp + r1) * 255.0f);
        pix.g = uint8_t(((rgba[1] - r2) / oProp + r2) * 255.0f);
        pix.b = uint8_t(((rgba[2] - r3) / oProp + r3) * 255.0f);
        pix.a = uint8_t(rgba[3] * alpha);
    }

    static inline void colorToAlpha(float& pA, float& p1, float& p2, float& p3, float r1, float r2, float r3, float mA = 1, float mX = 1) {
        float aA, a1, a2, a3;

        if (p1 > r1) a1 = mA * (p1 - r1) / (mX - r1);
        else if (p1 < r1) a1 = mA * (r1 - p1) / r1;
        else a1 = 0.0f;

        if (p2 > r2) a2 = mA * (p2 - r2) / (mX - r2);
        else if (p2 < r2) a2 = mA * (r2 - p2) / r2;
        else a2 = 0.0f;

        if (p3 > r3) a3 = mA * (p3 - r3) / (mX - r3);
        else if (p3 < r3) a3 = mA * (r3 - p3) / r3;
        else a3 = 0.0f;
 
        aA = a1;
        if (a2 > aA) aA = a2;
        if (a3 > aA) aA = a3;

        if (aA >= mA / mX) {
            pA = aA * pA / mA;
            p1 = mA * (p1 - r1) / aA + r1;
            p2 = mA * (p2 - r2) / aA + r2;
            p3 = mA * (p3 - r3) / aA + r3;
        }
        else {
            pA = 0;
            p1 = 0;
            p2 = 0;
            p3 = 0;
        }
    }

    inline uint8_t lerpUI8(uint8_t lhs, uint8_t rhs, uint8_t t) {
        return Math::clamp<int32_t>(int32_t(multUI8((255 - t), lhs)) + int32_t(multUI8(rhs, t)), 0, 255);
    }

    inline Color32 lerp(Color32 lhs, Color32 rhs, float t) {
        return Color32(
            uint8_t(Math::lerp<float>(float(lhs.r), float(rhs.r), t)),
            uint8_t(Math::lerp<float>(float(lhs.g), float(rhs.g), t)),
            uint8_t(Math::lerp<float>(float(lhs.b), float(rhs.b), t)),
            uint8_t(Math::lerp<float>(float(lhs.a), float(rhs.a), t))
        );
    }

    template<typename T>
    inline void getCbCrY(const T& color, float& cb, float& cr, float& y) {
        y = (color.r * 0.299f) + (color.b * 0.587f) + (color.g * 0.114f);
        cr = 128 + (-0.168736f * color.r) + (-0.331264 * color.g) + (0.5f * color.b);
        cb = 128 + (0.5f * color.r) + (-0.418688f * color.g) + (-0.081312f * color.b);
    }

    inline float getCbCrYDist(float cb, float cr, float cbRhs, float crRhs, float toLA, float toLB) {
        cb = cbRhs - cb;
        cr = crRhs - cr;
        float ret = sqrtf((cb * cb) + (cr * cr));
        if (ret < toLA) { return 0.0f; }
        if (ret < toLB) { return (ret - toLA) / (toLB - toLA); }
        return 1.0f;
    }

    inline Color32& blendUI8(Color32& lhs, Color32 rhs) {
        if (lhs.a == 0) {
            lhs = rhs;
            return lhs;
        } else if (rhs.a <= 0) { return lhs; }

        int32_t a = (int32_t(lhs.a) + rhs.a) - multUI8(lhs.a, rhs.a);

        lhs.r = uint8_t(Math::clamp(int32_t(lhs.r) + (int32_t(rhs.r) - lhs.r) * rhs.a / a, 0x00, 0xFF));
        lhs.g = uint8_t(Math::clamp(int32_t(lhs.g) + (int32_t(rhs.g) - lhs.g) * rhs.a / a, 0x00, 0xFF));
        lhs.b = uint8_t(Math::clamp(int32_t(lhs.b) + (int32_t(rhs.b) - lhs.b) * rhs.a / a, 0x00, 0xFF));
        lhs.a = uint8_t(Math::clamp(a, 0x00, 0xFF));
        return lhs;
    }

    inline Color32& premultiplyC32(Color32& color) {
        color.r = multUI8(color.r, color.a);
        color.g = multUI8(color.g, color.a);
        color.b = multUI8(color.b, color.a);
        return color;
    }

    inline void premultiplyC32(Color32* colors, size_t count) {
        for (size_t i = 0; i < count; i++) {
            auto& color = colors[i];
            color.r = multUI8(color.r, color.a);
            color.g = multUI8(color.g, color.a);
            color.b = multUI8(color.b, color.a);
        }
    }

    size_t calculateTextureSize(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize);

    template<typename T>
    void convertPixel(TextureFormat format, const uint8_t* dataStart, const uint8_t* data, T& output) {
        static_assert("Not implemented for given type!");
    }

    template<>
    inline void convertPixel<Color32>(TextureFormat format, const uint8_t* dataStart, const uint8_t* data, Color32& output) {
        static PixelConvert<Color32> converts[int32_t(TextureFormat::Count)] {
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //Unknown
                    *output = Color32::Clear;
               },                                               
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //R8
                    memset(output, data[0], 3);
                    output->a = 0xFF;
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //RGB24
                    memcpy(output, data, 3);
                    output->a = 0xFF;
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //RGB48
                    const uint16_t* dataUI = reinterpret_cast<const uint16_t*>(data);
                    output->r = remapUI16ToUI8(*dataUI++);
                    output->g = remapUI16ToUI8(*dataUI++);
                    output->b = remapUI16ToUI8(*dataUI++);
                    output->a = 0xFF;
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //RGBA32
                    memcpy(output, data, 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //RGBA64
                    const uint16_t* dataUI = reinterpret_cast<const uint16_t*>(data);
                    output->r = remapUI16ToUI8(*dataUI++);
                    output->g = remapUI16ToUI8(*dataUI++);
                    output->b = remapUI16ToUI8(*dataUI++);
                    output->a = remapUI16ToUI8(*dataUI++);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //Indexed8
                    memcpy(output, (dataStart + (size_t(*data) << 2)), 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //Indexed16
                    memcpy(output, (dataStart + ((size_t(data[0]) | size_t(data[1]) << 8) << 2)), 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color32* output) {      //RGBA4444
                    *output = Color32(*reinterpret_cast<const Color4444*>(data));
               }
        };

        int32_t fmt = int32_t(format);
        if (fmt < 0 || format >= TextureFormat::Count) { output = Color32::Clear; return; }
        converts[fmt](dataStart, data, &output);
    }

    template<>
    inline void convertPixel<Color4444>(TextureFormat format, const uint8_t* dataStart, const uint8_t* data, Color4444& output) {
        static PixelConvert<Color4444> converts[int32_t(TextureFormat::Count)]{
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //Unknown
                    *output = Color4444();
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //R8
                    *output = Color4444(data[0] >> 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //RGB24
                   *output = Color4444(data[0] >> 4, data[1] >> 4, data[2] >> 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //RGB48
                   const uint16_t* pix = reinterpret_cast<const uint16_t*>(data);
                    *output = Color4444(pix[0] >> 12, pix[1] >> 12, pix[2] >> 12);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //RGBA32
                    *output = Color4444(data[0] >> 4, data[1] >> 4, data[2] >> 4, data[3] >> 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //RGBA64
                   const uint16_t* pix = reinterpret_cast<const uint16_t*>(data);
                    *output = Color4444(pix[0] >> 12, pix[1] >> 12, pix[2] >> 12, pix[3] >> 12);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //Indexed8
                    const uint8_t* pix = (dataStart + (size_t(*data) << 2));
                    *output = Color4444(pix[0] >> 4, pix[1] >> 4, pix[2] >> 4, pix[3] >> 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //Indexed16
                    const uint8_t* pix = (dataStart + ((size_t(data[0]) | size_t(data[1]) << 8) << 2));
                    *output = Color4444(pix[0] >> 4, pix[1] >> 4, pix[2] >> 4, pix[3] >> 4);
               },
               [](const uint8_t* dataStart, const uint8_t* data, Color4444* output) {      //RGBA4444
                    memcpy(output, data, 2);
               }
        };

        int32_t fmt = int32_t(format);
        if (fmt < 0 || format >= TextureFormat::Count) { output = Color4444(); return; }
        converts[fmt](dataStart, data, &output);
    }

    enum : uint8_t {
        IMG_FLAG_HAS_ALPHA = 0x1,
        IMG_FLAG_ALIGNED = 0x80,
    };

    struct ImageData {
    public:
        int32_t width{ 0 };
        int32_t height{ 0 };
        TextureFormat format{ TextureFormat::Unknown };
        int32_t paletteSize{ 0 };
        uint8_t* data{ nullptr };
        uint8_t flags{ 0 };

        constexpr ImageData() : width(), height(), format(), paletteSize(), data(), flags() {}
        constexpr ImageData(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize, uint8_t* data, uint8_t flags) :
            width(width), height(height), format(format), paletteSize(paletteSize), data(data), flags(flags) {}

        constexpr ImageData(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize, uint8_t* data) :
            width(width), height(height), format(format), paletteSize(paletteSize), data(data), flags(0) {}

        constexpr bool isIndexed() const { return format >= TextureFormat::Indexed8 && format <= TextureFormat::Indexed16; }
        constexpr bool isAligned() const { return (flags & IMG_FLAG_ALIGNED) != 0; }
        constexpr size_t getPaletteOffset() const { return getPaletteOffset(format, paletteSize, isAligned()); }

        constexpr uint8_t* getData() const { return data + getPaletteOffset(); }
        uint8_t* getData() { return data + getPaletteOffset(); }

        constexpr uint8_t* getStart() const { return data; }
        uint8_t* getStart() { return data; }

        constexpr uint8_t* getFramedData(size_t index) const {
            return (data + getPaletteOffset()) + (index * getSize());
        }
        uint8_t* getFramedData(size_t index) {
            return (data + getPaletteOffset()) + (index * getSize());
        }

        constexpr uint8_t* getFramedStart(size_t index) const {
            return data + (index * getSize());
        }
        uint8_t* getFramedStart(size_t index) {
            return data + (index * getSize());
        }

        constexpr uint8_t* getFramedStart(size_t index, size_t frameSize) const {
            return data + (index * frameSize);
        }
        uint8_t* getFramedStart(size_t index, size_t frameSize) {
            return data + (index * frameSize);
        }

        constexpr size_t getBufferSize() const { return _bufferSize; }

        ImageData getFramed(size_t frame) const {
            if (data == nullptr) { return *this; }
            size_t framePos = frame * getSize();

            ImageData temp{};
            temp._bufferSize = _bufferSize - framePos;
            temp.data = data + framePos;
            temp.paletteSize = paletteSize;
            temp.format = format;
            temp.width = width;
            temp.height = height;
            temp.flags = flags;
            return temp;
        }

        constexpr size_t getSize() const {
            return calculateSize(width, height, format, paletteSize, (this->flags & IMG_FLAG_ALIGNED) != 0);
        }

        bool isEqual(const ImageData& img) const {
            if (this == &img) { return true; }
            bool indexed = isIndexed();
            if (_bufferSize != img._bufferSize ||
                width != img.width || height != img.height ||
                format != img.format || indexed != img.isIndexed() || (indexed && paletteSize != img.paletteSize)) {
                return false;
            }
            return memcmp(data, img.data, _bufferSize) == 0;
        }

        static constexpr bool isIndexed(TextureFormat format) {
            switch (format) {
            case TextureFormat::Indexed8:
            case TextureFormat::Indexed16: return true;
            default: return false;
            }
        }

        static constexpr int32_t getPaletteSize(TextureFormat format, int32_t rawPaletteSize, bool align) {
            switch (format) {
            case TextureFormat::Indexed8:
                return align ? 256 : Math::clamp(rawPaletteSize, 0, 256);
            case TextureFormat::Indexed16:
                return align ? Math::alignToPalette(rawPaletteSize) : Math::clamp(rawPaletteSize, 0, 256 * 256);
            default: return 0;
            }
        }

        static constexpr size_t calculateSize(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize, bool alignedPalette) {
            return (size_t(width) * height) * (getBitsPerPixel(format) >> 3) + (getPaletteSize(format, paletteSize, alignedPalette) * sizeof(Color32));
        }

        static constexpr size_t getPaletteOffset(TextureFormat format, int32_t paletteSize, bool alignedPalette) {
            return getPaletteSize(format, paletteSize, alignedPalette) * sizeof(Color32);
        }

        bool doAllocate() {
            size_t required = getSize();
            if (data) {
                if (required <= _bufferSize) { return true; }
                void* reloc = realloc(data, required);
                if (!reloc) { return false; }
                data = reinterpret_cast<uint8_t*>(reloc);
            }
            else
            {
                data = reinterpret_cast<uint8_t*>(malloc(required));
            }

            _bufferSize = required;
            if (data) {
                memset(data, 0, required);
            }
            return data != nullptr;
        }

        bool doAllocate(size_t size, bool clear = true) {
            size_t required = size;
            if (data) {
                if (required <= _bufferSize)
                {
                    if (clear) {
                        memset(data, 0, size);
                    }
                    return true;
                }

                void* reloc = realloc(data, required);
                if (!reloc) { return false; }
                data = reinterpret_cast<uint8_t*>(reloc);
                _bufferSize = required;
            }
            else
            {
                data = reinterpret_cast<uint8_t*>(malloc(required));
                _bufferSize = required;
            }

            if (data && clear) {
                memset(data, 0, required);
            }
            return data != nullptr;
        }

        bool doAllocate(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize = 0, bool alignedPalette = false, uint32_t frames = 1, bool modify = true, bool clear = true) {
            if (modify) {
                this->width = width;
                this->height = height;
                this->format = format;
                this->flags = alignedPalette ? (this->flags | IMG_FLAG_ALIGNED) : (this->flags & ~IMG_FLAG_ALIGNED);
                this->paletteSize = getPaletteSize(format, paletteSize, alignedPalette);
            }
            return doAllocate(calculateSize(width, height, format, paletteSize, alignedPalette) * frames);
        }

        void resize(int32_t newWidth, int32_t newHeight, bool linear, ImageData* tempBuffer = nullptr);

        void replaceData(uint8_t* newData, bool destroy);
        void clear(bool destroy);
    private:
        size_t _bufferSize{ 0 };
    };

    struct ImageBuffers {
    public:
        enum : uint8_t {
            //Buffer Indices
            Buf_In = 0,
            Buf_Out = 1,
            Buf_Aux0 = 2,
            Buf_Aux1 = 3,

            //Flags
            Own_Buf_In = 1 << Buf_In,
            Own_Buf_Out = 1 << Buf_Out,
            Own_Buf_Aux0 = 1 << Buf_Aux0,
            Own_Buf_Aux1 = 1 << Buf_Aux1,
        };

        ImageBuffers() : _flags(0), _buffers{ } {}
        ImageBuffers(uint8_t flags, ImageData buffers[4]) :
            _flags(flags),
            _buffers{ buffers[0], buffers[1], buffers[2], buffers[3] } {}

        ~ImageBuffers();

        const ImageData& operator[](int32_t index) const;
        ImageData& operator[](int32_t index);

        bool getOwnFlag(uint8_t flag) const;
        void setOwnFlag(uint8_t flag, bool value);
        void allocBuffer(uint8_t index, size_t size);
        void clear();

    private:
        uint8_t _flags{ 0 };
        ImageData _buffers[4]{ };
    };

    uint8_t calculateColorVariance(const ImageData& img, bool ignoreClear);
}