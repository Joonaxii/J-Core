#include <J-Core/IO/ImageUtils.h>
#include <J-Core/Log.h>
#include <J-Core/Math/Math.h>
#include <J-Core/ThreadManager.h>
#include <unordered_set>
#include <glm.hpp>
#include <stack>
#include <queue>

namespace JCore {

    bool hasAlpha(const uint8_t* data, int32_t width, int32_t height, TextureFormat format, int32_t paletteSize) {
        int32_t reso = width * height;
        switch (format) {
            default: return false;

            case JCore::TextureFormat::RGBA32: {
                for (size_t i = 0, j = 3; i < reso; i++, j += 4) {
                    if (data[j] < 0xFF) { return true; }
                }
                return false;
            }

            case JCore::TextureFormat::RGBA4444: {
                const uint16_t* ptr = reinterpret_cast<const uint16_t*>(data);
                for (size_t i = 0; i < reso; i++) {
                    if ((ptr[i] & 0xF000) != 0xF000) { return true; }
                }
                return false;
            }

            case JCore::TextureFormat::Indexed8:
                paletteSize = 256;
                break;
            case JCore::TextureFormat::Indexed16:
                paletteSize = paletteSize < -1 ? 256 * 256 : paletteSize;
                break;
        }

        bool alpha = false;
        for (size_t i = 0, j = 3; i < paletteSize; i++, j += 4) {
            if (data[j] < 0xFF) { alpha = true; break; }
        }
        if (!alpha) { return false; }

        const Color32* palette = reinterpret_cast<const Color32*>(data);
        switch (format) {
            case JCore::TextureFormat::Indexed8: {
                const uint8_t* pixels = data + paletteSize * 4;
                for (size_t i = 0; i < reso; i++) {
                    if (palette[pixels[i]].a < 255) { return true; }
                }
                break;
            }
            case JCore::TextureFormat::Indexed16: {
                const uint16_t* pixels = reinterpret_cast<const uint16_t*>(data + paletteSize * 4);
                for (size_t i = 0; i < reso; i++) {
                    if (palette[pixels[i]].a < 255) { return true; }
                }
                break;
            }
        }
        return false;
    }

    int32_t findInPalette(const Color32 color, const Color32* palette, uint32_t size) {
        for (size_t i = 0; i < size; i++) {
            if (color == palette[i]) { return int32_t(i); }
        }
        return -1;
    }

    bool tryBuildPalette8(Color32 pixel, int32_t& colors, TextureFormat& format, uint8_t* newPalette, Color32 buffer[32], uint32_t& bufSize, int32_t alphaClip) {
        int32_t target = format == TextureFormat::Indexed16 ? 256 * 256 : 256;
        if (pixel.a <= alphaClip) {
            *reinterpret_cast<uint32_t*>(&pixel) = 0;
        }

        uint32_t bSize = bufSize > 8 ? 8 : bufSize;
        for (size_t i = 0; i < bSize; i++) {
            if (buffer[i] == pixel) { return true; }
        }

        Color32* palette = reinterpret_cast<Color32*>(newPalette);
        int32_t ind = findInPalette(pixel, palette, colors);
        if (ind < 0) {
            palette[colors++] = pixel;
            if (colors > target) {
                if (colors > 256 * 256) {
                    JCORE_WARN("Couldn't build palette, too many colors!");
                    return false;
                }
                target = 256 * 256;
                format = TextureFormat::Indexed16;
            }
        }
        buffer[bufSize & 31] = pixel;
        bufSize++;
        return true;
    }
    bool tryBuildPalette16(Color32 pixel, int32_t& colors, TextureFormat& format, uint8_t* newPalette, Color32 buffer[256], uint32_t& bufSize, int32_t alphaClip, int32_t maxSize) {
        int32_t target = 256 * 256;
        if (pixel.a <= alphaClip) {
            *reinterpret_cast<uint32_t*>(&pixel) = 0;
        }
        maxSize = maxSize < 0 || maxSize > 256 * 256 ? 256 * 256 : maxSize;

        uint32_t bSize = bufSize > 8 ? 8 : bufSize;
        for (size_t i = 0; i < bSize; i++) {
            if (buffer[i] == pixel) { return true; }
        }

        Color32* palette = reinterpret_cast<Color32*>(newPalette);
        int32_t ind = findInPalette(pixel, palette, colors);
        if (ind < 0) {
            palette[colors++] = pixel;
            if (colors > target) {
                if (colors > maxSize) {
                    JCORE_WARN("Couldn't build palette, too many colors!");
                    return false;
                }
            }
        }
        buffer[bufSize & 0xFF] = pixel;
        bufSize++;
        return true;
    }

