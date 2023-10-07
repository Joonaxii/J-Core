#include <J-Core/Util/StringHelpers.h>

namespace JCore::Helpers {
    size_t strIIndexOf(const std::string_view& strA, const std::string_view& strB) {
        if (strB.length() < 1) { return 0; }
        size_t c = 0;
        size_t ind = 0;

        char cB = tolower(strB[c]);
        for (size_t i = 0; i < strA.length(); i++) {
            char cA = tolower(strA[i]);
            if (cA == cB) {
                if (c >= strB.length()) { return ind; }
                if (c == 0) {
                    ind = i;
                }
                cB = tolower(strB[++c]);
            }
            else if (c) {
                c = 0;
                cB = tolower(strB[c]);
            }
        }
        return std::string::npos;
    }

    bool strIEquals(const char* strA, const size_t lenA, const char* strB, const size_t lenB) {
        return strIEquals(std::string_view(strA, lenA), std::string_view(strB, lenB));
    }
    bool strIEquals(const std::string_view& strA, const std::string_view& strB) {
        if (strA.length() != strB.length()) { return false; }
        for (size_t i = 0; i < strA.length(); i++) {
            if (tolower(strA[i]) != tolower(strB[i])) { return false; }
        }
        return true;
    }

    bool strIContains(const char* strA, const char* strB) {
        return strIContains(std::string_view(strA, strlen(strA)), std::string_view(strB, strlen(strB)));
    }  
    bool strIContains(const char* strA, const size_t lenA, const char* strB, const size_t lenB) {
        return strIContains(std::string_view(strA, lenA), std::string_view(strB, lenB));
    }
    bool strIContains(const std::string_view& strA, const std::string_view& strB) {
        if (strB.length() < 1) { return true; }

        if (strB.length() > strA.length()) { return false; }
        size_t inRow = 0;
        for (size_t i = 0; i < strA.length(); i++) {
            if (tolower(strA[i]) == tolower(strB[inRow++])) 
            { 
                if (inRow >= strB.length()) { return true; }
                continue;
            }
            inRow = 0;
        }
        return false;
    }

    bool strEquals(const char* strA, const size_t lenA, const char* strB, const size_t lenB) {
        return strIEquals(std::string_view(strA, lenA), std::string_view(strB, lenB));
    }
    bool strEquals(const std::string_view& strA, const std::string_view& strB) {
        if (strA.length() != strB.length()) { return false; }
        for (size_t i = 0; i < strA.length(); i++) {
            if (strA[i] != strB[i]) { return false; }
        }
        return true;
    }

    bool shouldBeWide(const wchar_t* str, int32_t len) {
        for (size_t i = 0; i < len; i++) {
            if (str[i] > 255) { return true; }
        }
        return false;
    }

    bool wideToASCII(wchar_t* str, int32_t len) {
        char* ascii = reinterpret_cast<char*>(str);
        for (size_t i = 0; i < len; i++) {
            ascii[i] = char(str[i] & 0xFF);
        }
        return false;
    }

    void formatDataSize(char* buffer, size_t size) {
        static constexpr const char* SIZES[]{
            "bytes",
            "KB",
            "MB",
            "GB",
            "TB",
        };

        double val = double(size);
        int32_t ind = 0;
        while (val >= 1024 && ind < (sizeof(SIZES) / sizeof(char*) - 1)) {
            val /= 1024.0;
            ind++;
        }
        sprintf(buffer, "%.3f %s", val, SIZES[ind]);
    }

    void formatDataSize(char* buffer, double size) {
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
        sprintf(buffer, "%.3f %s", size, SIZES[ind]);
    }

    bool endsWith(const char* str, size_t len, const char* end, bool caseSensitive) {
        size_t lenB = strlen(end);

        if (len < lenB) { return false; }

        for (int i = len - 1, j = lenB - 1; j >= 0; i--, j--) {
            char cA = str[i];
            char cB = end[j];

            if (!caseSensitive) {
                cA = char(tolower(cA));
                cB = char(tolower(cB));
            }

            if (cA != cB) { return false; }
        }
        return true;
    }

