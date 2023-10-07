#pragma once

namespace JCore {
    template<typename T, size_t instance = 0>
    struct EnumNames {
        static constexpr int32_t Start{ 0 };
        static constexpr int32_t Count{ 0 };

        static bool isNoName(const char* name) {
            return name == nullptr || strlen(name) == 0;
        }

        static const char** getEnumNames() {
            return nullptr;
        }

        static const char* getEnumName(T value) {
            size_t index = size_t(value + (Start < 0 ? -Start : 0));
            return getEnumNameByIndex(index);
        }

        static const char* getEnumNameByIndex(size_t index) {
            auto names = getEnumNames();
            if (index >= Count || !names) { return ""; }
            return names[index];
        }


        static bool noDraw(int32_t index);
        static bool getNextValidIndex(int32_t& index, bool ignoreNoDraw = false);
    };

    template<typename T, size_t instance>
    bool EnumNames<T, instance>::noDraw(int32_t index) {
        auto names = getEnumNames();
        if (index < 0 || index >= Count || !names) { return true; }
        return isNoName(names[index]);
    }

    template<typename T, size_t instance>
    inline bool EnumNames<T, instance>::getNextValidIndex(int32_t& index, bool ignoreNoDraw) {
        auto names = getEnumNames();
        if (!names) { return true; }

        int32_t original = index;
        if (index < 0 || index >= Count || (!ignoreNoDraw && noDraw(index))) {
            while (true) {
                index++;
                if (index >= Count) {
                    index = 0;
                }
                if (index == original) { break; }

                if (!noDraw(index)) { break; }
            }
        }
        return index != original;
    }
}

#define ENUM_BIT_OPERATORS(ENUM) \
inline ENUM& operator&=(ENUM& lhs, const ENUM& rhs) { lhs = static_cast<ENUM>(int64_t(lhs) & int64_t(rhs)); return lhs; } \
inline ENUM& operator|=(ENUM& lhs, const ENUM& rhs) { lhs = static_cast<ENUM>(int64_t(lhs) | int64_t(rhs)); return lhs; } \
inline ENUM& operator&=(ENUM& lhs, const int64_t& rhs) { lhs = static_cast<ENUM>(int64_t(lhs) & rhs); return lhs; } \
inline ENUM& operator|=(ENUM& lhs, const int64_t& rhs) { lhs = static_cast<ENUM>(int64_t(lhs) | rhs); return lhs; } \
inline ENUM& operator^=(ENUM& lhs, const ENUM& rhs) { lhs = static_cast<ENUM>(int64_t(lhs) ^ int64_t(rhs)); return lhs; } \
inline ENUM& operator^=(ENUM& lhs, const int64_t& rhs) { lhs = static_cast<ENUM>(int64_t(lhs) ^ rhs); return lhs; } \
inline ENUM operator&(const ENUM& lhs, const ENUM& rhs) { return static_cast<ENUM>(int64_t(lhs) & int64_t(rhs)); } \
inline ENUM operator|(const ENUM& lhs, const ENUM& rhs) { return static_cast<ENUM>(int64_t(lhs) | int64_t(rhs)); } \
inline ENUM operator&(const ENUM& lhs, const int64_t& rhs) { return static_cast<ENUM>(int64_t(lhs) & rhs); } \
inline ENUM operator|(const ENUM& lhs, const int64_t& rhs) { return static_cast<ENUM>(int64_t(lhs) | rhs);} \
inline ENUM operator^(const ENUM& lhs, const ENUM& rhs) { return static_cast<ENUM>(int64_t(lhs) ^ int64_t(rhs));} \
inline ENUM operator^(const ENUM& lhs, const int64_t& rhs) { return static_cast<ENUM>(int64_t(lhs) ^ rhs); } \
inline ENUM operator~(const ENUM& lhs) { return static_cast<ENUM>(~int64_t(lhs)); } \
