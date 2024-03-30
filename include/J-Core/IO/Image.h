#pragma once
#include <cstdint>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/IO/FileStream.h>
#include <J-Core/Log.h>
#include <J-Core/IO/ImageUtils.h>
#include <J-Core/Util/DataFormatUtils.h>
static constexpr uint8_t F_IMG_BUILD_PALETTE = 0x1;

namespace JCore {
    struct ImageDecodeParams {
        uint8_t flags{0};
    };

    struct ImageEncodeParams {
        uint32_t dpi{ 96 };
        int32_t compression{ 6 };
    };

    namespace Png {
        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
        
        bool getInfo(std::string_view path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool encode(std::string_view path, const ImageData& imgData, const uint32_t compression = 6);
        bool encode(const Stream& stream, const ImageData& imgData, const uint32_t compression = 6);
    }

    namespace Bmp {
        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
        
        bool getInfo(std::string_view path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool encode(std::string_view path, const ImageData& imgData, uint32_t dpi = 96);
        bool encode(const Stream& stream, const ImageData& imgData, uint32_t dpi = 96);
    }

    namespace ICO {
        bool getInfo(std::string_view path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});

        bool encode(std::string_view path, const ImageData& imgData, bool compressed = false);
        bool encode(const Stream& stream, const ImageData& imgData, bool compressed = false);
    }

    namespace DXT {
        bool decodeDxt1(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decodeDxt1(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
                      
        bool encodeDxt1(std::string_view path, const ImageData& imgData);
        bool encodeDxt1(const Stream& stream, const ImageData& imgData);

        bool decodeDxt5(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decodeDxt5(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
                   
        bool encodeDxt5(std::string_view path, const ImageData& imgData);
        bool encodeDxt5(const Stream& stream, const ImageData& imgData);
    }

    namespace DDS {
        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});

        bool getInfo(std::string_view path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool encode(std::string_view path, const ImageData& imgData);
        bool encode(const Stream& stream, const ImageData& imgData);
    }

    namespace JTEX {
        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});

        bool getInfo(std::string_view path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool encode(std::string_view path, const ImageData& imgData);
        bool encode(const Stream& stream, const ImageData& imgData);
    }

    namespace Image {
        bool tryGetInfo(std::string_view path, ImageData& imgData, DataFormat& format);
        bool tryGetInfo(const Stream& stream, ImageData& imgData, DataFormat& format);

        bool tryDecode(std::string_view path, ImageData& imgData, DataFormat& format, const ImageDecodeParams params = {});
        bool tryDecode(const Stream& stream, ImageData& imgData, DataFormat& format, const ImageDecodeParams params = {});

        bool tryEncode(std::string_view path, const ImageData& imgData, DataFormat format, const ImageEncodeParams& encodeParams = {});
        bool tryEncode(const Stream& stream, const ImageData& imgData, DataFormat format, const ImageEncodeParams& encodeParams = {});
    }
}