#pragma once
#include <string_view>
#include <cwctype>

#define MAKE_STR_TRIM(STRA, STRB, WSPACE) \
__forceinline STRA trimStart(STRA view, STRB trimChars = WSPACE) {\
    return detail::trimStart(view, trimChars);\
}\
__forceinline STRA trimEnd(STRA view, STRB trimChars = WSPACE) {\
    return detail::trimEnd(view, trimChars);\
}\
__forceinline STRA trim(STRA view, STRB trimChars = WSPACE) {\
    return detail::trim(view, trimChars);\
}

#define MAKE_STR_TRIM_(STRA, STRB) \
__forceinline STRA trimStart(STRA view, STRB trimChars) {\
    return detail::trimStart(view, trimChars);\
}\
__forceinline STRA trimEnd(STRA view, STRB trimChars) {\
    return detail::trimEnd(view, trimChars);\
}\
__forceinline STRA trim(STRA view, STRB trimChars) {\
    return detail::trim(view, trimChars);\
}

#define MAKE_STR_FIND_CH(STRA, CH) \
__forceinline size_t indexOf(STRA view, CH ch) {\
    return detail::indexOf(view, ch);\
}\
__forceinline size_t indexOfI(STRA view, CH ch) {\
    return detail::indexOfI(view, ch);\
}\
__forceinline size_t lastIndexOf(STRA view, CH ch) {\
    return detail::lastIndexOf(view, ch);\
}\
__forceinline size_t lastIndexOfI(STRA view, CH ch) {\
    return detail::lastIndexOfI(view, ch);\
}\
__forceinline bool strContains(STRA view, CH ch) {\
    return detail::strContains(view, ch);\
}\
__forceinline bool striContains(STRA view, CH ch) {\
    return detail::strIContains(view, ch);\
}

#define MAKE_STR_TO_STR(STRA, STRB) \
__forceinline size_t indexOf(STRA lhs, STRB rhs) {\
    return detail::indexOf(lhs, rhs);\
}\
__forceinline size_t indexOfI(STRA lhs, STRB rhs) {\
    return detail::indexOfI(lhs, rhs);\
}\
__forceinline size_t lastIndexOf(STRA lhs, STRB rhs) {\
    return detail::lastIndexOf(lhs, rhs);\
}\
__forceinline size_t lastIndexOfI(STRA lhs, STRB rhs) {\
    return detail::lastIndexOfI(lhs, rhs);\
}\
__forceinline bool strContains(STRA lhs, STRB rhs) {\
    return detail::strContains(lhs, rhs);\
}\
__forceinline bool strIContains(STRA lhs, STRB rhs) {\
    return detail::strIContains(lhs, rhs);\
}\
__forceinline bool startsWith(STRA lhs, STRB rhs, bool caseSensitive = false) {\
    return detail::startsWith(lhs, rhs, caseSensitive);\
}\
__forceinline bool endsWith(STRA lhs, STRB rhs, bool caseSensitive = false) {\
    return detail::endsWith(lhs, rhs, caseSensitive);\
}\
__forceinline bool strEquals(STRA lhs, STRB rhs) {\
    return detail::strEquals(lhs, rhs);\
}\
__forceinline bool strIEquals(STRA lhs, STRB rhs) {\
    return detail::strIEquals(lhs, rhs);\
}

#define MAKE_STR_MISC(STRA) \
__forceinline bool isWhiteSpace(STRA view) {\
    return detail::isWhiteSpace(view);\
} \
__forceinline size_t indexOfNonNum(STRA view) {\
    return detail::indexOfNonNum(view);\
} 

namespace JCore::Utils {
    static constexpr std::string_view WHITE_SPACE = " \n\r\t\v\0";
    static constexpr std::wstring_view WWHITE_SPACE = L" \n\r\t\v\0";

    namespace detail {
        template<typename StrCh>
        bool isWhiteSpace(std::basic_string_view<StrCh> view) {
            for (size_t i = 0; i < view.length(); i++) {
                if (view[i] && iswspace(int32_t(view[i])) == 0) {
                    return false;
                }
            }
            return true;
        }