    bool tryBuildPalette(Color32 pixel, int32_t& colors, TextureFormat& format, uint8_t* newPalette, std::unordered_map<uint32_t, int32_t>& paletteLut, int32_t alphaClip, int32_t absoluteMax) {
        absoluteMax = absoluteMax <= 0 ? 256 * 256 : absoluteMax;

        uint32_t& colorInt = reinterpret_cast<uint32_t&>(pixel);
        if (pixel.a <= alphaClip) {
            colorInt = 0;
        }

        Color32* palette = reinterpret_cast<Color32*>(newPalette);
        auto find = paletteLut.find(colorInt);

        if (find == paletteLut.end()) {
            paletteLut[colorInt] = colors;
            palette[colors++] = pixel;

            if (colors > absoluteMax) {
                JCORE_WARN("Couldn't build palette, too many colors!");
                return false;
            }

            if (format == TextureFormat::Indexed8 && colors > 256) {
                format = TextureFormat::Indexed16;
            }
        }
        return true;
    }
    bool tryBuildPalette(const uint8_t* pixelData, int32_t start, int32_t length, int32_t& colors, TextureFormat& format, int32_t bpp, uint8_t* newPalette, int32_t alphaClip) {
        if (bpp < 2) {
            format = TextureFormat::Unknown;
            JCORE_WARN("[J-Core - ImageUtils] Warning: Couldn't build palette, bpp is too small {0}!", bpp);
            return false;
        }

        Color32 buffer{ 0,0,0,0xFF };
        int32_t target = format == TextureFormat::Indexed16 ? 256 * 256 : 256;

        Color32* palette = reinterpret_cast<Color32*>(newPalette);
        for (size_t i = 0, j = 0, k = start; i < length; i++, j += bpp, k++) {
            memcpy(&buffer, pixelData + j, bpp);
            if (buffer.a <= alphaClip) {
                *reinterpret_cast<uint32_t*>(&buffer) = 0;
            }

            int32_t ind = findInPalette(buffer, palette, colors);
            if (ind < 0) {
                if (colors >= target) {
                    if (colors >= 256 * 256) {
                        JCORE_WARN("[J-Core - ImageUtils] Warning: Couldn't build palette, too many colors!");
                        return false;
                    }
                    target = 256 * 256;
                    format = TextureFormat::Indexed16;
                }
                palette[colors++] = buffer;
            }
        }
        return true;
    }

    void applyPalette(const uint8_t* pixelData, int32_t width, int32_t height, int32_t colors, TextureFormat format, uint8_t* targetPixels, TextureFormat paletteFmt, int32_t alphaClip) {

        int32_t reso = width * height;
        Color32* palette = reinterpret_cast<Color32*>(targetPixels);
        Color32 colorBuf{ 0,0,0,0xFF };
        int32_t bpp = getBitsPerPixel(format) >> 3;
        switch (paletteFmt)
        {
            case JCore::TextureFormat::Indexed8: {
                uint8_t* pixData = targetPixels + 256 * 4;
                for (size_t i = 0, j = 0; i < reso; i++, j = 0) {
                    memcpy(&colorBuf, pixelData + j, bpp);
                    if (colorBuf.a <= alphaClip) {
                        *reinterpret_cast<uint32_t*>(&colorBuf) = 0;
                    }
                    int32_t index = findInPalette(colorBuf, palette, colors);
                    pixData[i] = uint8_t(index < 0 ? 0 : index);
                }
                break;
            }
            case JCore::TextureFormat::Indexed16: {
                uint16_t* pixData = reinterpret_cast<uint16_t*>(targetPixels + colors * 4);
                for (size_t i = 0, j = 0; i < reso; i++, j = 0) {
                    memcpy(&colorBuf, pixelData + j, bpp);
                    if (colorBuf.a <= alphaClip) {
                        *reinterpret_cast<uint32_t*>(&colorBuf) = 0;
                    }
                    int32_t index = findInPalette(colorBuf, palette, colors);
                    pixData[i] = uint16_t(index < 0 ? 0 : index);
                }
                break;
            }
            default:
                JCORE_WARN("Couldn't apply palette, invalid texture format! ({0})", getTextureFormatName(format));
                break;
        }
    }

