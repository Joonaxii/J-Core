#pragma once
#include <cstdint>
#include <functional>
#include <J-Core/Math/Color24.h>
#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Color555.h>
#include <J-Core/Math/Color565.h>
#include <J-Core/Math/Math.h>
#include <unordered_map>
#include <glm.hpp>

namespace JCore {
    template<typename T>
    using PixelConvert = std::function<void(const uint8_t*, const uint8_t*, T*)>;

    template<typename T>
    struct BezierCurve {
    public:
        float points[4]{ 0 };

        BezierCurve() : points{ 0, 1, 0, 1 } {
            bake();
        }

        BezierCurve(const glm::vec2& p0, const glm::vec2& p1) : points{ p0.x, p0.y, p1.x, p1.y } {
            bake();
        }

        const T* getValues() const { return _table; }

        const T* evaluate(float t) const {
            return _table + size_t((t < 0 ? 0 : t> 1 ? 1 : t) * 512);
        }

        const T* evaluate(uint32_t t) const {
            t = t > 255 ? 255 : t;
            return _table + t * 2;
        }

        const glm::vec2 getNormalized(uint32_t index) const {
            index = index > 255 ? 255 : index;
            index *= 2;
            return {
            Math::inverseLerp<T, float>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), float(_table[index])),
            Math::inverseLerp<T, float>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), float(_table[index + 1])),
            };
        }

        void bake() {
            glm::vec2 P[4]{ {0, 0}, {points[0], points[1]}, {points[2], points[3]}, {1, 1}};
            static float C[256 * 4]{ 0 }, * K = 0;
            if (!K) {
                K = C;
                for (size_t step = 0; step < 256; ++step) {
                    float t = (float)step / (float)255;
                    C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);
                    C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t;
                    C[step * 4 + 2] = 3 * (1 - t) * t * t;
                    C[step * 4 + 3] = t * t * t;
                }
            }

            for (size_t step = 0, stepX = 0, stepP = 0; step < 256; step++, stepX += 4, stepP += 2) {
               float temp = K[stepX + 0] * P[0].x + K[stepX + 1] * P[1].x + K[stepX + 2] * P[2].x + K[stepX + 3] * P[3].x;
               temp = temp < 0 ? 0 : temp > 1 ? 1 : temp;
               _table[stepP] = (Math::lerp<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), temp));

               temp = K[stepX + 0] * P[0].y + K[stepX + 1] * P[1].y + K[stepX + 2] * P[2].y + K[stepX + 3] * P[3].y;
               temp = temp < 0 ? 0 : temp > 1 ? 1 : temp;
               _table[stepP + 1] = (Math::lerp<T>(std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), temp));
            }
        }

    private:
        T _table[256*2]{ 0 };
    };

    template<>
    void BezierCurve<float>::bake() {
        glm::vec2 P[4]{ {0, 0}, {points[0], points[1]}, {points[2], points[3]},  {1, 1} };
        static float C[(256 + 1) * 4]{ 0 }, * K = 0;
        if (!K) {
            K = C;
            for (size_t step = 0; step <= 256; ++step) {
                float t = (float)step * (1.0f / 256.0f);
                C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);
                C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t;
                C[step * 4 + 2] = 3 * (1 - t) * t * t;
                C[step * 4 + 3] = t * t * t;
            }
        }

        for (size_t step = 0, stepX = 0, stepP = 0; step <= 256; step++, stepX += 4, stepP += 2) {
            _table[stepP] = K[stepX + 0] * P[0].x + K[stepX + 1] * P[1].x + K[stepX + 2] * P[2].x + K[stepX + 3] * P[3].x;
            _table[stepP + 1] = K[stepX + 0] * P[0].y + K[stepX + 1] * P[1].y + K[stepX + 2] * P[2].y + K[stepX + 3] * P[3].y;
        }
    }

    template<>
    const glm::vec2 BezierCurve<float>::getNormalized(uint32_t index) const {
        index = index > 255 ? 255 : index;
        index *= 2;
        return {
        _table[index],
        _table[index + 1],
        };
    }

    template<> 
    void BezierCurve<double>::bake() {
        glm::dvec2 P[4]{ {0, 0}, {points[0], points[1]}, {points[2], points[3]},  {1, 1} };
        static double C[(256 + 1) * 4]{ 0 }, * K = 0;
        if (!K) {
            K = C;
            for (size_t step = 0; step <= 256; ++step) {
                double t = (double)step * (1.0 / 256.0);
                C[step * 4 + 0] = (1 - t) * (1 - t) * (1 - t);
                C[step * 4 + 1] = 3 * (1 - t) * (1 - t) * t;
                C[step * 4 + 2] = 3 * (1 - t) * t * t;
                C[step * 4 + 3] = t * t * t;
            }
        }

        for (size_t step = 0, stepX = 0, stepP = 0; step <= 256; step++, stepX += 4, stepP += 2) {
            _table[stepP] = K[stepX + 0] * P[0].x + K[stepX + 1] * P[1].x + K[stepX + 2] * P[2].x + K[stepX + 3] * P[3].x;
            _table[stepP + 1] = K[stepX + 0] * P[0].y + K[stepX + 1] * P[1].y + K[stepX + 2] * P[2].y + K[stepX + 3] * P[3].y;
        }
    }

    template<>
    const glm::vec2 BezierCurve<double>::getNormalized(uint32_t index) const {
        index = index > 255 ? 255 : index;
        index *= 2;
        return {
        float(_table[index]),
        float(_table[index + 1]),
        };
    }

    enum class TextureFormat : uint8_t {
        Unknown,

        R8,
        RGB24,
        RGB48,
        RGBA32,
        RGBA64,
        Indexed8,
        Indexed16,

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
    void unpackRGB565(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b);
    void unpackRGB555(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b);
    void unpackRGB555(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a);

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

    template<typename T, typename P = int32_t>
    inline P channelDistance(const T& a, const T& b) {
        return (std::abs(P(a.r) - b.r) + std::abs(P(a.g) - b.g) + std::abs(P(a.b) - b.b)) / P(3);
    }

    inline uint8_t colorToAlpha(Color32 input, Color32 ref, int32_t floor, int32_t ceiling) {
        float diff = channelDistance<Color32, float>(input, ref);
        return uint8_t((Math::clamp(Math::inverseLerp<float>(float(floor), float(ceiling), diff), 0.0f, 1.0f)) * 255.0f);
    }

   /* inline Color32 colorToAlpha(Color32 pix, Color32 ref, uint8_t maxA = 0xFF, uint8_t maxCha = 0xFF) {
        if (ref.a < 1 || pix.a < 1) { return pix; }

        int32_t r = 0, g = 0, b = 0, a = 0;

        if (pix.r > ref.r) { r = multUI8(maxA, divUI8((pix.r - ref.r), maxCha < pix.r ? 0x00 : (maxCha - pix.r))); }
        else if (ref.r > pix.r) { r = divUI8(multUI8(maxCha, (ref.r - pix.r)), ref.r); }

        if (pix.g > ref.g) { g = multUI8(maxA, divUI8((pix.g - ref.g), maxCha < pix.g ? 0x00 : (maxCha - pix.g))); }
        else if (ref.g > pix.g) { g = divUI8(multUI8(maxCha, (ref.g - pix.g)), ref.g); }

        if (pix.b > ref.b) { b = multUI8(maxA, divUI8((pix.b - ref.b), maxCha < pix.b ? 0x00 : (maxCha - pix.b))); }
        else if (ref.b > pix.b) { b = divUI8(multUI8(maxCha, (ref.b - pix.b)), ref.b); }

        a = r;
        if (g > a) { a = g; }
        if (b > a) { a = b; }

        if (a >= divUI8(maxA, maxCha)) {
            return Color32(
                divUI8(multUI8(maxA, (pix.r - ref.r)), a) + ref.r,
                divUI8(multUI8(maxA, (pix.g - ref.g)), a) + ref.g,
                divUI8(multUI8(maxA, (pix.b - ref.b)), a) + ref.b,
                divUI8(multUI8(a, pix.a), maxA));
        }
        return Color32::Clear;
    }
    */
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

    inline Color32& premultiply(Color32& color) {
        color.r = multUI8(color.r, color.a);
        color.g = multUI8(color.g, color.a);
        color.b = multUI8(color.b, color.a);
        return color;
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
               }
        };

        int32_t fmt = int32_t(format);
        if (fmt < 0 || format >= TextureFormat::Count) { output = Color32::Clear; return; }
        converts[fmt](dataStart, data, &output);
    }

    enum : uint8_t {
        IMG_FLAG_HAS_ALPHA = 0x1,
    };

    struct ImageData {
    public:
        int32_t width{ 0 };
        int32_t height{ 0 };
        TextureFormat format{ TextureFormat::Unknown };
        int32_t paletteSize{ 0 };
        uint8_t* data{ nullptr };
        uint8_t flags{0};

        ImageData() : width(), height(), format(), paletteSize(), data(), flags() {}
        ImageData(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize, uint8_t* data, uint8_t flags) : 
            width(width), height(height), format(format), paletteSize(paletteSize), data(data), flags(flags) {}

        ImageData(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize, uint8_t* data) : 
            width(width), height(height), format(format), paletteSize(paletteSize), data(data), flags(0) {}

        bool isIndexed() const { return format >= TextureFormat::Indexed8 && format <= TextureFormat::Indexed16; }

        size_t getSize() const {
            size_t required = width * height;
            required *= (getBitsPerPixel(format) >> 3);
            if (isIndexed()) {
                required += paletteSize * sizeof(Color32);
            }
            return required;
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

        void resize(int32_t newWidth, int32_t newHeight, uint8_t* buffer = nullptr);

        void replaceData(uint8_t* newData, bool destroy);
        void clear(bool destroy);
    private:
        size_t _bufferSize{0};
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