#pragma once
#include <cstdint>
#include <J-Core/IO/Stream.h>
#include <J-Core/Util/Span.h>
#include <bitset>
#include <functional>
#include <J-Core/Util/EnumUtils.h>

namespace JCore {
    enum DataFormat : uint8_t {
        FMT_UNKNOWN = 0xFF,
        FMT_BINARY = 0,
        FMT_TEXT,

        FMT_JSON, //Requires complex analysis
        FMT_YAML, //Requires complex analysis

        _FMT_START,

        //Texture formats
        FMT_PNG,
        FMT_BMP,
        FMT_DDS,
        FMT_JPEG,
        FMT_GIF87,
        FMT_GIF89,
        FMT_WEBP,

        //Audio formats
        FMT_WAVE,
        FMT_OGG,
        FMT_MP3,

        //Font formats
        FMT_TTF,
        FMT_OTF,

        //J Formats
        FMT_JTEX,

        _FMT_COUNT,
    };

    enum FormatNameSource {
        FMTN_NAME = 0x0,
        FMTN_EXTENSION = 0x1,
    };

    struct FormatInfo {
        const char* signature{0};
        uint64_t mask = 0;
        size_t size = 0;

        FormatInfo() : signature{0}, mask(0), size(0) {}
        FormatInfo(const void* sig, uint64_t mask, size_t size) :
            signature{reinterpret_cast<const char*>(sig)}, mask(mask), size(size > 64 ? 64 : size)
        {
        }

        bool isValid(const void* data, size_t dataSize) const {
            if (!data || size == 0 || dataSize < size || !signature) { return false; }
            ConstSpan<char> headerPtr = ConstSpan<char>(signature, size);
            ConstSpan<char> dataPtr   = ConstSpan<char>(reinterpret_cast<const char*>(data), size);

            for (size_t i = 0, j = 1; i < size; i++, j <<= 1) {
                if ((mask & j) && (headerPtr[i] != dataPtr[i])) {
                    return false;
                }
            }
            return true;
        }
    };

    enum DataFormatFlags : uint8_t {
        FMT_F_NONE = 0x0,
        FMT_F_RESET_POS = 0x1,

        FMT_F_ANALYZE_SHIFT = 6,
        FMT_F_ANALYZE_MASK  = 0xC0,
        FMT_F_ANALYSIS_SIMPLE  = 1 << FMT_F_ANALYZE_SHIFT,
        FMT_F_ANALYSIS_COMPLEX = 2 << FMT_F_ANALYZE_SHIFT,
    };

    namespace Format {
         const FormatInfo* getFormats();
         size_t getLargestFormat();

         DataFormat getFormat(const char* path, int64_t size = -1, DataFormatFlags flags = DataFormatFlags(0), std::function<void(void*, size_t)> dataTransform = nullptr);
         DataFormat getFormat(const Stream& stream, int64_t size = -1, DataFormatFlags flags = DataFormatFlags(0), std::function<void(void*, size_t)> dataTransform = nullptr);
         const char* getExtension(const char* path);
         const char* getExtension(const Stream& stream);
         const char* getExtension(DataFormat format);
    }

    template<>
    inline constexpr int32_t JCore::EnumNames<DataFormat, FMTN_NAME>::Count{ _FMT_COUNT };

    template<>
    inline const char** JCore::EnumNames<DataFormat, FMTN_NAME>::getEnumNames() {
        static const char* names[Count] =
        {
            "Binary",
            "Text",
            "JSON",
            "YAML",

            "",

            "PNG",
            "BMP",
            "DDS",
            "JPEG",
            "GIF87",
            "GIF89",
            "WEBP",
            "WAVE",
            "OGG",
            "MP3",
            "TTF",
            "OTF",
            "JTEX",
        };
        return names;
    }

    template<>
    inline bool EnumNames<DataFormat, FMTN_NAME>::noDraw(int32_t index) {
        static bool noDraw[Count] {
            false, false, true, true, true
        };

        auto names = getEnumNames();
        if (index < 0 || index >= Count || !names) { return true; }
        return noDraw[index] || isNoName(names[index]);
    }

    //Extensions
    template<>
    inline constexpr int32_t JCore::EnumNames<DataFormat, FMTN_EXTENSION>::Count{ _FMT_COUNT };

    template<>
    inline const char** JCore::EnumNames<DataFormat, FMTN_EXTENSION>::getEnumNames() {
        static const char* names[Count] =
        {
            ".bin",
            ".txt",
            ".json",
            ".yaml",

            "",

            ".png",
            ".bmp",
            ".dds",
            ".jpeg",
            ".gif",
            ".gif",
            ".webp",
            ".wav",
            ".ogg",
            ".mp3",
            ".ttf",
            ".otf",
            ".jtex",
        };
        return names;
    }

    template<>
    inline bool EnumNames<DataFormat, FMTN_EXTENSION>::noDraw(int32_t index) {
        static bool noDraw[Count]{
            false, false, true, true, true
        };

        auto names = getEnumNames();
        if (index < 0 || index >= Count || !names) { return true; }
        return noDraw[index] || isNoName(names[index]);
    }
}