    bool startsWith(const char* str, size_t len, const char* end, bool caseSensitive) {
        size_t lenB = strlen(end);

        if (len < lenB) { return false; }

        for (size_t i = 0, j = 0; j < lenB; i++, j++) {
            char cA = str[i];
            char cB = end[j];
            if (!caseSensitive) {
                cA = char(tolower(cA));
                cB = char(tolower(cB));
            }
            if (cA != cB) { return false; }
        }
        return true;
    }

    size_t lastIndexOf(const char* str, size_t len, const char* end, bool caseSensitive) {
        if (len < 1 || !str) { return std::string::npos; }
        size_t bufSize = strlen(end);
        size_t i = len;
        size_t j = bufSize;
        size_t sI = 0;
        while (i > 0)
        {
            char cA = str[--i];
            char cB = end[--j];

            if (!caseSensitive) {
                cA = tolower(cA);
                cB = tolower(cB);
            }

            if (cA == cB)
            {
                if (j <= 0) {
                    return i;
                }
                continue;
            }
            if (j != bufSize) { j = bufSize; }
        }
        return std::string::npos;
    }
    size_t indexOf(const char* str, size_t len, const char* end, bool caseSensitive) {
        if (!str) { return std::string::npos; }
        size_t j = 0;
        size_t sI = 0;
        size_t bufSize = strlen(end);
        for (size_t i = 0; i < len; i++) {
            char cA = str[i++];
            char cB = end[j++];

            if (!caseSensitive) {
                cA = tolower(cA);
                cB = tolower(cB);
            }

            if (cA == cB)
            {
                if (j == 1) {
                    sI = i;
                }
                if (j >= bufSize) { return sI; }
                continue;
            }

            if (j != 0) { j = 0; }
        }
        return std::string::npos;
    }

    size_t lastIndexOf(const char* str, const char* end, bool caseSensitive) {
        return lastIndexOf(str, strlen(str), end, caseSensitive);
    }

    size_t indexOf(const char* str, const char* end, bool caseSensitive) {
        return indexOf(str, strlen(str), end, caseSensitive);
    }

    bool endsWith(const char* str, const char* end, bool caseSensitive) {
        return endsWith(str, strlen(str), end, caseSensitive);
    }

    bool startsWith(const char* str, const char* end, bool caseSensitive) {
        return startsWith(str, strlen(str), end, caseSensitive);
    }

    size_t lastIndexOf(const char* str, const char* const* end, size_t endCount, bool caseSensitive) {
        return lastIndexOf(str, strlen(str), end, endCount, caseSensitive);
    }
    size_t indexOf(const char* str, const char* const* end, size_t endCount, bool caseSensitive) {
        return indexOf(str, strlen(str), end, endCount, caseSensitive);
    }

    size_t lastIndexOf(const char* str, size_t len, const char* const* end, size_t endCount, bool caseSensitive) {
        if(endCount < 1 || !end) { return std::string::npos; }
        size_t ind = lastIndexOf(str, len, end[0], caseSensitive);
        for (size_t i = 1; i < endCount; i++) {
            auto ii = lastIndexOf(str, len, end[i], caseSensitive);
            ind = (ind == std::string::npos) ? ii : ii != std::string::npos ? std::max(ii, ind) : ind;
        }
        return ind;
    }
    size_t indexOf(const char* str, size_t len, const char* const* end, size_t endCount, bool caseSensitive) {
        if (endCount < 1 || !end) { return std::string::npos; }
        size_t ind = indexOf(str, len, end[0], caseSensitive);
        for (size_t i = 1; i < endCount; i++) {
            ind = std::min(indexOf(str, len, end[i], caseSensitive), ind);
        }
        return ind;
    }

    size_t indexOfNonNum(const char* str, size_t len) {
        for (size_t i = 0; i < len; i++) {
            if (str[i] < '0' || str[i] > '9') { return i; }
        }
        return len;
    }

}