#pragma once
#include <string>
#include <J-Core/Util/Span.h>

namespace JCore::Helpers {
    enum IntBase : int32_t {
        IBase_Null = 0x00,
        IBase_2 = 0x02,
        IBase_8 = 0x08,
        IBase_10 = 0x0A,
    };

    size_t strIIndexOf(const std::string_view& strA, const std::string_view& strB);

    bool strIEquals(const char* strA, const size_t lenA, const char* strB, const size_t lenB);
    bool strIEquals(const std::string_view& strA, const std::string_view& strB);

    bool strEquals(const char* strA, const size_t lenA, const char* strB, const size_t lenB);
    bool strEquals(const std::string_view& strA, const std::string_view& strB);

    bool shouldBeWide(const wchar_t* str, int32_t len);
    bool wideToASCII(wchar_t* str, int32_t len);

    void formatDataSize(char* buffer, size_t size);
    void formatDataSize(char* buffer, double size);

    bool endsWith(const char* str, const char* end, bool caseSensitive = true);
    bool startsWith(const char* str, const char* end, bool caseSensitive = true);

    bool endsWith(const char* str, size_t len, const char* end, bool caseSensitive = true);
    bool startsWith(const char* str, size_t len, const char* end, bool caseSensitive = true);

    size_t lastIndexOf(const char* str, const char* end, bool caseSensitive = true);
    size_t indexOf(const char* str, const char* end, bool caseSensitive = true);

    size_t lastIndexOf(const char* str, size_t len, const char* end, bool caseSensitive = true);
    size_t indexOf(const char* str, size_t len, const char* end, bool caseSensitive = true);

    size_t lastIndexOf(const char* str, const char* const* end, size_t endCount, bool caseSensitive = true);
    size_t indexOf(const char* str, const char* const* end, size_t endCount, bool caseSensitive = true);

    size_t lastIndexOf(const char* str, size_t len, const char* const* end, size_t endCount, bool caseSensitive = true);
    size_t indexOf(const char* str, size_t len, const char* const* end, size_t endCount, bool caseSensitive = true);

    template<size_t bufSize>
    size_t lastIndexOf(const char* str, const char* (&end)[bufSize], bool caseSensitive = true) {
        return lastIndexOf(str, strlen(str), end, bufSize, caseSensitive);
    }
    template<size_t bufSize>
    size_t indexOf(const char* str, const char* (&end)[bufSize], bool caseSensitive = true) {
        return indexOf(str, strlen(str), end, bufSize, caseSensitive);
    }

    template<size_t bufSize>
    size_t lastIndexOf(const char* str, size_t len, const char* (&end)[bufSize], bool caseSensitive = true) {
        return lastIndexOf(str, len, end, bufSize, caseSensitive);
    }
    template<size_t bufSize>
    size_t indexOf(const char* str, size_t len, const char* (&end)[bufSize], bool caseSensitive = true) {
        return indexOf(str, len, end, bufSize, caseSensitive);
    }

    inline constexpr uint8_t parseHexChar(char ch) {
        if (ch < '0') { return 0xFF; }
        if (ch < '9') { return uint8_t(ch - '0'); }
        if (ch >= 'a' && ch <= 'f') { return uint8_t(ch - 'a' + 10); }
        return ch >= 'A' ? uint8_t(ch - 'A' + 10) : 0xFF;
    }

    template<typename T>
    int32_t parseIntBase(const char* str, size_t length, T& output, int32_t base = IBase_Null) {
        if (!str || length == 0) { return IBase_Null; }
        if (length < 2) {

            if ((str[0] >= '0' && str[0] <= '9')) {
                char val = str[0] - '0';
                memcpy(&output, &val, 1);
                return IBase_10;
            }
            return IBase_Null;
        }
        static constexpr size_t UINT_COUNT = sizeof(T) <= sizeof(uint64_t) ? 1 : (sizeof(T) & 0x8) ? (sizeof(T) >> 3) : (sizeof(T) >> 3) + 1;

        uint32_t offset = 0;
        if (base == IBase_Null) {
            base = IBase_10;
            if (str[0] == '0') {
                base = IBase_Null;
                switch (str[1]) {
                    default:
                        base = str[1] >= '0' && str[1] <= '9' ? IBase_10 : IBase_Null;
                        break;
                    case 'b':
                    case 'B':
                        base = IBase_2;
                        offset = 2;
                        break;
                    case 'x':
                    case 'X':
                        base = IBase_8;
                        offset = 2;
                        break;
                    case 'd':
                    case 'D':
                        base = IBase_10;
                        offset = 2;
                        break;
                }
            }
        }

        uint64_t curUI = 0;
        uint64_t value[UINT_COUNT]{ 0 };
        int32_t shift = 0;
        size_t i = length;
        switch (base) {
            default: return IBase_Null;
            case IBase_2:
                shift = 0;
                while (i > offset) {
                    uint64_t val = str[--i];
                    if (val < '0' || val > '1') { return IBase_Null; }
                    value[curUI] |= ((val - '0') << shift++);

                    if (sizeof(T) <= sizeof(uint64_t) && shift >= (sizeof(T) << 3)) {
                        break;
                    }

                    if (shift >= (sizeof(uint64_t) << 3)) {
                        shift = 0;
                        curUI++;
                        if (curUI >= UINT_COUNT) { break; }
                    }
                }
                break;
            case IBase_8:
                shift = 0;
                while (i > offset && (shift < (sizeof(T) << 3))) {
                    uint64_t val = parseHexChar(str[--i]);
                    if (val > 15) { return IBase_Null; }
                    value[curUI] |= (val << shift);
                    shift += 4;

                    if (sizeof(T) <= sizeof(uint64_t) && shift >= (sizeof(T) << 3)) {
                        break;
                    }

                    if (shift >= (sizeof(uint64_t) << 3)) {
                        shift = 0;
                        curUI++;
                        if (curUI >= UINT_COUNT) { break; }
                    }
                }
                break;
            case IBase_10:
                shift = 1;
                while (i > offset) {
                    uint64_t val = str[--i];
                    if (val < '0' || val > '9') { return IBase_Null; }
                    value[curUI] += (val - '0') * shift;
                    shift *= 10;
                }
                break;
        }
        memcpy(&output, value, sizeof(T));
        return base;
    }

    template<typename T>
    bool tryParseInt(const char* str, size_t length, T& output, int32_t base = IBase_Null, const T& nullVal = T()) {
        base = parseIntBase<T>(str, length, output, base);
        if (base == IBase_Null) {
            output = nullVal;
            return false;
        }
        return true;
    }

    template<typename T>
    bool tryParseInt(ConstSpan<char> span, T& output, int32_t base = IBase_Null, const T& nullVal = T()) {
        base = parseIntBase<T>(span.get(), span.length(), output, base);
        if (base == IBase_Null) {
            output = nullVal;
            return false;
        }
        return true;
    }

    template<typename T>
    bool tryParseInt(Span<char> span, T& output, int32_t base = IBase_Null, const T& nullVal = T()) {
        base = parseIntBase<T>(span.get(), span.length(), output, base);
        if (base == IBase_Null) {
            output = nullVal;
            return false;
        }
        return true;
    }
}