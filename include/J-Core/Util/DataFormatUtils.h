#pragma once
#include <cstdint>
#include <J-Core/IO/Stream.h>
#include <J-Core/Util/Span.h>
#include <bitset>
#include <functional>
#include <J-Core/Util/EnumUtils.h>

namespace JCore {
    enum DataFormat : uint8_t {
        FMT_BINARY,
        FMT_TEXT,

        FMT_JSON, //Requires complex analysis
        FMT_YAML, //Requires complex analysis

        _FMT_START,

        //Texture formats
        FMT_PNG,
        FMT_BMP,
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

        _FMT_COUNT,
    };

    struct FormatInfo {
        const char* signature{0};
        uint64_t mask = 0;
        size_t size = 0;

        FormatInfo() : signature{0}, mask(0), size(0) {}
        FormatInfo(const void* sig, uint64_t mask, size_t size) :
            signature{reinterpret_cast<const char*>(sig)}, mask(mask), size(size > 64 ? 64 : 0)
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

    const FormatInfo* getDataFormats();
    size_t getLargestFormat();
    DataFormat getDataFormat(const Stream& stream, int64_t size = -1, DataFormatFlags flags = DataFormatFlags(0), std::function<void(void*, size_t)> dataTransform = nullptr);

    template<>
    inline constexpr int32_t JCore::EnumNames<DataFormat>::Count{ _FMT_COUNT };

    template<>
    inline const char** JCore::EnumNames<DataFormat>::getEnumNames() {
        static const char* names[Count] =
        {
            "Binary",
            "Text",
            "JSON",
            "YAML",

            "",

            "PNG",
            "BMP",
            "JPEG",
            "GIF87",
            "GIF89",
            "WEBP",
            "WAVE",
            "OGG",
            "MP3",
            "TTF",
            "OTF",
        };
        return names;
    }

    template<>
    inline bool EnumNames<DataFormat>::noDraw(int32_t index) {
        static bool noDraw[Count] {
            false, false, true, true, true
        };

        auto names = getEnumNames();
        if (index < 0 || index >= Count || !names) { return true; }
        return noDraw[index] || isNoName(names[index]);
    }
}