    int32_t tryBuildPalette(const uint8_t* pixelData, uint32_t width, uint32_t height, TextureFormat format, uint8_t* newTexture, int32_t alphaClip) {
        switch (format)
        {
            case JCore::TextureFormat::Unknown:
                JCORE_WARN("Couldn't build palette, texture format unknown!");
                return 0;
            case JCore::TextureFormat::Indexed8:
            case JCore::TextureFormat::Indexed16:
                JCORE_WARN("Didn't build palette, texture is already indexed!");
                return 0;
        }

        int32_t colCount = 0;
        Color32 buffer{};

        Color32* palette = reinterpret_cast<Color32*>(newTexture);
        for (size_t i = 0; i < 256; i++) {
            palette[i].a = 0xFF;
        }

        uint32_t bpp = getBitsPerPixel(format);
        uint32_t reso = width * height;
        uint8_t* newPixData = newTexture + 1024;
        for (size_t i = 0, j = 0; i < reso; i++, j += bpp) {
            memcpy(&buffer, pixelData + j, bpp);
            if (buffer.a <= alphaClip) {
                *reinterpret_cast<uint32_t*>(&buffer) = 0;
            }

            int32_t ind = findInPalette(buffer, palette, colCount);
            if (ind < 0) {
                if (colCount >= 256) {
                    JCORE_WARN("Couldn't build palette, too many colors!");
                    return false;
                }

                ind = colCount;
                palette[colCount++] = buffer;
            }
            newPixData[i] = ind;
        }
        return colCount;
    }

    uint8_t quantizeUI8(uint8_t value, int32_t bits){
        if (bits < 1 || bits > 7) { return value; }
        static bool init{ false };
        static uint8_t LUT[256 * 7]{ 0 };

        if (!init) {
            size_t offset = 0;
            for (size_t i = 0, j = 1; i < 8; i++, j++) {
                float fLev = float(1 << j) - 1.0f;
                for (size_t k = 0; k < 256; k++) {
                    LUT[offset + k] = uint8_t(std::clamp<float>(std::round(k / 255.0f * fLev) / fLev * 255.0f, 0, 255.0f));
                }
                offset += 256;
            }
            init = true;
        }

        return LUT[(bits - 1) * 256 + value];
    }

    uint8_t remapUI16ToUI8(uint16_t value) {
        static uint8_t REMAPPED[UINT16_MAX+1]{0};
        static bool init{ false };
        if (!init) {
            for (size_t i = 0; i <= UINT16_MAX; i++) {
                REMAPPED[i] = uint8_t(i / 256);
            }
            init = true;
        }
        return REMAPPED[value];
    }

    void unpackRGB565(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b) {
        static uint8_t RGB565LUT[32 + 64]{};
        static bool init{ false };

        if (!init) {
            for (size_t i = 0; i < 32; i++) {
                RGB565LUT[i] = uint8_t((i * 255) / 31);
            }

            for (size_t i = 0; i < 64; i++) {
                RGB565LUT[i + 32] = uint8_t((i * 255) / 63);
            }
            init = true;
        }

        r = RGB565LUT[(value & RED_MASK_565)];
        g = RGB565LUT[32 + ((value & GREEN_MASK_565) >> 5)];
        b = RGB565LUT[(value & BLUE_MASK_565) >> 11];
    }

    void unpackRGB555(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b) {
        uint8_t a{};
        unpackRGB555(value, r, g, b, a);
    }

    void unpackRGB555(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
        static uint8_t RGB555LUT[32]{};
        static bool init{ false };

        if (!init) {
            for (size_t i = 0; i < 32; i++) {
                RGB555LUT[i] = uint8_t((i * 255) / 31);
            }
            init = true;
        }

        r = RGB555LUT[(value & RED_MASK_555)];
        g = RGB555LUT[(value & GREEN_MASK_555) >> 5];
        b = RGB555LUT[(value & BLUE_MASK_555) >> 10];
        a = value & ALPHA_MASK_555 ? 0xFF : 0x00;
    }