        template<typename StrA, typename CharVal>
        size_t indexOf(std::basic_string_view<StrA> view, CharVal ch) {
            for (size_t i = 0; i < view.size(); i++) {
                if (view[i] == ch) {
                    return i;
                }
            }
            return std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename CharVal>
        size_t lastIndexOf(std::basic_string_view<StrA> view, CharVal ch) {
            size_t lPos = view.length();
            while (lPos > 0) {
                if (view[--lPos] == ch) {
                    return lPos;
                }
            }
            return std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename CharVal>
        size_t indexOfI(std::basic_string_view<StrA> view, CharVal ch) {
            int32_t low = towlower(int32_t(ch));
            for (size_t i = 0; i < view.size(); i++) {
                if (towlower(int32_t(view[i])) == low) {
                    return i;
                }
            }
            return std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename CharVal>
        size_t lastIndexOfI(std::basic_string_view<StrA> view, CharVal ch) {
            size_t lPos = view.length();
            int32_t low = towlower(int32_t(ch));
            while (lPos > 0) {
                if (towlower(int32_t(view[--lPos])) == low) {
                    return lPos;
                }
            }
            return std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename StrB>
        size_t indexOf(std::basic_string_view<StrA> lhs, std::basic_string_view<StrB> rhs) {
            if (lhs.length() < 1 || rhs.size() > lhs.size()) { return std::basic_string_view<StrA>::npos; }
            for (size_t i = 0, j = 0; i < lhs.size(); i++) {
                if (lhs[i] == rhs[j++]) {
                    if (j >= rhs.size()) {
                        return i - rhs.size();
                    }
                    continue;
                }
                j = 0;
            }
            return std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename StrB>
        size_t lastIndexOf(std::basic_string_view<StrA> lhs, std::basic_string_view<StrB> rhs) {
            if (lhs.length() < 1 || rhs.size() > lhs.size()) { return std::basic_string_view<StrA>::npos; }

            size_t lPos = lhs.size() - rhs.size();
            while (lPos > 0) {
                for (size_t i = 0, j = lPos; i < rhs.size(); i++, j++) {
                    if (lhs[j] != rhs[i]) {
                        lPos--;
                        break;
                    }
                }
            }

            for (size_t i = 0; i < rhs.size(); i++) {
                if (lhs[i] != rhs[i]) {
                    return std::basic_string_view<StrA>::npos;
                }
            }
            return 0;
        }

        template<typename StrA, typename StrB>
        size_t indexOfI(std::basic_string_view<StrA> lhs, std::basic_string_view<StrB> rhs) {
            if (lhs.length() < 1 || rhs.size() > lhs.size()) { return std::basic_string_view<StrA>::npos; }
            for (size_t i = 0, j = 0; i < lhs.size(); i++) {
                if (towlower(int32_t(lhs[i])) == towlower(int32_t(rhs[j++]))) {
                    if (j >= rhs.size()) {
                        return i - rhs.size();
                    }
                    continue;
                }
                j = 0;
            }
            return std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename StrB>
        size_t lastIndexOfI(std::basic_string_view<StrA> lhs, std::basic_string_view<StrB> rhs) {
            if (lhs.length() < 1 || rhs.size() > lhs.size()) { return std::basic_string_view<StrA>::npos; }

            size_t lPos = lhs.size() - rhs.size();
            while (lPos > 0) {
                for (size_t i = 0, j = lPos; i < rhs.size(); i++, j++) {
                    if (towlower(int32_t(lhs[i])) != towlower(int32_t(rhs[j]))) {
                        lPos--;
                        break;
                    }
                }
            }

            for (size_t i = 0; i < rhs.size(); i++) {
                if (towlower(int32_t(lhs[i])) != towlower(int32_t(rhs[i]))) {
                    return std::basic_string_view<StrA>::npos;
                }
            }
            return 0;
        }

        template<typename StrA, typename StrB = std::string_view>
        std::basic_string_view<StrA> trimStart(std::basic_string_view<StrA> view, std::basic_string_view<StrB> trimChars = WHITE_SPACE) {
            if (view.length() < 1) { return view; }
            size_t start = 0;
            for (size_t i = 0; i < view.size(); i++) {
                if (indexOf<StrB, StrA>(trimChars, view[i]) == std::string_view::npos) { break; }
                start++;
            }
            return view.substr(start);
        }

        template<typename StrA, typename StrB = std::string_view>
        std::basic_string_view<StrA> trimEnd(std::basic_string_view<StrA> view, std::basic_string_view<StrB> trimChars = WHITE_SPACE) {
            if (view.length() < 1) { return view; }
            size_t end = view.size();
            size_t len = view.size() - 1;
            for (size_t i = 0; i < view.size(); i++) {
                if (indexOf<StrB, StrA>(trimChars, view[len - i]) == std::string_view::npos) { break; }
                end--;
            }
            return view.substr(0, end);
        }

        template<typename StrA, typename StrB = std::string_view>
        std::basic_string_view<StrA> trim(std::basic_string_view<StrA> view, std::basic_string_view<StrB> trimChars = WHITE_SPACE) {
            return trimEnd<StrA, StrB>(trimStart<StrA, StrB>(view, trimChars), trimChars);
        }

        template<typename StrA, typename StrB>
        bool strContains(std::basic_string_view<StrA> strA, std::basic_string_view<StrB> strB) {
            return indexOf<StrA, StrB>(strA, strB) != std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename StrB>
        bool strIContains(std::basic_string_view<StrA> strA, std::basic_string_view<StrB> strB) {
            return indexOfI<StrA, StrB>(strA, strB) != std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename ChB>
        bool strContains(std::basic_string_view<StrA> strA, ChB chVal) {
            return indexOf<StrA, ChB>(strA, chVal) != std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename ChB>
        bool strIContains(std::basic_string_view<StrA> strA, ChB chVal) {
            return indexOfI<StrA, ChB>(strA, chVal) != std::basic_string_view<StrA>::npos;
        }

        template<typename StrA, typename StrB>
        bool strEquals(std::basic_string_view<StrA> strA, std::basic_string_view<StrB> strB) {
            if (strA.length() != strB.length()) { return false; }
            for (size_t i = 0; i < strA.length(); i++) {
                if (strA[i] != strB[i]) { return false; }
            }
            return true;
        }

        template<typename StrA, typename StrB>
        bool strIEquals(std::basic_string_view<StrA> strA, std::basic_string_view<StrB> strB) {
            if (strA.length() != strB.length()) { return false; }
            for (size_t i = 0; i < strA.length(); i++) {
                if (towlower(wchar_t(strA[i])) != towlower(wchar_t(strB[i]))) { return false; }
            }
            return true;
        }

        template<typename StrA, typename StrB>
        bool endsWith(std::basic_string_view<StrA> strA, std::basic_string_view<StrB> strB, bool caseSensitive = false) {
            if (strA.length() < strB.length()) { return false; }
            std::basic_string_view<StrA> end = strA.substr(strA.length() - strB.length());
            return caseSensitive ? strEquals(end, strB) : strIEquals(end, strB);
        }

        template<typename StrA, typename StrB>
        bool startsWith(std::basic_string_view<StrA> strA, std::basic_string_view<StrB> strB, bool caseSensitive = false) {
            if (strA.length() < strB.length()) { return false; }
            std::basic_string_view<StrA> start = strA.substr(0, strB.length());
            return caseSensitive ? strEquals(start, strB) : strIEquals(start, strB);
        }

        template<typename StrA>
        size_t indexOfNonNum(std::basic_string_view<StrA> str) {
            for (size_t i = 0; i < str.length(); i++) {
                if (str[i] < '0' || str[i] > '9') { return i; }
            }
            return std::basic_string_view<StrA>::npos;
        }
    }

    enum IntBase : int32_t {
        IBase_Null = 0x00,
        IBase_2 = 0x02,
        IBase_8 = 0x08,
        IBase_10 = 0x0A,
    };

    void formatDataSize(char* buffer, size_t bufSize, double size);

    inline void formatDataSize(char* buffer, size_t bufSize, size_t size) {
        formatDataSize(buffer, bufSize, double(size));
    }

    std::string& appendFormat(std::string& str, const char* format, ...);

    template<size_t BUF_SIZE>
    inline void formatDataSize(char(&buffer)[BUF_SIZE], size_t size) {
        formatDataSize(buffer, BUF_SIZE - 1, double(size));
    }
    template<size_t BUF_SIZE>
    inline void formatDataSize(char(&buffer)[BUF_SIZE], double size) {
        formatDataSize(buffer, BUF_SIZE - 1, size);
    }

    MAKE_STR_TRIM(std::string_view, std::string_view, WHITE_SPACE)
    MAKE_STR_TRIM_(std::string_view, std::wstring_view)
    MAKE_STR_TRIM(std::wstring_view, std::wstring_view, WWHITE_SPACE)
    MAKE_STR_TRIM_(std::wstring_view, std::string_view)

    MAKE_STR_FIND_CH(std::string_view, char)
    MAKE_STR_FIND_CH(std::string_view, wchar_t)
    MAKE_STR_FIND_CH(std::wstring_view, char)
    MAKE_STR_FIND_CH(std::wstring_view, wchar_t)

    MAKE_STR_TO_STR(std::string_view, std::string_view)
    MAKE_STR_TO_STR(std::string_view, std::wstring_view)
    MAKE_STR_TO_STR(std::wstring_view, std::string_view)
    MAKE_STR_TO_STR(std::wstring_view, std::wstring_view)

    MAKE_STR_MISC(std::string_view)
    MAKE_STR_MISC(std::wstring_view)

    inline constexpr uint8_t parseHexChar(char ch) {
        if (ch < '0') { return 0xFF; }
        if (ch < '9') { return uint8_t(ch - '0'); }
        if (ch >= 'a' && ch <= 'f') { return uint8_t(ch - 'a' + 10); }
        return ch >= 'A' ? uint8_t(ch - 'A' + 10) : 0xFF;
    }

    template<typename T>
    int32_t parseIntBase(std::string_view str, T& output, int32_t base = IBase_Null) {
        if (str.length() == 0) { return IBase_Null; }
        if (str.length() < 2) {

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
        size_t i = str.length();
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
    bool tryParseInt(std::string_view str, T& output, int32_t base = IBase_Null, const T& nullVal = T()) {
        base = parseIntBase<T>(str, output, base);
        if (base == IBase_Null) {
            output = nullVal;
            return false;
        }
        return true;
    }

}