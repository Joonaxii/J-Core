#include <J-Core/Util/StringUtils.h>
#include <cstdarg>

namespace JCore::Utils {
    void formatDataSize(char* buffer, size_t bufSize, double size) {
        static constexpr const char* SIZES[]{
            "bytes",
            "KB",
            "MB",
            "GB",
            "TB",
        };

        int32_t ind = 0;
        while (size >= 1024 && ind < (sizeof(SIZES) / sizeof(char*) - 1)) {
            size /= 1024.0;
            ind++;
        }
        sprintf_s(buffer, bufSize, "%.3f %s", size, SIZES[ind]);
    }

    std::string& appendFormat(std::string& str, const char* format, ...) {
        va_list args;
        va_start(args, format);
        size_t len = std::vsnprintf(NULL, 0, format, args);
        va_end(args);

        if (len <= 1) {
            return str;
        }
        size_t s = str.size();
        str.resize(s + len);

        va_start(args, format);
        std::vsnprintf(str.data() + s, len + 1, format, args);
        va_end(args);
        return str;
    }
}