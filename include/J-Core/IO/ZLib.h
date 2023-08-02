#pragma once
#include <zlib.h>
#include <cstdint>
#include <J-Core/IO/Stream.h>

namespace JCore::ZLib {
    struct ZLibContext {
        z_stream stream;
        bool initialized = false;

        ZLibContext() : stream() {}
        ZLibContext(void* dataIn, const size_t lenIn, void* dataOut, const size_t lenOut) : stream() {
            init(dataIn, lenIn, dataOut, lenOut);
        }

        void init(void* dataIn, const size_t lenIn, void* dataOut, const size_t lenOut) {
            stream.zalloc = 0;
            stream.zfree = 0;

            stream.total_in = 0;
            stream.avail_in = uInt(lenIn);
            
            stream.total_out = 0;
            stream.avail_out = uInt(lenOut);

            stream.next_in = reinterpret_cast<uint8_t*>(dataIn);
            stream.next_out = reinterpret_cast<uint8_t*>(dataOut);

            initialized = true;
        }

        void refreshNext(void* dataIn, const size_t lenIn) {
            stream.next_in = reinterpret_cast<uint8_t*>(dataIn);
            stream.avail_in = uInt(lenIn);
        }
    };

    static constexpr int32_t CHUNK = 16384;
    int32_t deflateData(const Stream& streamIn, Stream& target, const int32_t level);
    int32_t inflateData(const Stream& streamIn, Stream& target);

    int32_t deflateData(void* dataIn, const size_t lenIn, void* dataOut, const size_t lenOut, const int32_t level);
    int32_t inflateData(void* dataIn, const size_t lenIn, void* dataOut, const size_t lenOut);

    int32_t deflateBegin(ZLibContext& context, uint32_t level, void* buffer, const size_t bufferSize);
    int32_t deflateSegment(ZLibContext& context, void* dataIn, const size_t lenIn, const Stream& streamIn, void* buffer, const size_t bufferSize);
    int32_t deflateEnd(ZLibContext& context, int32_t& dataOut, const Stream& streamIn, void* buffer, const size_t bufferSize);

    int32_t inflateBegin(ZLibContext& context, void* dataOut, const size_t lenOut);
    int32_t inflateSegment(ZLibContext& context, void* dataIn, const size_t lenIn, int32_t& dataOut, const bool isLast);
    int32_t inflateEnd(ZLibContext& context, int32_t& dataOut);

    static inline constexpr const char* zerr(const int32_t ret) {
        switch (ret) {
            default:              return "";
            case Z_ERRNO:         return "ZLib error!";
            case Z_STREAM_ERROR:  return "ZStream error!";
            case Z_DATA_ERROR:    return "Invalid or incomplete deflate data!";
            case Z_MEM_ERROR:     return "ZLib out of memory!";
            case Z_VERSION_ERROR: return "ZLib version mismatch!";
        }
    }
}