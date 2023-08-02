#pragma once
#include <cstdint>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/IO/FileStream.h>
#include <J-Core/Log.h>
#include <J-Core/IO/ImageUtils.h>
static constexpr uint8_t F_IMG_BUILD_PALETTE = 0x1;

namespace JCore {
    struct ImageDecodeParams {
        uint8_t flags{0};
    };

    namespace Png {
        bool decode(const char* path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
        
        bool getInfo(const char* path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool encode(const char* path, const ImageData& imgData, const uint32_t compression = 6);
        bool encode(const Stream& stream, const ImageData& imgData, const uint32_t compression = 6);
    }

    namespace Bmp {

        bool decode(const char* path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
        
        bool getInfo(const char* path, ImageData& imgData);
        bool getInfo(const Stream& stream, ImageData& imgData);

        bool encode(const char* path, const ImageData& imgData);
        bool encode(const Stream& stream, const ImageData& imgData);
    }

    namespace DXT {

        bool decodeDxt1(const char* path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decodeDxt1(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
                      
        bool encodeDxt1(const char* path, const ImageData& imgData);
        bool encodeDxt1(const Stream& stream, const ImageData& imgData);

        bool decodeDxt5(const char* path, ImageData& imgData, const ImageDecodeParams params = {});
        bool decodeDxt5(const Stream& stream, ImageData& imgData, const ImageDecodeParams params = {});
                   
        bool encodeDxt5(const char* path, const ImageData& imgData);
        bool encodeDxt5(const Stream& stream, const ImageData& imgData);
    }
}