    void unpackRGB4444(uint16_t value, uint8_t& r, uint8_t& g, uint8_t& b, uint8_t& a) {
        static uint8_t RGB4444LUT[32]{};
        static bool init{ false };

        if (!init) {
            for (size_t i = 0; i < 16; i++) {
                RGB4444LUT[i] = uint8_t((i * 255) / 15);
            }
            init = true;
        }
        r = RGB4444LUT[value & 0xF];
        g = RGB4444LUT[(value >> 4) & 0xF];
        b = RGB4444LUT[(value >> 8) & 0xF];
        a = RGB4444LUT[(value >> 12) & 0xF];
    }

    size_t calculateTextureSize(int32_t width, int32_t height, TextureFormat format, int32_t paletteSize) {
        switch (format) {
            case JCore::TextureFormat::R8:
                return size_t(width) * height;
            case JCore::TextureFormat::RGB24:
                return size_t(width) * height * 3;
            case JCore::TextureFormat::RGBA32:
                return size_t(width) * height * 4;           
            case JCore::TextureFormat::RGB48:
                return size_t(width) * height * 6;
            case JCore::TextureFormat::RGBA64:
                return size_t(width) * height * 8;
            case JCore::TextureFormat::Indexed8:
                return size_t(width) * height + 1024;
            case JCore::TextureFormat::Indexed16:
                paletteSize = Math::isAlignedToPalette(paletteSize) ? paletteSize : Math::alignToPalette(paletteSize);
                return (size_t(width) * height * 2) + size_t(paletteSize) * 4;
            default: return 0;
        }
    }

    bool tryBuildPalette(const uint8_t* pixelData, int32_t start, int32_t length, int32_t& colors, int32_t bpp, uint8_t* newTexture, int32_t alphaClip) {
        Color32 buffer{ 0,0,0,0xFF };

        Color32* palette = reinterpret_cast<Color32*>(newTexture);
        uint8_t* newPix = (newTexture + 1024);
        for (size_t i = 0, j = 0, k = start; i < length; i++, j += bpp, k++) {
            memcpy(&buffer, pixelData + j, bpp);
            if (buffer.a <= alphaClip) {
                *reinterpret_cast<uint32_t*>(&buffer) = 0;
            }
            int32_t ind = findInPalette(buffer, palette, colors);
            if (ind < 0) {
                if (colors >= 256) {
                    JCORE_WARN("Couldn't build palette, too many colors!");
                    return false;
                }

                palette[colors] = buffer;
                ind = colors++;
            }
            newPix[k] = uint8_t(ind);
        }
        return true;
    }

    ImageBuffers::~ImageBuffers() { clear(); }
    const ImageData& ImageBuffers::operator[](int32_t index) const {
        JCORE_ASSERT(index >= 0 && index < 4, "Index out of range");
        return _buffers[index];
    }

    ImageData& ImageBuffers::operator[](int32_t index) {
        JCORE_ASSERT(index >= 0 && index < 4, "Index out of range");
        return _buffers[index];
    }

    bool ImageBuffers::getOwnFlag(uint8_t flag) const { return bool(_flags & flag); }
    void ImageBuffers::setOwnFlag(uint8_t flag, bool value) {
        _flags &= ~flag;
        if (value) {
            _flags |= flag;
        }
    }

    void ImageBuffers::allocBuffer(uint8_t index, size_t size) {
        JCORE_ASSERT(index >= 0 && index < 4, "Index out of range");
        uint8_t flag = (1 << index);
        auto& buf = _buffers[index];
        if (_flags & flag) {
            void* reloc = realloc(buf.data, size);
            if (!reloc) {
                JCORE_ERROR("[ImageUtils - Alloc] Failed to reallocate buffer! ({0} bytes)", size);
                return;
            }
            buf.data = reinterpret_cast<uint8_t*>(reloc);
        }
        else {
            buf.data = reinterpret_cast<uint8_t*>(malloc(size));
            if (!buf.data) {
                JCORE_ERROR("[ImageUtils - Alloc] Failed to allocate buffer! ({0} bytes)", size);
                return;
            }
        }
        _flags |= (1 << index);
    }

    void ImageBuffers::clear() {
        for (size_t i = 0, j = 1; i < 4; i++, j <<= 1) {
            _buffers[i].clear((_flags & j));
        }
    }

