#pragma once
#include <J-Core/Util/HexStr.h>
#include <string_view>
#include <cstdint>

#define FROM_BASE(TYPE, VAL) static_cast<TYPE>(VAL) 
#define BASE_TYPE(TYPE) std::underlying_type_t<TYPE>
#define TO_BASE(TYPE, VAL) static_cast<typename BASE_TYPE(TYPE)>(VAL)
#define TO_BASE_REF(TYPE, INPUT) reinterpret_cast<BASE_TYPE(TYPE)&>(INPUT)
#define TO_REF(TYPE, INPUT) reinterpret_cast<TYPE&>(INPUT)
#define NULL_NAME std::string_view{}


namespace JCore {
    namespace detail {
        template<typename T, size_t ID, uint64_t MinValue, uint64_t Count, bool IsBitField>
        struct _Enum {
            static constexpr std::string_view getEnumName(T value, const std::string_view Names[Count]) {
                uint64_t index = (IsBitField ? uint64_t(Math::log2(value)) : uint64_t(value)) - MinValue;
                if (index < 0 || index >= Count) { return std::string_view{}; }
                return Names[index];
            }

            static std::string_view getEnumName_Void(const void* value, const std::string_view Names[Count]) {
                return value ? getEnumName(*reinterpret_cast<const T*>(value), Names) : std::string_view{};
            }

            static constexpr bool isNoName(std::string_view name) {
                return name.length() == 0;
            }
            static constexpr bool noDraw(uint64_t index, const std::string_view Names[Count]) {
                return index >= Count || isNoName(Names[index]);
            }
            static constexpr bool getNextValidIndex(uint64_t& index, const std::string_view Names[Count]) {
                if (Count < 1) { return true; }

                uint64_t original = index;
                while (noDraw(index, Names)) {
                    index++;
                    if (index >= Count) {
                        index = 0;
                    }
                    if (index == original) { break; }
                }
                return index != original;
            }
        };
    }

    template<typename T, size_t ID = 0>
    struct EnumInfo {
        static_assert(std::is_enum<T>::value, "Type is not an enum!");
        static_assert(std::is_unsigned<BASE_TYPE(T)>::value, "Base type of an enum must to be unsigned!");
        static_assert(sizeof(T) <= 8, "Size of enum's base type has to be 8 bytes or less!");

        static constexpr bool IsDefined{ false };
        static constexpr bool IsBitField{ false };
        static constexpr uint64_t MinValue{};
        static constexpr uint64_t Count{ 1 };
        static constexpr std::string_view Names[Count]{ "" };
    };

    namespace Enum {
        typedef std::string_view(*GetEnumName)(const void* value);

        template<typename T>
        constexpr bool isUnsigned() {
            return std::is_enum<T>::value ? std::is_unsigned<BASE_TYPE(T)>::value : std::is_unsigned<T>::value;
        }

        template<typename T, size_t ID = 0>
        constexpr bool isDefinedEnum() {
            return std::is_enum<T>::value && EnumInfo<T, ID>::IsDefined;
        }

        template<typename T, size_t ID = 0>
        FORCE_INLINE std::string_view nameOf_Void(const void* value) {
            return ::JCore::detail::_Enum<T, ID, EnumInfo<T, ID>::MinValue, EnumInfo<T, ID>::Count, EnumInfo<T, ID>::IsBitField>::getEnumName_Void(value, EnumInfo<T, ID>::Names);
        }

        template<typename T, size_t ID = 0>
        FORCE_INLINE constexpr std::string_view nameOf(const T& value) {
            return ::JCore::detail::_Enum<T, ID, EnumInfo<T, ID>::MinValue, EnumInfo<T, ID>::Count, EnumInfo<T, ID>::IsBitField>::getEnumName(value, EnumInfo<T, ID>::Names);
        }

        template<typename T, size_t ID = 0>
        FORCE_INLINE constexpr bool nextValidIndex(size_t& index) {
            return ::JCore::detail::_Enum<T, ID, EnumInfo<T, ID>::MinValue, EnumInfo<T, ID>::Count, EnumInfo<T, ID>::IsBitField>::getNextValidIndex(index, EnumInfo<T, ID>::Names);
        }
    }
}

#define IS_ENUM_DEFINED(ENUM, ID) (::JCore::Enum::isDefinedEnum<ENUM, ID>())

#define DEFINE_ENUM(TYPE, IS_BITFIELD, MIN_VAL, COUNT, ...)\
template<>\
struct ::JCore::EnumInfo<TYPE, 0> { \
    static constexpr bool IsDefined = true;\
    static constexpr bool IsBitField = IS_BITFIELD;\
    static constexpr uint64_t MinValue = uint64_t(MIN_VAL);\
    static constexpr size_t Count = COUNT;\
    static constexpr std::string_view Names[Count] { __VA_ARGS__ };\
};

#define DEFINE_ENUM_ID(TYPE, ID, IS_BITFIELD, MIN_VAL, COUNT, ...)\
template<>\
struct ::JCore::EnumInfo<TYPE, ID> { \
    static constexpr bool IsDefined = true;\
    static constexpr bool IsBitField = IS_BITFIELD;\
    static constexpr uint64_t MinValue = uint64_t(MIN_VAL);\
    static constexpr size_t Count = COUNT;\
    static constexpr std::string_view Names[Count] { __VA_ARGS__ };\
};

