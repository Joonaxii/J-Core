#include <J-Core/IO/Image.h>
#include <iostream>
#include <J-Core/Log.h>
#include <J-Core/Math/Color24.h>
#include <J-Core/Math/Color4444.h>
#include <J-Core/Math/Color32.h>
#include <J-Core/Math/Math.h>
#include <J-Core/Util/DataUtils.h>
#include <J-Core/Util/Span.h>
#include <J-Core/IO/FileStream.h>
#include <J-Core/IO/ZLib.h>
#include <J-Core/IO/MemoryStream.h>
#include <J-Core/Rendering/Texture.h>

namespace JCore {
    static inline constexpr int32_t getPaddedWidth(int32_t width, int32_t bpp) {
        return ((width * bpp + 31) / 32) * 4;
    }

    static inline constexpr int32_t calcualtePadding(int32_t width, int32_t bpp) {
        int32_t rem = (width * bpp) & 0x3;
        return rem ? 4 - rem : 0;
    }

    static void flipRB(uint8_t* data,  int32_t width, int32_t bpp) {
        switch (bpp)
        {
            default: return;
            case 3:
            case 4:
                for (size_t i = 0, j = 0, k = 2; i < width; i++, j += bpp, k += bpp) {
                    uint8_t temp = data[j];
                    data[j] = data[k];
                    data[k] = temp;
                }
                break;
        }
    }

    namespace Bmp {
#pragma pack(push, 1)
        struct BmpHeader {
            //File Header
            uint16_t signature;
            uint32_t length;
            char reserved[4];
            uint32_t dataOffset;

            //Info Header
            uint32_t headerSize;

            int32_t width;
            int32_t height;

            uint16_t planes;
            uint16_t bpp;

            uint32_t compression;
            uint32_t imageSize;

            uint32_t pPMX;
            uint32_t pPMY;

            int32_t colorsUsed;
            int32_t numOfColors;
        };
#pragma pack(pop, 1)
        bool getInfo(std::string_view path, ImageData& imgData) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return getInfo(stream, imgData);
            }

            JCORE_ERROR("[Image-IO] (BMP) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool getInfo(const Stream& stream, ImageData& imgData) {
            size_t start = stream.tell();
            size_t dataSize = stream.size() - start;

            if (dataSize < 54) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Not enough data to read header!");
                return false;
            }

            BmpHeader header;
            stream.read(&header, sizeof(header), false);

