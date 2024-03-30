#include <J-Core/Util/DataFormatUtils.h>
#include <J-Core/Util/Span.h>
#include <J-Core/IO/FileStream.h>

namespace JCore {
    namespace Format {
        const FormatInfo* getFormats() {
            static constexpr uint8_t PNG_SIG[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
            static constexpr uint8_t BMP_SIG[] = { 0x42, 0x4D, };
            static constexpr uint8_t DDS_SIG[] = { 0x20, 0x53, 0x44, 0x44, };
            static constexpr uint8_t JPEG_SIG[] = { 0xFF, 0xD8, 0xFF, 0xDB, };
            static constexpr uint8_t GIF87_SIG[] = { 0x47, 0x49, 0x46, 0x38, 0x37, 0x61, };
            static constexpr uint8_t GIF89_SIG[] = { 0x47, 0x49, 0x46, 0x38, 0x39, 0x61, };
            static constexpr uint8_t WEBP_SIG[] = { 0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x45, 0x42, 0x50, };
            static constexpr uint8_t WAVE_SIG[] = { 0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, };
            static constexpr uint8_t OGG_SIG[] = { 0x4F, 0x67, 0x67, 0x53, };
            static constexpr uint8_t MP3_SIG[] = { 0x49, 0x44, 0x33, };
            static constexpr uint8_t TTF_SIG[] = { 0x00, 0x01, 0x00, 0x00, 0x00, };
            static constexpr uint8_t OTF_SIG[] = { 0x4F, 0x54, 0x54, 0x4F, };
            static constexpr uint8_t JTEX_SIG[] = { 0x58, 0x45, 0x54, 0x4A, };
            static FormatInfo formats[_FMT_COUNT]{
                {}, {}, {}, {}, //Raw binary & text formats

                {}, //Identifiable formats marker

                FormatInfo(PNG_SIG  , 0x0FF, 8),
                FormatInfo(BMP_SIG  , 0x003, 2),
                FormatInfo(DDS_SIG , 0x00F, 4),
                FormatInfo(JPEG_SIG , 0x00F, 4),
                FormatInfo(GIF87_SIG, 0x03F, 6),
                FormatInfo(GIF89_SIG, 0x03F, 6),
                FormatInfo(WEBP_SIG , 0xF0F, 12),

                FormatInfo(WAVE_SIG , 0xF0F, 12),
                FormatInfo(OGG_SIG  , 0x00F, 4),
                FormatInfo(MP3_SIG  , 0x007, 3),

                FormatInfo(TTF_SIG  , 0x01F, 5),
                FormatInfo(OTF_SIG  , 0x00F, 4),
                FormatInfo(JTEX_SIG , 0x00F, 4),
            };
            return formats;
        }

        size_t getLargestFormat() {
            static size_t maxSize = 0;
            if (maxSize == 0) {
                auto formats = getFormats();
                for (size_t i = _FMT_START + 1; i < _FMT_COUNT; i++) {
                    maxSize = std::max(maxSize, formats[i].size);
                }
            }
            return maxSize;
        }

        DataFormat getFormat(const char* path, int64_t size, DataFormatFlags flags, std::function<void(void*, size_t)> dataTr) {
            FileStream fs(path, "rb");
            if (fs.isOpen()) {
                return getFormat(fs, size, flags, dataTr);
            }
            return DataFormat::FMT_UNKNOWN;
        }
        DataFormat getFormat(const Stream& stream, int64_t size, DataFormatFlags flags, std::function<void(void*, size_t)> dataTr) {
            size_t start = stream.tell();
            size = size < 0 ? int64_t(stream.size() - start) : size;

            size_t pos = start;
            static const FormatInfo* formats = getFormats();
            static size_t maxSize = getLargestFormat();

            uint8_t* buffer = reinterpret_cast<uint8_t*>(_malloca(maxSize));

            if (!buffer) {
                JCORE_ERROR("[J-Core - DataUtils] Error: Couldn't allocate header buffer! ({0})", maxSize);
                return DataFormat::FMT_BINARY;
            }

            DataFormat fmt = FMT_BINARY;
            size_t len = stream.read(buffer, maxSize, false);

            if (dataTr) {
                dataTr(buffer, len);
            }
            for (size_t i = _FMT_START; i < _FMT_COUNT; i++) {
                if (formats[i].isValid(buffer, len)) {
                    fmt = DataFormat(i);
                    if ((flags & FMT_F_RESET_POS) == 0) {
                        pos += formats[i].size;
                    }
                    break;
                }
            }

            if (fmt == FMT_BINARY) {
                uint8_t analysisM = flags >> FMT_F_ANALYZE_SHIFT;

                if (analysisM > 0) {
                    uint8_t* data = reinterpret_cast<uint8_t*>(_malloca(size));
                    if (data) {
                        stream.read(data, size, false);
                        if (dataTr) {
                            dataTr(data, size);
                        }

                        pos += size;
                        if (analysisM > 1) {
                            if (data[0] == '{' && data[size - 1] == '}') {
                                fmt = FMT_JSON;
                                goto end;
                            }
                        }

                        static constexpr float MIN_PRINT = 0.75f;

                        size_t printable = 0;
                        bool even = (size & 0x1) == 0;
                        for (int64_t i = 0; i < size; i++) {
                            if (isprint(data[i])) {
                                printable++;
                            }
                        }

                        if (float(printable) / size >= MIN_PRINT) {
                            fmt = FMT_TEXT;
                        }

                    end:
                        _freea(data);
                    }
                    else {
                        JCORE_WARN("[J-Core - DataUtils] Warn: Couldn't allocate analysis buffer! ({0})", size);
                    }
                }
            }

            stream.seek(pos, SEEK_SET);
            return fmt;
        }

        std::string_view getExtension(const char* path) {
            return getExtension(getFormat(path));
        }
        std::string_view getExtension(const Stream& stream) {
            return getExtension(getFormat(stream));
        }

        
    }
}