    void ImageData::resize(int32_t newWidth, int32_t newHeight, uint8_t* buffer) {
        if ((newWidth == width && newHeight == height) || ((newHeight & newWidth) & 0x7FFFFFFF) == 0 || !data) { return; }

        int32_t oldW = width;
        int32_t oldH = height;

        int32_t bpp = (getBitsPerPixel(format) >> 3);
        int32_t resoNew = newWidth * newHeight;
        int32_t resoOld = width * height;

        bool newBuf = buffer == nullptr;

        size_t offset = isIndexed() ? paletteSize * sizeof(Color32) : 0;
        uint8_t* selfBuf = data + offset;

        if (newBuf) {
            buffer = reinterpret_cast<uint8_t*>(_malloca(resoOld * bpp));
            if (!buffer) { return; }
        }
        memcpy(buffer, selfBuf, resoOld * bpp);

        width = newWidth;
        height = newHeight;

        if (resoNew > resoOld) { doAllocate(); }

        int32_t off = newWidth * bpp;
        int32_t offOld = oldW * bpp;
        for (size_t y = 0, yP = 0; y < newHeight; y++, yP += off) {
            int32_t srY = Math::lerp<int32_t>(0, (oldH - 1), y / float(newHeight - 1.0f)) * offOld;
            for (size_t x = 0, xP = yP; x < newWidth; x++, xP += bpp) {
                int32_t srX = Math::lerp<int32_t>(0, (oldW - 1), x / float(newWidth - 1.0f)) * bpp;
                memcpy(data + xP, buffer + srY + srX, bpp);
            }
        }

        if (newBuf) {
            _freea(buffer);
        }
    }

    void ImageData::replaceData(uint8_t* newData, bool destroy) {
        if (destroy && data) {
            free(data);
        }
        data = newData;
    }

    void ImageData::clear(bool destroy) {
        if (destroy && data) {
            free(data);
        }
        data = nullptr;
        format = TextureFormat::Unknown;
        paletteSize = 0;
        width = 0;
        height = 0;
        flags = 0;
    }

    uint8_t calculateColorVariance(const ImageData& img, bool ignoreClear) {
        if (!img.data) { return 0; }
        const Color32* pixels = reinterpret_cast<const Color32*>(img.data);

        size_t offset = img.paletteSize * 4;
        std::function<bool(Color24&, int32_t)> formats[]{
            {},

            [&img](Color24& color, int32_t index) {
                color.r = color.g = color.b = img.data[index];
                return true;
            },

            [&img](Color24& color, int32_t index) {
                color = reinterpret_cast<const Color24*>(img.data)[index];
                return true;
            },

            //RGB48
            [&img](Color24& color, int32_t index) {
                const uint16_t* data = reinterpret_cast<const uint16_t*>(img.data) + index * 3;
                color.r = remapUI16ToUI8(data[0]);
                color.g = remapUI16ToUI8(data[1]);
                color.b = remapUI16ToUI8(data[2]);
                return true;
            },
            
            
            [&img, pixels, ignoreClear](Color24& color, int32_t index) {
                const Color32& tmp = pixels[index];
                if (tmp.a < 1 && ignoreClear) { return false; }
                memcpy(&color, &tmp, sizeof(Color24));
                return true;
            },
            
            //RGBA64
            [&img, ignoreClear](Color24& color, int32_t index) {
                 const uint16_t* data = reinterpret_cast<const uint16_t*>(img.data) + index * 4;
                 if (data[3] < 1 && ignoreClear) { return false; }
                 color.r = remapUI16ToUI8(data[0]);
                 color.g = remapUI16ToUI8(data[1]);
                 color.b = remapUI16ToUI8(data[2]);
                 return true;
            },
            
            [&img, pixels, ignoreClear](Color24& color, int32_t index) {
                 auto& tmp = pixels[img.data[index + 1024]];
                 if (tmp.a < 1 && ignoreClear) { return false; }
                 memcpy(&color, &tmp, sizeof(Color24));
                 return true;
            },
            
            [&img, pixels, offset, ignoreClear](Color24& color, int32_t index) {
                 auto& tmp = pixels[reinterpret_cast<const uint16_t*>(img.data + offset)[index]];;
                 if (tmp.a < 1 && ignoreClear) { return false; }
                 memcpy(&color, &tmp, sizeof(Color24));
                 return true;
            },

        };
        auto& func = formats[(int32_t)img.format];
        Color24 temp{};

        size_t reso = img.width * img.height;
        std::unordered_set<Color24> colors{};
        size_t total = 0;
        for (int32_t i = 0; i < reso; i++) {
            if (func(temp, i)) {
                colors.insert(temp);
                total++;
            }
        }
        return total < 1 ? 0 : uint8_t((colors.size() / float(total)) * 255.0f);
    }
}