            if (header.signature != 0x4D42U) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Invalid BMP signature!");
                return false;
            }
            if (header.length > dataSize) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Not enough data in stream!");
                return false;
            }

            switch (header.bpp)
            {
                case 8:
                case 24:
                case 32:
                    break;
                default:
                    JCORE_ERROR("[Image-IO] (BMP) Decode Error: Unsupported bitdepth ({0})!", header.bpp);
                    return false;
            }
            if (header.compression != 0 && header.compression != 0x3) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Unsupported compression mode ({0})!", header.compression);
                return false;
            }

            if (header.width < 0) { header.width = -header.width; }
            if (header.height < 0) { header.height = -header.height; }

            imgData.width = header.width;
            imgData.height = header.height;

            switch (header.bpp)
            {
                case 8: {
                    imgData.paletteSize = 256;
                    imgData.format = TextureFormat::Indexed8;
                    break;
                }
                case 24: {
                    imgData.format = TextureFormat::RGB24;
                    break;
                }
                case 32: {
                    imgData.format = TextureFormat::RGBA32;
                    break;
                }
            }
            return true;
        }


        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return decode(stream, imgData, params);
            }

            JCORE_ERROR("[Image-IO] (BMP) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params) {
            size_t start = stream.tell();
            size_t dataSize = stream.size() - start;

            if (dataSize < 54) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Not enough data to read header!");
                return false;
            }

            BmpHeader header;
            stream.read(&header, sizeof(header), false);

            if (header.signature != 0x4D42U) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Invalid BMP signature!");
                return false;
            }
            if (header.length > dataSize) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Not enough data in stream!");
                return false;
            }

            switch (header.bpp)
            {
                case 8:
                case 24:
                case 32:
                    break;
                default:
                    JCORE_ERROR("[Image-IO] (BMP) Decode Error: Unsupported bitdepth ({0})!", header.bpp);
                    return false;
            }
            if (header.compression != 0 && header.compression != 0x3) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Unsupported compression mode ({0})!", header.compression);
                return false;
            }

            const bool flipX = header.width < 0;
            const bool flipY = header.height > 0;
            const uint32_t bytesPerPixel = header.bpp >> 3;

            if (header.width < 0) { header.width = -header.width; }
            if (header.height < 0) { header.height = -header.height; }

            const uint32_t reso = header.width * header.height;

            const int32_t padding = calcualtePadding(header.width, bytesPerPixel);

            const uint32_t rawScanSize = (header.width * bytesPerPixel);
            const uint32_t scanSize = rawScanSize + padding;
            char* scan = reinterpret_cast<char*>(_malloca(scanSize));

            if (!scan) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Failed to allocate scanline of size {0} bytes!", scanSize);
                return false;
            }

            const uint32_t paletteOff = header.bpp == 8 ? 256LL * 4 : 0;

            const uint32_t pixelDataSize = rawScanSize * header.height;
            const uint32_t outSize = pixelDataSize + paletteOff;

            if (!imgData.doAllocate(outSize)) {
                JCORE_ERROR("[Image-IO] (BMP) Decode Error: Failed to allocate pixel buffer of size {0} bytes!", scanSize);
                _freea(scan);
                return false;
            }

            imgData.width = header.width;
            imgData.height = header.height;

            //Take note of position before any potential extra data used for 
            //bit depts other than 24 bits.
            size_t pos = stream.tell();

            const size_t end = size_t(pixelDataSize - rawScanSize) + paletteOff;
            //Seek to start of data and read data to buffer
            stream.seek(start + std::streamoff(header.dataOffset), std::ios::beg);
            for (size_t i = 0, sP = flipY ? 0 : paletteOff; i < header.height; i++, sP += rawScanSize) {
                stream.read(scan, scanSize, false);
                memcpy(imgData.data + (flipY ? (end - sP) : sP), scan, rawScanSize);
            }

            stream.seek(pos, std::ios::beg);
            switch (header.bpp)
            {
                case 8: {
                    //Read palette, swap Red & Blue channels, and then set alpha to 255
                    stream.read(imgData.data, header.numOfColors * 4, false);
                    Color32* cPtr = reinterpret_cast<Color32*>(imgData.data);

                    for (size_t i = 0; i < header.numOfColors; i++) {
                        auto& color = cPtr[i];
                        std::swap(color.b, color.r);
                        color.a = 0xFF;
                    }
                    imgData.paletteSize = 256;
                    imgData.format = TextureFormat::Indexed8;
                    break;
                }
                case 24: {
                    //Swap Red & Blue channels
                    Color24* cPtr = reinterpret_cast<Color24*>(imgData.data);
                    for (size_t i = 0; i < reso; i++) {
                        auto& color = cPtr[i];
                        std::swap(color.b, color.r);
                    }
                    imgData.format = TextureFormat::RGB24;
                    break;
                }
                case 32: {
                    //Read color bit masks and calculate the bit offsets for them
                    uint32_t maskBuffer[4]{ 0 };

                    if (stream.tell() == header.dataOffset) {
                        maskBuffer[0] = 0x00FF0000U;
                        maskBuffer[1] = 0x0000FF00U;
                        maskBuffer[2] = 0x000000FFU;
                        maskBuffer[3] = 0xFF000000U;
                    }
                    else {
                        stream.read(reinterpret_cast<char*>(maskBuffer), 16, false);
                    }

                    const int32_t maskOffsets[4]{
                        Math::findFirstLSB(maskBuffer[0]),
                        Math::findFirstLSB(maskBuffer[1]),
                        Math::findFirstLSB(maskBuffer[2]),
                        Math::findFirstLSB(maskBuffer[3]),
                    };

                    //Apply color masks
                    uint32_t* iPtr = reinterpret_cast<uint32_t*>(imgData.data);
                    Color32* cPtr = reinterpret_cast<Color32*>(imgData.data);
                    for (size_t i = 0, j = 0; i < reso; i++, j++) {
                        auto& color = cPtr[i];
                        const uint32_t data = uint32_t(color);

                        color.r = (data & maskBuffer[0]) >> maskOffsets[0];
                        color.g = (data & maskBuffer[1]) >> maskOffsets[1];
                        color.b = (data & maskBuffer[2]) >> maskOffsets[2];
                        color.a = (data & maskBuffer[3]) >> maskOffsets[3];
                    }
                    imgData.format = TextureFormat::RGBA32;
                    break;
                }
            }
            return true;
        }

        bool encode(std::string_view path, const ImageData& imgData, uint32_t dpi) {
            FileStream fs(path);
            if (fs.open("wb")) {
                return encode(fs, imgData, dpi);
            }
            JCORE_ERROR("[Image-IO] (BMP) Encode Error: Failed to open file '{0}' for writing!", path);
            return false;
        }

        bool encode(const Stream& stream, const ImageData& imgData, uint32_t dpi) {

            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (BMP) Encode Error: Stream isn't open!");
                return false;
            }

            static constexpr const char* BMP_SIG = "BM";
            static constexpr uint32_t HEADER_SIZE = 54;

            if (imgData.format == TextureFormat::Unknown) {
                JCORE_ERROR("[Image-IO] (BMP) Encode Error: Unknown texture format!");
                return false;
            }

            uint8_t bpp = getBitsPerPixel(imgData.format) >> 3;

            int32_t scanSR = imgData.width * bpp;
            int32_t padding = calcualtePadding(imgData.width, 3);
            int32_t scanSP = scanSR + padding;

            int32_t extra = 0;
            switch (imgData.format)
            {
                case TextureFormat::Indexed8:
                    extra = 1024;
                    break;
                case TextureFormat::RGBA32:
                    extra = 16;
                    break;
            }

            int32_t dataOffset = HEADER_SIZE + extra;
            int32_t total = (dataOffset + (scanSP * imgData.height));

            uint8_t* scan = reinterpret_cast<uint8_t*>(_malloca(scanSP));
            if (!scan) {
                JCORE_ERROR("[Image-IO] (BMP) Encode Error: Failed to allocate scanline buffer of size {0} bytes!", scanSP);
                return false;
            }
            memset(scan, 0, scanSP);

            //File Header
            stream.write(BMP_SIG, 2);
            stream.writeValue<uint32_t>(total, 1);
            stream.writeZero(4);
            stream.writeValue(dataOffset);

            //Info Header
            stream.writeValue(40);

            stream.write(&imgData.width, 4);
            stream.write(&imgData.height, 4);
            stream.writeValue<uint16_t>(1);

            stream.writeValue<uint16_t>(bpp << 3);

            switch (imgData.format)
            {
                default:
                    stream.writeValue<int32_t>(0, 1);
                    stream.writeValue<int32_t>(0, 1);
                    break;
                case TextureFormat::RGBA32:
                    stream.writeValue<int32_t>(3, 1);
                    stream.writeValue<int32_t>(scanSP * imgData.height, 1);
                    break;
            }

            stream.writeValue<uint32_t>(uint32_t(std::round(dpi * 39.3701f)), 2);

            if (imgData.format == TextureFormat::Indexed8) {
                stream.writeValue<int32_t>(256);
                stream.writeZero(4);
            }
            else {
                stream.writeZero(8);
            }

            switch (imgData.format)
            {
                default:
                    break;
                case TextureFormat::RGBA32:
                    stream.writeValue<uint32_t>(0x00FF0000U, 1);
                    stream.writeValue<uint32_t>(0x0000FF00U, 1);
                    stream.writeValue<uint32_t>(0x000000FFU, 1);
                    stream.writeValue<uint32_t>(0xFF000000U, 1);
                    break;
                case TextureFormat::Indexed8: {
                    Color32 palette[256]{};
                    memcpy(palette, imgData.data, 1024);
                    for (size_t i = 0; i < 256; i++) {
                        palette[i].flipRB();
                        palette[i].a = 0x00;
                    }
                    stream.write(palette, sizeof(palette));
                }
                                            break;
            }

            for (size_t y = 0, yP = size_t(imgData.height) * scanSR - size_t(scanSR); y < imgData.height; y++, yP -= scanSR) {
                memcpy(scan, imgData.data + yP, scanSR);
                flipRB(scan, imgData.width, bpp);
                stream.write(scan, scanSP);
            }
            return true;
        }
    }

    namespace Png {
        enum PngChunkType : uint32_t {
            CH_IHDR = 0x52444849U,
            CH_PLTE = 0x45544C50U,
            CH_tRNS = 0x534E5274U,
            CH_IDAT = 0x54414449U,
            CH_IEND = 0x444E4549U,
        };

#pragma pack(push, 1)
        struct PngChunk {
            uint32_t length{ 0 };
            PngChunkType type{};
            size_t position{ 0 };
            uint32_t crc{ 0 };

            PngChunk() : length(), type(), position(), crc() {}
            PngChunk(const uint32_t length, const PngChunkType type) : length(length), type(type), position(), crc() {}

            void set(const uint32_t length, const PngChunkType type) {
                this->length = length;
                this->type = type;
            }
        };

        struct IHDRChunk {
            int32_t width;
            int32_t height;
            uint8_t bitDepth;
            uint8_t colorType;
            uint8_t compression;
            uint8_t filter;
            uint8_t interlaced;
        };

#pragma pack(pop, 1)

        static bool calculateDiff(const uint8_t* data, size_t width, size_t bpp, uint64_t& current) {
            uint64_t curVal = 0;
            uint64_t buf = 0;
            for (size_t i = 0, j = 0; i < width; i++, j += bpp) {
                memcpy(&buf, data + j, bpp);
                curVal += buf;
                if (curVal >= current) { return false; }
            }
            current = curVal;
            return true;
        }

        static int32_t paethPredictor(int32_t a, int32_t b, int32_t c) {
            int32_t p = a + b - c;
            int32_t pA = std::abs(p - a);
            int32_t pB = std::abs(p - b);
            int32_t pC = std::abs(p - c);

            if (pA <= pB && pA <= pC) { return a; }
            return pB <= pC ? b : c;
        }

        static void applyFilter(Span<uint8_t>& current, Span<uint8_t>& prior, Span<uint8_t>& target, int32_t width, int32_t bpp, uint8_t filter) {
            width *= bpp;
            switch (filter)
            {
                case 1: //Sub
                    memcpy(target.get(), current.get(), bpp);
                    for (int32_t x = bpp, xS = 0; x < width; x++, xS++) {
                        target[x] = uint8_t(int32_t(current[x]) - current[xS]);
                    }
                    break;
                case 2: //Up
                    for (int32_t x = 0; x < width; x++) {
                        target[x] = uint8_t(int32_t(current[x]) - prior[x]);
                    }
                    break;
                case 3: //Average
                    for (int32_t x = 0, xS = -bpp; x < width; x++, xS++) {
                        target[x] = uint8_t(int32_t(current[x]) - ((int32_t(prior[x]) + (xS < 0 ? 0 : current[xS])) >> 1));
                    }
                    break;
                case 4: //Paeth
                    for (int32_t x = 0, xS = -bpp; x < width; x++, xS++) {
                        int32_t a = (xS < 0 ? 0 : current[xS]);
                        int32_t b = prior[x];
                        int32_t c = (xS < 0 ? 0 : prior[xS]);
                        target[x] = uint8_t(int32_t(current[x]) - paethPredictor(a, b, c));
                    }
                    break;
            }
        }

        static void reverseFilter(Span<uint8_t>& current, Span<uint8_t>& prior, int32_t width, int32_t bpp, uint8_t filter) {
            width *= bpp;
            switch (filter)
            {
                case 1: //Sub
                    for (int32_t x = bpp, xS = 0; x < width; x++, xS++) {
                        current[x] = uint8_t(current[x] + current[xS]);
                    }
                    break;
                case 2: //Up
                    for (int32_t x = 0; x < width; x++) {
                        current[x] = uint8_t(current[x] + prior[x]);
                    }
                    break;
                case 3: //Average
                    for (int32_t x = 0, xS = -bpp; x < width; x++, xS++) {
                        current[x] = uint8_t(current[x] + ((prior[x] + (xS < 0 ? 0 : current[xS])) >> 1));
                    }
                    break;
                case 4: //Paeth
                    for (int32_t x = 0, xS = -bpp; x < width; x++, xS++) {
                        int32_t a = (xS < 0 ? 0 : current[xS]);
                        int32_t b = prior[x];
                        int32_t c = (xS < 0 ? 0 : prior[xS]);
                        current[x] = uint8_t(current[x] + paethPredictor(a, b, c));
                    }
                    break;
            }
        }

        static bool readChunk(const Stream& stream, PngChunk& chunk) {
            stream.readValue<uint32_t>(&chunk.length, 1, true);
            stream.readValue<PngChunkType>(&chunk.type, 1, false);
            chunk.position = stream.tell();
            stream.seek(chunk.length, SEEK_CUR);
            stream.readValue<uint32_t>(&chunk.crc, 1, true);
            stream.seek(-int64_t(chunk.length + 4), SEEK_CUR);

            //TODO: Maybe adding some validation here to return true or false?
            return true;
        }

        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream stream(path, "rb");
            if (stream.isOpen()) {
                return decode(stream, imgData, params);
            }

            JCORE_ERROR("[Image-IO] (PNG) Decode Error: Failed to open '{0}'!", path);
            return false;
        }

        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params) {
            static uint8_t PNG_HEADER[]{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, };
            uint8_t buffer[32]{ 0 };

            stream.read(buffer, 8, false);
            if (std::memcmp(PNG_HEADER, buffer, 8)) {
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Invalid PNG signature!");
                return false;
            }

            std::vector<PngChunk> idats{};
            PngChunk paletteChnk{};
            PngChunk alphaChnk{};

            size_t totalIdat = 0;

            IHDRChunk ihdr{};
            PngChunk chunk{};
            while (!stream.isEOF()) {
                if (!readChunk(stream, chunk)) { break; }
                switch (chunk.type)
                {
                    case CH_PLTE:
                        paletteChnk = chunk;
                        stream.seek(chunk.length + 4, SEEK_CUR);
                        break;
                    case CH_tRNS:
                        alphaChnk = chunk;
                        stream.seek(chunk.length + 4, SEEK_CUR);
                        break;

                    default:
                        stream.seek(chunk.length + 4, SEEK_CUR);
                        break;

                    case CH_IHDR:
                        stream.read(&ihdr, sizeof(IHDRChunk), false);
                        Data::reverseEndianess(&ihdr, sizeof(uint32_t), 2);
                        stream.seek(4, SEEK_CUR);

                        /*  if (ihdr.interlaced) {
                              JCORE_ERROR("[Image-IO] (PNG) Decode Error: Interlaced PNGs are note supported!");
                              return false;
                          }*/

                        switch (ihdr.bitDepth)
                        {
                            default:
                                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Unsupported bitdepth {0}!", ihdr.bitDepth);
                                return false;
                            case 8:
                            case 16:
                                break;
                        }

                        switch (ihdr.colorType)
                        {
                            default:
                                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Unsupported color type {0}!", ihdr.colorType);
                                return false;
                            case 0:
                                if (ihdr.bitDepth != 8) {
                                    JCORE_ERROR("[Image-IO] (PNG) Decode Error: Unsupported bitdepth {0}!", ihdr.bitDepth);
                                    return false;
                                }
                                imgData.format = TextureFormat::R8;
                                break;
                            case 2:
                                imgData.format = ihdr.bitDepth == 8 ? TextureFormat::RGB24 : TextureFormat::RGB48;
                                break;
                            case 6:
                                imgData.format = ihdr.bitDepth == 8 ? TextureFormat::RGBA32 : TextureFormat::RGBA64;
                                break;
                            case 3:
                                imgData.format = TextureFormat::Indexed8;
                                break;
                        }

                        imgData.width = std::abs(ihdr.width);
                        imgData.height = std::abs(ihdr.height);
                        break;

                    case CH_IDAT:
                        totalIdat += chunk.length;
                        idats.emplace_back(chunk);
                        stream.seek(chunk.length + 4, SEEK_CUR);
                        break;

                    case CH_IEND: //If we hit an IEND chunk, break out of the loop
                        stream.seek(4, SEEK_CUR);
                        goto end;
                }
            }
        end:

            uint8_t bpp = getBitsPerPixel(imgData.format) >> 3;
            uint32_t scanSR = imgData.width * bpp;
            uint32_t scanSP = scanSR + 1;

            uint32_t paletteSize = imgData.format == TextureFormat::Indexed8 ? 256 * 4 : (params.flags & F_IMG_BUILD_PALETTE) ? 256 * 256 * 4 : 0;
            uint32_t totalSize = scanSR * imgData.height + paletteSize;
            uint32_t rawSize = scanSP * imgData.height;

            if (!imgData.doAllocate(totalSize)) {
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Failed to allocate pixel buffer! ({0} bytes)", totalSize);
                return false;
            }

            uint8_t* rawBuffer = reinterpret_cast<uint8_t*>(_malloca(rawSize));
            if (!rawBuffer) {
                free(imgData.data);
                imgData.data = nullptr;
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Failed to allocate decompress buffer! ({0} bytes)", rawSize);
                return false;
            }

            uint8_t* compBuffer = reinterpret_cast<uint8_t*>(_malloca(totalIdat));
            if (!compBuffer) {
                _freea(rawBuffer);
                free(imgData.data);
                imgData.data = nullptr;
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Failed to allocate IDAT buffer! ({0} bytes)", totalIdat);
                return false;
            }

            size_t pos = 0;
            for (const auto& ch : idats) {
                stream.seek(ch.position, SEEK_SET);
                stream.read(compBuffer + pos, ch.length, false);
                pos += ch.length;
            }
            int32_t ret = ZLib::inflateData(compBuffer, totalIdat, rawBuffer, rawSize);
            _freea(compBuffer);

            if (ret == -1) {
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: ZLib Inflate failed!");
                _freea(rawBuffer);

                free(imgData.data);
                imgData.data = nullptr;
                return false;
            }

            uint8_t* scanBuffer = reinterpret_cast<uint8_t*>(_malloca(scanSP * 2));
            if (!scanBuffer) {
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Failed to allocate scan buffer!");
                _freea(rawBuffer);

                free(imgData.data);
                imgData.data = nullptr;
                return false;
            }
            memset(scanBuffer, 0, scanSP * 2);

            if (imgData.format == TextureFormat::Indexed8) {
                if (paletteChnk.type == CH_PLTE) {
                    stream.seek(paletteChnk.position, SEEK_SET);
                    size_t count = paletteChnk.length / 3;

                    Color24 palette[256]{};
                    stream.read(palette, sizeof(Color24), count, false);

                    for (size_t i = 0, j = 0; i < count; i++, j += 4) {
                        memcpy(imgData.data + j, &palette[i], 3);
                        imgData.data[j + 3] = 0xFF;
                    }
                }

                if (alphaChnk.type == CH_tRNS) {
                    stream.seek(alphaChnk.position, SEEK_SET);

                    uint8_t alpha[256]{ 0 };
                    stream.read(alpha, alphaChnk.length, false);

                    for (size_t i = 0, j = 3; i < alphaChnk.length; i++, j += 4) {
                        imgData.data[j] = alpha[i];
                    }
                }
            }

            Span<uint8_t> prior(scanBuffer, scanSP);
            Span<uint8_t> current(scanBuffer + scanSP, scanSP);

            Span<uint8_t> priorPix = prior.slice(1);
            Span<uint8_t> currentPix = current.slice(1);

            TextureFormat fmt = TextureFormat::Indexed8;
            size_t posR = paletteSize;
            int32_t colorCount = 0;
            bool canBuild = bpp > 1;
            size_t bytesPC = ihdr.bitDepth >> 3;
            size_t channelsW = (bpp / bytesPC) * imgData.width;
            for (size_t y = 0, xP = 0, xPS = 0; y < imgData.height; y++, xP += scanSP) {
                memcpy(current.get(), rawBuffer + xP, scanSP);
                Data::reverseEndianess(currentPix.get(), bytesPC, channelsW);

                reverseFilter(currentPix, priorPix, imgData.width, bpp, current[0]);

                auto pixTgt = imgData.data + posR;
                currentPix.copyTo(pixTgt);
                posR += scanSR;

                if (canBuild && imgData.format != TextureFormat::Indexed8 && (params.flags & F_IMG_BUILD_PALETTE)) {
                    if (!tryBuildPalette(pixTgt, 0, scanSR, colorCount, fmt, bpp, imgData.data, -1)) {
                        canBuild = false;
                    }
                }
                current.copyTo(prior);
            }

            if (colorCount > 0 && canBuild) {
                colorCount = fmt == TextureFormat::Indexed8 ? 256 : Math::alignToPalette(colorCount);

                JCORE_TRACE("[Image-IO] (PNG) Decode: Building palette with {0} colors!", colorCount);
                totalSize = colorCount * 4 + (imgData.width * imgData.height * 2);
                uint8_t* temp = reinterpret_cast<uint8_t*>(malloc(totalSize));

                if (!temp) {
                    JCORE_ERROR("[Image-IO] (PNG) Decode Error: Failed to allocate Indexed pixel buffer! ({0} bytes)", totalSize);
                    return false;
                }
                applyPalette(imgData.data, imgData.width, imgData.height, colorCount, imgData.format, temp, fmt, -1);

                free(imgData.data);
                imgData.data = temp;
            }

            _freea(scanBuffer);
            _freea(rawBuffer);
            return true;
        }

        bool getInfo(std::string_view path, ImageData& imgData) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return getInfo(stream, imgData);
            }

            JCORE_ERROR("[Image-IO] (PNG) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool getInfo(const Stream& stream, ImageData& imgData) {
            static uint8_t PNG_HEADER[]{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, };
            uint8_t buffer[32]{ 0 };

            stream.read(buffer, 8, false);
            if (std::memcmp(PNG_HEADER, buffer, 8)) {
                JCORE_ERROR("[Image-IO] (PNG) Decode Error: Invalid PNG signature!");
                return false;
            }

            PngChunk chunk{};
            IHDRChunk ihdr{};
            while (readChunk(stream, chunk)) {
                if (chunk.type == PngChunkType::CH_IHDR) {
                    stream.read(&ihdr, sizeof(IHDRChunk), false);
                    Data::reverseEndianess(&ihdr, sizeof(uint32_t), 2);
                    stream.seek(4, SEEK_CUR);
                    goto foundHeader;
                }
            }

            JCORE_ERROR("[Image-IO] (PNG) Decode Error: Couldn't find IHDR!");
            return false;

        foundHeader:
            switch (ihdr.bitDepth)
            {
                default:
                    JCORE_ERROR("[Image-IO] (PNG) Decode Error: Unsupported bitdepth {0}!", ihdr.bitDepth);
                    return false;
                case 8:
                case 16:
                    break;
            }

            switch (ihdr.colorType)
            {
                default:
                    JCORE_ERROR("[Image-IO] (PNG) Decode Error: Unsupported color type {0}!", ihdr.colorType);
                    return false;
                case 0:
                    if (ihdr.bitDepth != 8) {
                        JCORE_ERROR("[Image-IO] (PNG) Decode Error: Unsupported bitdepth {0}!", ihdr.bitDepth);
                        return false;
                    }
                    imgData.format = TextureFormat::R8;
                    break;
                case 2:
                    imgData.format = ihdr.bitDepth == 8 ? TextureFormat::RGB24 : TextureFormat::RGB48;
                    break;
                case 6:
                    imgData.format = ihdr.bitDepth == 8 ? TextureFormat::RGBA32 : TextureFormat::RGBA64;
                    break;
                case 3:
                    imgData.format = TextureFormat::Indexed8;
                    break;
            }
            imgData.width = ihdr.width;
            imgData.height = ihdr.height;

            return true;
        }

        static void writeChunk(const Stream& stream, const PngChunk& chunk, const uint8_t* data) {
            stream.writeValue(chunk.length, 1, true);
            stream.writeValue(chunk.type);
            stream.write(data, chunk.length);

            uint32_t crc = Data::updateCRC(0xFFFFFFFFU, &chunk.type, sizeof(PngChunkType));
            crc = Data::updateCRC(crc, data, chunk.length) ^ 0xFFFFFFFFU;
            stream.writeValue(crc, 1, true);
        }

        bool encode(std::string_view path, const ImageData& imgData, const uint32_t compression) {
            FileStream fs(path);
            if (fs.open("wb")) {
                return encode(fs, imgData, compression);
            }
            JCORE_ERROR("[Image-IO] (PNG) Encode Error: Failed to open file '{0}' for writing!", path);
            return false;
        }

        bool encode(const Stream& stream, const ImageData& imgData, const uint32_t compression) {
            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (PNG) Encode Error: Stream isn't open!");
                return false;
            }

            if (!imgData.data) {
                JCORE_ERROR("[Image-IO] (PNG) Encode Error: Given pixel array is null!");
                return false;
            }

            switch (imgData.format)
            {
                default:
                    JCORE_ERROR("[Image-IO] (PNG) Encode Error: Encoding for format '{0}' isn't supported!", getTextureFormatName(imgData.format));
                    return false;
                case TextureFormat::R8:
                case TextureFormat::Indexed8:
                case TextureFormat::Indexed16:
                case TextureFormat::RGB24:
                case TextureFormat::RGBA32:
                    break;
            }

            if (imgData.width < 1 || imgData.height < 1) {
                JCORE_ERROR("[Image-IO] (PNG) Encode Error: Invalid resolution ({0}x{1}) for encoding!", imgData.width, imgData.height);
                return false;
            }

            static uint8_t PNG_HEADER[]{ 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, };
            uint8_t buffer[32]{ 0 };
            Span<uint8_t> bufSpan(buffer, 32);
            PngChunk chunk;

            stream.write(PNG_HEADER, sizeof(PNG_HEADER));

            //IHDR
            bufSpan.writeAt<uint32_t>(0, imgData.width, true);
            bufSpan.writeAt<uint32_t>(4, imgData.height, true);

            TextureFormat fmt = imgData.format;
            switch (imgData.format)
            {
                default:
                    bufSpan.writeAt<uint8_t>(8, 8);
                    bufSpan.writeAt<uint8_t>(9, 0);
                    break;
                case TextureFormat::Indexed16:
                    if (hasAlpha(imgData.data, imgData.width, imgData.height, imgData.format, imgData.paletteSize)) {
                        fmt = TextureFormat::RGBA32;
                        goto rgbaSet;
                    }
                    fmt = TextureFormat::RGB24;
                    goto rgbSet;
                    break;
                case TextureFormat::RGB24:
                rgbSet:
                    bufSpan.writeAt<uint8_t>(8, 8);
                    bufSpan.writeAt<uint8_t>(9, 2);
                    break;
                case TextureFormat::RGBA32:
                rgbaSet:
                    bufSpan.writeAt<uint8_t>(8, 8);
                    bufSpan.writeAt<uint8_t>(9, 6);
                    break;
                case TextureFormat::Indexed8:
                    bufSpan.writeAt<uint8_t>(8, 8);
                    bufSpan.writeAt<uint8_t>(9, 3);
                    break;
            }
            bufSpan.writeValuesAt<uint8_t>(10, 0, 3, false);

            chunk.type = CH_IHDR;
            chunk.length = 13;
            writeChunk(stream, chunk, bufSpan.get());

            if (imgData.format == TextureFormat::Indexed8) {
                //Main palette, always 256
                Color24 palette[256]{};

                for (size_t i = 0, j = 0, k = 0; i < 256; i++, j += 3, k += 4) {
                    memcpy(palette + i, imgData.data + k, 3);
                }

                chunk.type = CH_PLTE;
                chunk.length = sizeof(palette);
                writeChunk(stream, chunk, reinterpret_cast<uint8_t*>(palette));

                //Transparency
                uint8_t* alpha = reinterpret_cast<uint8_t*>(palette);
                for (size_t i = 0, j = 3; i < 256; i++, j += 4) {
                    alpha[i] = imgData.data[j];
                }
                chunk.type = CH_tRNS;
                chunk.length = 256;
                writeChunk(stream, chunk, alpha);
            }

            int32_t bpp = getBitsPerPixel(fmt) >> 3;
            uint32_t scanSR = imgData.width * bpp;
            uint32_t scanSP = scanSR + 1;
            uint8_t* scanBuffer = reinterpret_cast<uint8_t*>(malloc(scanSP * 6));
            if (!scanBuffer) {
                JCORE_ERROR("[Image-IO] (PNG) Encode Error: Couldn't allocate scan/filter buffer! ({0} bytes)", scanSP * 6);
                return false;
            }

            size_t cmpBufSize = scanSP * (imgData.height + 2) + 4;
            uint8_t* compressBuf = reinterpret_cast<uint8_t*>(malloc(cmpBufSize));
            Span<uint8_t> compBuf(compressBuf, cmpBufSize);
            if (!compressBuf) {
                free(scanBuffer);
                JCORE_ERROR("[Image-IO] (PNG) Encode Error: Couldn't allocate compression buffer! ({0} bytes)", cmpBufSize);
                return false;
            }

            bool indexed = imgData.format == TextureFormat::Indexed8 || imgData.format == TextureFormat::Indexed16;
            MemoryStream compStrm(compressBuf, 0, cmpBufSize);
            uint8_t* pixData = imgData.data + (indexed ? imgData.paletteSize * 4 : 0);

            memset(scanBuffer, 0, scanSP * 6);
            Span<uint8_t> prior(scanBuffer, scanSP);
            Span<uint8_t> current(scanBuffer + scanSP, scanSP);

            Span<uint8_t> priorPix = prior.slice(1);
            Span<uint8_t> currentPix = current.slice(1);

            Span<uint8_t> filters[4]
            {
                Span<uint8_t>(scanBuffer + scanSP * 2 , scanSR),
                Span<uint8_t>(scanBuffer + scanSP * 2 + scanSR, scanSR),
                Span<uint8_t>(scanBuffer + scanSP * 2 + scanSR * 2, scanSR),
                Span<uint8_t>(scanBuffer + scanSP * 2 + scanSR * 3, scanSR),
            };

            static constexpr size_t ZLIB_BUFFER_SIZE = 8192 << 3;
            static uint8_t BUFFER[ZLIB_BUFFER_SIZE]{};
            ZLib::ZLibContext context{};
            ZLib::deflateBegin(context, compression, BUFFER, ZLIB_BUFFER_SIZE);

            uint64_t score = 0;
            uint8_t filter = 0;
            int32_t dataOut = 0;
            current[0] = 0;

            uint32_t compBufPos = 0;

            int32_t bppR = getBitsPerPixel(imgData.format) >> 3;
            for (int32_t y = 0, yP = 0, yPP = 0; y < imgData.height; y++, yP += scanSR, yPP += bppR) {
                if (imgData.format == TextureFormat::Indexed16) {
                    currentPix.write<uint8_t>(pixData + yPP, scanSR);
                }
                else {
                    currentPix.write<uint8_t>(pixData + yP, scanSR);
                }

                if (imgData.format != TextureFormat::Indexed8) {
                    score = UINT64_MAX;
                    filter = 0;
                    calculateDiff(currentPix.get(), imgData.width, bpp, score);

                    Span<uint8_t> pixScan(pixData + yP, scanSR);
                    for (uint8_t i = 0; i < 4; i++) {
                        applyFilter(pixScan, priorPix, filters[i], imgData.width, bpp, i + 1);
                        if (calculateDiff(filters[i].get(), imgData.width, bpp, score)) {
                            filter = uint8_t(i + 1);
                        }
                    }

                    if (filter > 0) {
                        filters[filter - 1].copyTo(currentPix.get());
                    }

                    current[0] = filter;
                    priorPix.write<uint8_t>(pixData + yP, scanSR);
                }

                ZLib::deflateSegment(context, current.get(), current.length(), compStrm, BUFFER, ZLIB_BUFFER_SIZE);
            }
            ZLib::deflateEnd(context, dataOut, compStrm, BUFFER, ZLIB_BUFFER_SIZE);

            chunk.type = CH_IDAT;
            chunk.length = uint32_t(compStrm.tell());
            writeChunk(stream, chunk, compressBuf);

            chunk.type = CH_IEND;
            chunk.length = 0;
            writeChunk(stream, chunk, nullptr);

            free(scanBuffer);
            free(compressBuf);
            return true;
        }
    }

    namespace DXT {
        uint32_t packRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
            return ((r << 0) | (g << 8) | (b << 16) | (a << 24));
        }

        void decompressDXT1Block(int32_t x, int32_t y, int32_t width, int32_t height, const uint8_t* blockStorage, Color24* image) {
            uint16_t color0 = *reinterpret_cast<const uint16_t*>(blockStorage);
            uint16_t color1 = *reinterpret_cast<const uint16_t*>(blockStorage + 2);

            uint32_t temp{};

            temp = (color0 >> 11) * 255 + 16;
            uint16_t r0 = uint16_t((temp / 32 + temp) / 32);
            temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
            uint16_t g0 = uint16_t((temp / 64 + temp) / 64);
            temp = (color0 & 0x001F) * 255 + 16;
            uint16_t b0 = uint16_t((temp / 32 + temp) / 32);

            temp = (color1 >> 11) * 255 + 16;
            uint16_t r1 = uint16_t((temp / 32 + temp) / 32);
            temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
            uint16_t g1 = uint16_t((temp / 64 + temp) / 64);
            temp = (color1 & 0x001F) * 255 + 16;
            uint16_t b1 = uint16_t((temp / 32 + temp) / 32);

            uint32_t code = *reinterpret_cast<const uint32_t*>(blockStorage + 4);

            for (int32_t j = 0; j < 4; j++)
            {
                for (int32_t i = 0; i < 4; i++)
                {
                    Color24 finalColor{};
                    uint8_t positionCode = (code >> 2 * (4 * j + i)) & 0x03;

                    if (color0 > color1)
                    {
                        switch (positionCode)
                        {
                            case 0:
                                finalColor = Color24(uint8_t(r0), uint8_t(g0), uint8_t(b0));
                                break;
                            case 1:
                                finalColor = Color24(uint8_t(r1), uint8_t(g1), uint8_t(b1));
                                break;
                            case 2:
                                finalColor = Color24(uint8_t((2 * r0 + r1) / 3), uint8_t((2 * g0 + g1) / 3), uint8_t((2 * b0 + b1) / 3));
                                break;
                            case 3:
                                finalColor = Color24(uint8_t((r0 + 2 * r1) / 3), uint8_t((g0 + 2 * g1) / 3), uint8_t((b0 + 2 * b1) / 3));
                                break;
                        }
                    }
                    else
                    {
                        switch (positionCode)
                        {
                            case 0:
                                finalColor = Color24(uint8_t(r0), uint8_t(g0), uint8_t(b0));
                                break;
                            case 1:
                                finalColor = Color24(uint8_t(r1), uint8_t(g1), uint8_t(b1));
                                break;
                            case 2:
                                finalColor = Color24(uint8_t((r0 + r1) / 2), uint8_t((g0 + g1) / 2), uint8_t((b0 + b1) / 2));
                                break;
                            case 3:
                                finalColor = Color24(0, 0, 0);
                                break;
                        }
                    }

                    if (x + i < width) {
                        image[(y + j) * width + (x + i)] = finalColor;
                    }
                }
            }
        }


        bool decodeDxt1(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream fs(path);
            if (fs.open("rb")) {
                return decodeDxt1(fs, imgData);
            }
            JCORE_ERROR("[Image-IO] (DXT1) Decode Error: Failed to open file '{0}' for reading!", path);
            return false;
        }

        bool decodeDxt1(const Stream& stream, ImageData& img, const ImageDecodeParams params) {
            uint32_t blockCountX = (img.width + 3) >> 2;
            uint32_t blockCountY = (img.height + 3) >> 2;
            uint32_t blockWidth = (img.width < 4) ? img.width : 4;
            uint32_t blockHeight = (img.height < 4) ? img.height : 4;

            size_t bufSize = blockCountX * 8;
            uint8_t* blockStorage = reinterpret_cast<uint8_t*>(_malloca(bufSize));
            Color24* pixels = reinterpret_cast<Color24*>(img.data);

            if (blockStorage) {
                for (uint32_t j = 0; j < blockCountY; j++)
                {
                    stream.read(blockStorage, 1, bufSize, false);
                    for (uint32_t i = 0, k = 0; i < blockCountX; i++, k += 8) {
                        decompressDXT1Block(i * 4, j * 4, img.width, img.height, blockStorage + k, pixels);
                    }
                }
                _freea(blockStorage);
                return true;
            }
            return false;
        }


        void decompressDXT5Block(int32_t x, int32_t y, int32_t width, int32_t height, const uint8_t* blockStorage, Color32* image) {
            uint8_t alpha0 = *reinterpret_cast<const uint8_t*>(blockStorage);
            uint8_t alpha1 = *reinterpret_cast<const uint8_t*>(blockStorage + 1);

            const uint8_t* bits = blockStorage + 2;
            uint32_t alphaCode1 = bits[2] | (bits[3] << 8) | (bits[4] << 16) | (bits[5] << 24);
            uint16_t alphaCode2 = bits[0] | (bits[1] << 8);

            uint16_t color0 = *reinterpret_cast<const uint16_t*>(blockStorage + 8);
            uint16_t color1 = *reinterpret_cast<const uint16_t*>(blockStorage + 10);

            uint32_t temp;

            temp = (color0 >> 11) * 255 + 16;
            uint8_t r0 = (uint8_t)((temp / 32 + temp) / 32);
            temp = ((color0 & 0x07E0) >> 5) * 255 + 32;
            uint8_t g0 = (uint8_t)((temp / 64 + temp) / 64);
            temp = (color0 & 0x001F) * 255 + 16;
            uint8_t b0 = (uint8_t)((temp / 32 + temp) / 32);

            temp = (color1 >> 11) * 255 + 16;
            uint8_t r1 = (uint8_t)((temp / 32 + temp) / 32);
            temp = ((color1 & 0x07E0) >> 5) * 255 + 32;
            uint8_t g1 = (uint8_t)((temp / 64 + temp) / 64);
            temp = (color1 & 0x001F) * 255 + 16;
            uint8_t b1 = (uint8_t)((temp / 32 + temp) / 32);

            uint32_t code = *reinterpret_cast<const uint32_t*>(blockStorage + 12);

            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    int alphaCodeIndex = 3 * (4 * j + i);
                    int alphaCode;

                    if (alphaCodeIndex <= 12)
                    {
                        alphaCode = (alphaCode2 >> alphaCodeIndex) & 0x07;
                    }
                    else if (alphaCodeIndex == 15)
                    {
                        alphaCode = (alphaCode2 >> 15) | ((alphaCode1 << 1) & 0x06);
                    }
                    else // alphaCodeIndex >= 18 && alphaCodeIndex <= 45
                    {
                        alphaCode = (alphaCode1 >> (alphaCodeIndex - 16)) & 0x07;
                    }

                    uint8_t finalAlpha;
                    if (alphaCode == 0)
                    {
                        finalAlpha = alpha0;
                    }
                    else if (alphaCode == 1)
                    {
                        finalAlpha = alpha1;
                    }
                    else
                    {
                        if (alpha0 > alpha1)
                        {
                            finalAlpha = ((8 - alphaCode) * alpha0 + (alphaCode - 1) * alpha1) / 7;
                        }
                        else
                        {
                            if (alphaCode == 6)
                                finalAlpha = 0;
                            else if (alphaCode == 7)
                                finalAlpha = 255;
                            else
                                finalAlpha = ((6 - alphaCode) * alpha0 + (alphaCode - 1) * alpha1) / 5;
                        }
                    }

                    uint8_t colorCode = (code >> 2 * (4 * j + i)) & 0x03;

                    Color32 finalColor;
                    switch (colorCode)
                    {
                        case 0:
                            finalColor = Color32(r0, g0, b0, finalAlpha);
                            break;
                        case 1:
                            finalColor = Color32(r1, g1, b1, finalAlpha);
                            break;
                        case 2:
                            finalColor = Color32((2 * r0 + r1) / 3, (2 * g0 + g1) / 3, (2 * b0 + b1) / 3, finalAlpha);
                            break;
                        case 3:
                            finalColor = Color32((r0 + 2 * r1) / 3, (g0 + 2 * g1) / 3, (b0 + 2 * b1) / 3, finalAlpha);
                            break;
                    }

                    if (x + i < width) {
                        image[(y + j) * width + (x + i)] = finalColor;
                    }
                }
            }
        }


        bool decodeDxt5(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream fs(path);
            if (fs.open("rb")) {
                return decodeDxt5(fs, imgData);
            }
            JCORE_ERROR("[Image-IO] (DXT5) Decode Error: Failed to open file '{0}' for reading!", path);
            return false;
        }

        bool decodeDxt5(const Stream& stream, ImageData& img, const ImageDecodeParams params) {

            uint32_t blockCountX = (img.width + 3) / 4;
            uint32_t blockCountY = (img.height + 3) / 4;
            uint32_t blockWidth = (img.width < 4) ? img.width : 4;
            uint32_t blockHeight = (img.height < 4) ? img.height : 4;
            size_t bufSize = blockCountX * 16;
            uint8_t* blockStorage = reinterpret_cast<uint8_t*>(_malloca(bufSize));
            Color32* pixels = reinterpret_cast<Color32*>(img.data);

            if (blockStorage) {
                for (uint32_t j = 0; j < blockCountY; j++)
                {
                    stream.read(blockStorage, 1, bufSize, false);
                    for (uint32_t i = 0, k = 0; i < blockCountX; i++, k += 16) {
                        decompressDXT5Block(i * 4, j * 4, img.width, img.height, blockStorage + k, pixels);
                    }
                }

                _freea(blockStorage);
                return true;
            }

            return false;
        }

    }

    namespace DDS {

        enum DDSCompression : uint32_t {
            DDS_None = 0x00,

            DDS_DXT1 = 0x31585444U,
            DDS_DXT2 = 0x32585444U,
            DDS_DXT3 = 0x33585444U,
            DDS_DXT4 = 0x34585444U,
            DDS_DXT5 = 0x35585444U,
        };

#pragma pack(push, 1)
        struct DDSFormat {
            uint32_t flags{};
            DDSCompression compression{};
            uint32_t bitCount{};
            uint32_t compMasks[4]{};
        };
#pragma pack(pop, 1)

        bool getInfo(std::string_view path, ImageData& imgData) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return getInfo(stream, imgData);
            }

            JCORE_ERROR("[Image-IO] (DDS) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool getInfo(const Stream& stream, ImageData& imgData) {
            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (DDS) Decode Error: Stream isn't open!");
                return false;
            }

            static constexpr uint32_t SIGNATURE = 0x20534444;
            uint32_t sig{};
            stream.readValue(sig, false);

            if (sig != SIGNATURE) {
                JCORE_ERROR("[Image-IO] (DDS) Error: Signature isn't valid!");
                return false;
            }


            size_t dataStart = 0;
            stream.read(&dataStart, 4, 1, false);
            dataStart += (stream.tell() - 4);
            uint32_t flags = 0;
            uint32_t pitch = 0, scanSize = 0;

            stream.readValue(flags, false);
            stream.readValue(imgData.height, false);
            stream.readValue(imgData.width, false);
            stream.readValue(pitch, false);
            stream.seek(56, SEEK_CUR);

            DDSFormat fmt{};
            stream.readValue(fmt, false);

            switch (fmt.compression)
            {
                default:
                    JCORE_ERROR("[Image-IO] (DDS) Error: Unsupported compression format!");
                    return false;
                case DDS_None:
                    break;
            }

            switch (fmt.bitCount)
            {
                default:
                    JCORE_ERROR("[Image-IO] (DDS) Error: Unsupported bit count!");
                    return false;
                case 16:
                    //TODO: Add additional checks for Color565 and Color555
                    //for now we just assume Color4444
                    imgData.format = TextureFormat::RGBA4444;
                    break;
                case 24:
                    imgData.format = TextureFormat::RGB24;
                    break;
                case 32:
                    imgData.format = TextureFormat::RGBA32;
                    break;
            }
            return true;
        }

        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return decode(stream, imgData, params);
            }

            JCORE_ERROR("[Image-IO] (DDS) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params) {
            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (DDS) Decode Error: Stream isn't open!");
                return false;
            }

            static constexpr uint32_t SIGNATURE = 0x20534444;
            uint32_t sig{};
            stream.readValue(sig, false);

            if (sig != SIGNATURE) {
                JCORE_ERROR("[Image-IO] (DDS) Error: Signature isn't valid!");
                return false;
            }

            size_t dataStart = 0;
            stream.read(&dataStart, 4, 1, false);
            dataStart += (stream.tell() - 4);
            uint32_t flags = 0;
            uint32_t pitch = 0, scanSize = 0;

            stream.readValue(flags, false);
            stream.readValue(imgData.height, false);
            stream.readValue(imgData.width, false);
            stream.readValue(pitch, false);
            stream.seek(56, SEEK_CUR);

            DDSFormat fmt{};
            stream.readValue(fmt, false);

            switch (fmt.compression)
            {
                default:
                    JCORE_ERROR("[Image-IO] (DDS) Error: Unsupported compression format!");
                    return false;
                case 0:
                    break;
            }

            switch (fmt.bitCount)
            {
                default:
                    JCORE_ERROR("[Image-IO] (DDS) Error: Unsupported bit count!");
                    return false;
                case 16:
                    //TODO: Add additional checks for Color565 and Color555
                    //for now we just assume Color4444
                    imgData.format = TextureFormat::RGBA4444;
                    pitch = (imgData.width * 16 + 7) / 8;
                    scanSize = imgData.width << 1;
                    break;
                case 24:
                    imgData.format = TextureFormat::RGB24;
                    pitch = (imgData.width * 24 + 7) / 8;
                    scanSize = imgData.width * 3;
                    break;
                case 32:
                    imgData.format = TextureFormat::RGBA32;
                    pitch = imgData.width << 2;
                    scanSize = pitch;
                    break;
            }

            if (!imgData.doAllocate()) {
                JCORE_ERROR("[Image-IO] (DDS) Error: Failed to allocate image data!");
                return false;
            }

            uint8_t* scan = reinterpret_cast<uint8_t*>(_malloca(pitch));
            if (!scan) {
                JCORE_ERROR("[Image-IO] (DDS) Error: Failed to allocate scan buffer!");
                return false;
            }

            stream.seek(dataStart, SEEK_SET);
            const int32_t maskOffsets[4]{
                   Math::findFirstLSB(fmt.compMasks[0]),
                   Math::findFirstLSB(fmt.compMasks[1]),
                   Math::findFirstLSB(fmt.compMasks[2]),
                   Math::findFirstLSB(fmt.compMasks[3]),
            };

            for (int32_t y = 0, yP = 0; y < imgData.height; y++, yP += scanSize) {
                stream.read(scan, pitch, false);
                memcpy(imgData.data + yP, scan, scanSize);
            }

            int32_t reso = imgData.width * imgData.height;
            //Apply color masks
            switch (imgData.format)
            {
                case TextureFormat::RGBA32: {
                    Color32* cPtr = reinterpret_cast<Color32*>(imgData.data);
                    for (size_t i = 0, j = 0; i < reso; i++, j++) {
                        auto& color = cPtr[i];
                        const uint32_t data = uint32_t(color);

                        color.r = (data & fmt.compMasks[0]) >> maskOffsets[0];
                        color.g = (data & fmt.compMasks[1]) >> maskOffsets[1];
                        color.b = (data & fmt.compMasks[2]) >> maskOffsets[2];
                        color.a = (data & fmt.compMasks[3]) >> maskOffsets[3];
                    }
                    break;
                }

                case TextureFormat::RGB24: {
                    Color24* cPtr = reinterpret_cast<Color24*>(imgData.data);
                    for (size_t i = 0, j = 0; i < reso; i++, j++) {
                        auto& color = cPtr[i];
                        const uint32_t data = uint32_t(color);

                        color.r = (data & fmt.compMasks[0]) >> maskOffsets[0];
                        color.g = (data & fmt.compMasks[1]) >> maskOffsets[1];
                        color.b = (data & fmt.compMasks[2]) >> maskOffsets[2];
                    }
                    break;
                }

                case TextureFormat::RGBA4444: {
                    Color4444* cPtr = reinterpret_cast<Color4444*>(imgData.data);
                    for (size_t i = 0, j = 0; i < reso; i++, j++) {
                        auto& color = cPtr[i];
                        const uint32_t data = color.data;
                        color = Color4444(
                            uint16_t((data & fmt.compMasks[0]) >> maskOffsets[0]),
                            uint16_t((data & fmt.compMasks[1]) >> maskOffsets[1]),
                            uint16_t((data & fmt.compMasks[2]) >> maskOffsets[2]),
                            uint16_t((data & fmt.compMasks[3]) >> maskOffsets[3])
                        );
                    }
                    break;
                }
            }

            _freea(scan);
            return true;
        }

        bool encode(std::string_view path, const ImageData& imgData) {
            FileStream fs(path);
            if (fs.open("wb")) {
                return encode(fs, imgData);
            }
            JCORE_ERROR("[Image-IO] (DDS) Encode Error: Failed to open file '{0}' for writing!", path);
            return false;
        }

        bool encode(const Stream& stream, const ImageData& imgData) {
            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (DDS) Encode Error: Stream isn't open!");
                return false;
            }

            switch (imgData.format) {
                default:
                    JCORE_ERROR("[Image-IO] (DDS) Encode Error: Format '{0}' isn't supported!", getTextureFormatName(imgData.format));
                    return false;
                case TextureFormat::R8:
                case TextureFormat::RGB24:
                case TextureFormat::RGBA32:
                case TextureFormat::Indexed8:
                case TextureFormat::Indexed16:
                case TextureFormat::RGBA4444:
                    break;
            }

            DDSFormat fmt{};

            fmt.compression = DDSCompression::DDS_None;
            switch (imgData.format)
            {
                case TextureFormat::R8:
                case TextureFormat::RGB24:
                    fmt.compMasks[0] = 0x000000FFU;
                    fmt.compMasks[1] = 0x0000FF00U;
                    fmt.compMasks[2] = 0x00FF000U;
                    fmt.compMasks[3] = 0x0000000U;
                    fmt.bitCount = 24;
                    fmt.flags = 0x40;
                    break;
                case TextureFormat::RGBA32:
                case TextureFormat::Indexed8:
                case TextureFormat::Indexed16:
                    fmt.compMasks[0] = 0x000000FFU;
                    fmt.compMasks[1] = 0x0000FF00U;
                    fmt.compMasks[2] = 0x00FF000U;
                    fmt.compMasks[3] = 0xFF00000U;
                    fmt.bitCount = 32;
                    fmt.flags = 0x40 | 0x1;
                    break;
                case TextureFormat::RGBA4444:
                    fmt.compMasks[0] = 0x0000000FU;
                    fmt.compMasks[1] = 0x000000F0U;
                    fmt.compMasks[2] = 0x00000F00U;
                    fmt.compMasks[3] = 0x0000F000U;
                    fmt.bitCount = 16;
                    fmt.flags = 0x40 | 0x1;
                    break;
            }

            uint32_t pitch = (imgData.width * fmt.bitCount + 7) / 8;

            uint8_t* scan = reinterpret_cast<uint8_t*>(_malloca(pitch));
            if (!scan) {
                JCORE_ERROR("[Image-IO] (DDS) Error: Failed to allocate scan buffer!");
                return false;
            }
            memset(scan, 0, pitch);

            static constexpr uint32_t SIGNATURE = 0x20534444;
            static constexpr uint32_t FLAGS = 0x1 | 0x2 | 0x4 | 0x8 | 0x1000;
            stream.writeValue(SIGNATURE);
            stream.writeValue(124U);
            stream.writeValue(FLAGS);
            stream.writeValue(imgData.height);
            stream.writeValue(imgData.width);
            stream.writeValue(pitch);
            stream.writeValue<uint8_t>(0, 52);

            stream.writeValue(32);
            stream.writeValue(fmt);

            stream.writeValue(0x1000U);
            stream.writeValue(0x0000U, 4);

            switch (imgData.format)
            {
                case TextureFormat::R8: {
                    size_t scanS = size_t(imgData.width) * 3;
                    for (size_t y = 0, yP = 0; y < imgData.height; y++, yP += imgData.width) {
                        memset(scan, imgData.data[yP], scanS);
                        stream.write(scan, pitch);
                    }
                    break;
                }
                case TextureFormat::RGB24:
                case TextureFormat::RGBA4444:
                case TextureFormat::RGBA32: {
                    size_t scanS = size_t(getBitsPerPixel(imgData.format) >> 3) * imgData.width;
                    for (size_t y = 0, yP = 0; y < imgData.height; y++, yP += scanS) {
                        memcpy(scan, imgData.data + yP, scanS);
                        stream.write(scan, pitch);
                    }
                    break;
                }
                case TextureFormat::Indexed8: {
                    const Color32* palette = reinterpret_cast<const Color32*>(imgData.data);
                    const uint8_t* pixels = imgData.data + 1024;
                    for (size_t y = 0, yP = 0; y < imgData.height; y++, yP += imgData.width) {
                        for (size_t x = 0, xP = 0; x < imgData.width; x++, xP += 4) {
                            memcpy(scan + xP, palette + pixels[yP + x], 4);
                        }
                        stream.write(scan, pitch);
                    }
                    break;
                }
                case TextureFormat::Indexed16: {
                    const Color32* palette = reinterpret_cast<const Color32*>(imgData.data);
                    const uint16_t* pixels = reinterpret_cast<const uint16_t*>(imgData.data + (imgData.paletteSize * sizeof(Color32)));
                    for (size_t y = 0, yP = 0; y < imgData.height; y++, yP += imgData.width) {
                        for (size_t x = 0, xP = 0; x < imgData.width; x++, xP += 4) {
                            memcpy(scan + xP, palette + pixels[yP + x], 4);
                        }
                        stream.write(scan, pitch);
                    }
                    break;
                }
            }
            return true;
        }
    }

    namespace ICO {
        struct ICOHeader {
            uint16_t reserved{};
            uint16_t type{};
            uint16_t resDirs{};
        };
        struct ICORes {
            uint8_t width{};
            uint8_t height{};
            uint8_t palSize{};
            uint8_t reserved{};
            uint16_t planes{};
            uint16_t bpp{};
            uint32_t size{};
            uint16_t offset{};
        };

        bool getInfo(std::string_view path, ImageData& imgData) {
            FileStream stream(path, "rb");
            if (stream.isOpen()) {
                return getInfo(stream, imgData);
            }
            JCORE_ERROR("[Image-IO] (ICO) Decode Error: Failed to open '{0}'!", path);
            return false;
        }
        bool getInfo(const Stream& stream, ImageData& imgData) {
            return false;
        }

        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream stream(path, "rb");
            if (stream.isOpen()) {
                return decode(stream, imgData, params);
            }
            JCORE_ERROR("[Image-IO] (ICO) Decode Error: Failed to open '{0}'!", path);
            return false;
        }
        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params) {
            size_t ogStart = stream.tell();
            ICOHeader ico{};
            stream.readValue(ico, false);

            if (ico.reserved != 0 && ico.type != 1) {
                JCORE_ERROR("[Image-IO] (ICO) Decode Error: Not an ICO file or is corrupted!");
                return false;
            }   

            if (ico.resDirs > 0) {
                JCORE_ERROR("[Image-IO] (ICO) Decode Error: No icon resources!");
                return false;
            }
            ICORes* res = reinterpret_cast<ICORes*>(_malloca(sizeof(ICORes) * ico.resDirs));
            if (!res) {
                JCORE_ERROR("[Image-IO] (ICO) Decode Error: Failed to allocate res dirs!");
                return false;
            }
            stream.read(res, sizeof(ICORes) * ico.resDirs, false);

            int32_t largestRes = 0;
            ICORes* largest = res;
            for (size_t i = 0; i < ico.resDirs; i++) {
                auto& rIco = res[i];
                if (rIco.width < 1 || rIco.height < 0) {
                    stream.seek(ogStart + rIco.offset, SEEK_SET);
                    if (Png::getInfo(stream, imgData)) {
                        int32_t reso = imgData.width * imgData.height;
                        if (reso > largestRes) {
                            largestRes = reso;
                            largest = res + i;
                        }
                    }
                }
                else {
                    int32_t reso = rIco.width * rIco.height;
                    if (reso > largestRes) {
                        largestRes = reso;
                        largest = res + i;
                    }
                }
            }

            stream.seek(ogStart + largest->offset, SEEK_SET);
            bool ret = false;
            if (largest->width == 0 || largest->height == 0) {
                ret = Png::decode(stream, imgData);
            }
            else {
                ret = Bmp::decode(stream, imgData);
            }

            largest = res + (ico.resDirs - 1);
            stream.seek(ogStart + largest->offset + largest->size, SEEK_SET);
            return ret;
        }

        bool encode(std::string_view path, const ImageData& imgData, bool compressed) {
            FileStream stream(path, "rb");
            if (stream.isOpen()) {
                return encode(stream, imgData, compressed);
            }
            JCORE_ERROR("[Image-IO] (ICO) Encode Error: Failed to open '{0}'!", path);
            return false;
        }

        bool encode(const Stream& stream, const ImageData& imgData, bool compressed) {
            return false;
        }

        static size_t calculateMaxIcoSize(const ImageData& icon) {
            return 40ULL + ((size_t(icon.width) * icon.height * 4) +  (size_t(icon.height) * getPaddedWidth(icon.width, 1)));
        }
    }


    namespace JTEX {
        static constexpr uint32_t JTEX_SIG = 0x5845544AU;
        enum JTEXFlags : uint32_t {
            JTEX_None,
            JTEX_Compressed = 0x1,
        };

        bool getInfo(std::string_view path, ImageData& imgData) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return getInfo(stream, imgData);
            }

            JCORE_ERROR("[Image-IO] (JTEX) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool getInfo(const Stream& stream, ImageData& imgData) {
            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (JTEX) Decode Error: Stream isn't open!");
                return false;
            }