#define CREATE_ENUM_OPERATORS_SELF(TYPE) \
FORCE_INLINE constexpr TYPE operator~(TYPE lhs) { return FROM_BASE(TYPE, ~TO_BASE(TYPE, lhs)); } \
FORCE_INLINE constexpr TYPE operator^(TYPE lhs, TYPE rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) ^ TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE operator|(TYPE lhs, TYPE rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) | TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE operator&(TYPE lhs, TYPE rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) & TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE& operator^=(TYPE& lhs, const TYPE& rhs) { return lhs = FROM_BASE(TYPE, TO_BASE(TYPE, lhs) ^ TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE& operator|=(TYPE& lhs, const TYPE& rhs) { return lhs = FROM_BASE(TYPE, TO_BASE(TYPE, lhs) | TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE& operator&=(TYPE& lhs, const TYPE& rhs) { return lhs = FROM_BASE(TYPE, TO_BASE(TYPE, lhs) & TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE operator-(TYPE lhs, TYPE rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) + TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE operator+(TYPE lhs, TYPE rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) - TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE& operator-=(TYPE& lhs, const TYPE& rhs) { return lhs = FROM_BASE(TYPE, TO_BASE(TYPE, lhs) - TO_BASE(TYPE, rhs)); } \
FORCE_INLINE constexpr TYPE& operator+=(TYPE& lhs, const TYPE& rhs) { return lhs = FROM_BASE(TYPE, TO_BASE(TYPE, lhs) + TO_BASE(TYPE, rhs)); } \

#define CREATE_ENUM_OPERATORS(TYPE) \
CREATE_ENUM_OPERATORS_SELF(TYPE)\
FORCE_INLINE constexpr bool operator!=(TYPE lhs, BASE_TYPE(TYPE) rhs) { return TO_BASE(TYPE, lhs) != rhs; } \
FORCE_INLINE constexpr bool operator==(TYPE lhs, BASE_TYPE(TYPE) rhs) { return TO_BASE(TYPE, lhs) == rhs; } \
FORCE_INLINE constexpr bool operator!=(BASE_TYPE(TYPE) lhs,TYPE rhs) { return lhs != TO_BASE(TYPE, rhs); } \
FORCE_INLINE constexpr bool operator==(BASE_TYPE(TYPE) lhs,TYPE rhs) { return lhs == TO_BASE(TYPE, rhs); } \
FORCE_INLINE constexpr TYPE operator^(TYPE lhs, BASE_TYPE(TYPE) rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) ^ rhs); } \
FORCE_INLINE constexpr TYPE operator|(TYPE lhs, BASE_TYPE(TYPE) rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) | rhs); } \
FORCE_INLINE constexpr TYPE operator&(TYPE lhs, BASE_TYPE(TYPE) rhs) { return FROM_BASE(TYPE, TO_BASE(TYPE, lhs) & rhs); } \
FORCE_INLINE constexpr BASE_TYPE(TYPE) operator^(BASE_TYPE(TYPE) lhs, TYPE rhs) { return lhs ^ TO_BASE(TYPE, rhs); } \
FORCE_INLINE constexpr BASE_TYPE(TYPE) operator|(BASE_TYPE(TYPE) lhs, TYPE rhs) { return lhs | TO_BASE(TYPE, rhs); } \
FORCE_INLINE constexpr BASE_TYPE(TYPE) operator&(BASE_TYPE(TYPE) lhs, TYPE rhs) { return lhs & TO_BASE(TYPE, rhs); } \
FORCE_INLINE constexpr bool operator!(TYPE lhs) { return TO_BASE(TYPE, lhs) == 0; } \
FORCE_INLINE TYPE& operator^=(TYPE& lhs, TYPE rhs) { return lhs = lhs ^ rhs; } \
FORCE_INLINE TYPE& operator&=(TYPE& lhs, TYPE rhs) { return lhs = lhs & rhs; } \
FORCE_INLINE TYPE& operator|=(TYPE& lhs, TYPE rhs) { return lhs = lhs | rhs; } \
FORCE_INLINE TYPE& operator^=(TYPE& lhs, BASE_TYPE(TYPE) rhs) { return lhs = lhs ^ FROM_BASE(TYPE, rhs); } \
FORCE_INLINE TYPE& operator&=(TYPE& lhs, BASE_TYPE(TYPE) rhs) { return lhs = lhs & FROM_BASE(TYPE, rhs); } \
FORCE_INLINE TYPE& operator|=(TYPE& lhs, BASE_TYPE(TYPE) rhs) { return lhs = lhs | FROM_BASE(TYPE, rhs); } \
FORCE_INLINE BASE_TYPE(TYPE)& operator^=(BASE_TYPE(TYPE)& lhs, TYPE rhs) { return lhs ^= TO_BASE(TYPE, rhs); } \
FORCE_INLINE BASE_TYPE(TYPE)& operator&=(BASE_TYPE(TYPE)& lhs, TYPE rhs) { return lhs &= TO_BASE(TYPE, rhs); } \
FORCE_INLINE BASE_TYPE(TYPE)& operator|=(BASE_TYPE(TYPE)& lhs, TYPE rhs) { return lhs |= TO_BASE(TYPE, rhs); } 