#pragma pack(push, 1)
            struct Header {
                uint32_t sig;
                JTEXFlags flags;
                int32_t width;
                int32_t height;
                TextureFormat format;
                int32_t paletteSize;
                uint8_t imgFlags;
            };
#pragma pack(pop, 1)

            Header hdr{};
            stream.readValue(hdr, false);

            if (hdr.sig != JTEX_SIG) {
                JCORE_ERROR("[Image-IO] (JTEX) Decode Error: Signature isn't valid!");
                return false;
            }

            imgData.width = hdr.width;
            imgData.height = hdr.height;
            imgData.format = hdr.format;
            imgData.paletteSize = hdr.paletteSize;
            imgData.flags = hdr.imgFlags;
            return true;
        }

        bool decode(std::string_view path, ImageData& imgData, const ImageDecodeParams params) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                return decode(stream, imgData, params);
            }

            JCORE_ERROR("[Image-IO] (JTEX) Error: Failed to open '{0}'!", path);
            return false;
        }

        bool decode(const Stream& stream, ImageData& imgData, const ImageDecodeParams params) {
            if (!getInfo(stream, imgData)) {
                return false;
            }

            if (!imgData.doAllocate()) {
                JCORE_ERROR("[Image-IO] (JTEX) Decode Error: Failed to allocate pixel buffer!");
                return false;
            }
            stream.read(imgData.data, imgData.getSize(), false);
            return true;
        }

        bool encode(std::string_view path, const ImageData& imgData) {
            FileStream fs(path);
            if (fs.open("wb")) {
                return encode(fs, imgData);
            }
            JCORE_ERROR("[Image-IO] (JTEX) Encode Error: Failed to open file '{0}' for writing!", path);
            return false;
        }

        bool encode(const Stream& stream, const ImageData& imgData) {
            if (!stream.isOpen()) {
                JCORE_ERROR("[Image-IO] (JTEX) Encode Error: Stream isn't open!");
                return false;
            }

            stream.writeValue(JTEX_SIG);
            stream.writeValue(0U);
            stream.writeValue(imgData.width);
            stream.writeValue(imgData.height);
            stream.writeValue(imgData.format);
            stream.writeValue(imgData.paletteSize);
            stream.writeValue(imgData.flags);
            stream.write(imgData.data, imgData.getSize(), false);
            return true;
        }
    }

    namespace Image {
        bool tryGetInfo(std::string_view path, ImageData& imgData, DataFormat& format) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                if (tryGetInfo(stream, imgData, format)) {
                    return true;
                }
                JCORE_ERROR("[Image-IO] Error: Failed to get info for '{0}'!", path);
                return false;;
            }
            JCORE_ERROR("[Image-IO] Error: Failed to open '{0}'!", path);
            return false;
        }

        bool tryGetInfo(const Stream& stream, ImageData& imgData, DataFormat& format) {
            size_t pos = stream.tell();
            format = Format::getFormat(stream);
            stream.seek(pos, SEEK_SET);

            switch (format) {
                default:
                    JCORE_WARN("[Image-IO] Warning: Unsupported image format '{0}'!", Enum::nameOf(format));
                    return false;
                case DataFormat::FMT_PNG:
                    return Png::getInfo(stream, imgData);
                case DataFormat::FMT_BMP:
                    return Bmp::getInfo(stream, imgData);
                case DataFormat::FMT_DDS:
                    return DDS::getInfo(stream, imgData);
                case DataFormat::FMT_JTEX:
                    return JTEX::getInfo(stream, imgData);
            }
        }

        bool tryDecode(std::string_view path, ImageData& imgData, DataFormat& format, const ImageDecodeParams params) {
            FileStream stream(path, "rb");

            if (stream.isOpen()) {
                if (tryDecode(stream, imgData, format, params)) {
                    return true;
                }
                JCORE_ERROR("[Image-IO] Error: Failed to decode '{0}'!", path);
                return false;;
            }

            JCORE_ERROR("[Image-IO] Error: Failed to open '{0}'!", path);
            return false;
        }

        bool tryDecode(const Stream& stream, ImageData& imgData, DataFormat& format, const ImageDecodeParams params) {
            if (!stream.isOpen()) { format = DataFormat::FMT_UNKNOWN; return false; }
            if (format == DataFormat::FMT_UNKNOWN) {
                size_t pos = stream.tell();
                format = Format::getFormat(stream);
                stream.seek(pos, SEEK_SET);
            }
            switch (format)
            {
                default:
                    JCORE_WARN("[Image-IO] Warning: Unsupported image format '{0}'!", Enum::nameOf(format));
                    return false;
                case DataFormat::FMT_PNG:
                    return Png::decode(stream, imgData, params);
                case DataFormat::FMT_BMP:
                    return Bmp::decode(stream, imgData, params);
                case DataFormat::FMT_DDS:
                    return DDS::decode(stream, imgData, params);
                case DataFormat::FMT_JTEX:
                    return JTEX::decode(stream, imgData, params);
            }
        }

        bool tryEncode(std::string_view path, const ImageData& imgData, DataFormat format, const ImageEncodeParams& encodeParams) {
            FileStream fs(path);
            if (fs.open("wb")) {
                return tryEncode(fs, imgData, format, encodeParams);
            }
            JCORE_ERROR("[Image-IO] ({1}) Encode Error: Failed to open file '{0}' for writing!", path, Enum::nameOf(format));
            return false;
        }

        bool tryEncode(const Stream& stream, const ImageData& imgData, DataFormat format, const ImageEncodeParams& encodeParams) {
            switch (format) {
                default:
                    JCORE_WARN("[Image-IO] Warning: Given encoding format is unsupported! ({0})", Enum::nameOf(format));
                    return false;

                case JCore::FMT_PNG:
                    return Png::encode(stream, imgData, encodeParams.compression);
                case JCore::FMT_BMP:
                    return Bmp::encode(stream, imgData, encodeParams.dpi);
                case JCore::FMT_DDS:
                    return DDS::encode(stream, imgData);
                case JCore::FMT_JTEX:
                    return JTEX::encode(stream, imgData);
            }
        }
    }
}

