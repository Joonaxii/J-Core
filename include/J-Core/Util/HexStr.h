#pragma once
#include <cstdint>
#include <J-Core/Math/Math.h>

#define FORCE_INLINE __forceinline

namespace JCore {
    namespace detail {
        FORCE_INLINE constexpr size_t cLen(const char* input) {
            size_t len = 0;
            while (input && *input != 0) {
                len++;
                input++;
            }
            return len;
        }

        FORCE_INLINE constexpr char toUpperHex(char charV) {
            return charV >= 'a' && charV <= 'f' ? (charV - 32) : charV;
        }

        FORCE_INLINE constexpr uint8_t fromHex(char charV) {
            if (charV >= '0' && charV <= '9') {
                return charV - '0';
            }
            else if (charV >= 'A' && charV <= 'F') {
                return charV - 'A' + 10;
            }
            else if (charV >= 'a' && charV <= 'f') {
                return charV - 'a' + 10;
            }
            return 0;
        }

        FORCE_INLINE constexpr uint8_t fromHex(const char buffer[2]) {
            return (fromHex(buffer[0]) & 0xF) + ((fromHex(buffer[1]) & 0xF) << 4);
        }
    }

    enum UI8StrType : uint8_t {
        HEX_LE = 0,
        HEX_BE = 1,
        RAW_LE = 2,
        RAW_BE = 3,
    };

    template<size_t MAX_LENGTH>
    struct CXPRStr {
    public:
        constexpr CXPRStr() : _length(0), _chars{ 0 }{}

        template<size_t bufSize>
        constexpr CXPRStr(const char(&INPUT)[bufSize]) : StrBase() {
            _length = Math::min(bufSize, MAX_LENGTH);
            for (size_t i = 0; i < _length; i++) {
                _chars[i] = INPUT[i];
            }
            _chars[_length] = 0;
        }
        constexpr CXPRStr(const CXPRStr& other) noexcept : CXPRStr() {
            _length = other._length;
            for (size_t i = 0; i < MAX_LENGTH; i++) {
                _chars[i] = other._chars[i];
            }
            _chars[_length] = 0;
        }

        FORCE_INLINE constexpr CXPRStr& operator=(const CXPRStr& other) {
            _length = other._length;
            for (size_t i = 0; i < MAX_LENGTH; i++) {
                _chars[i] = other._chars[i];
            }
            _chars[_length] = 0;
        }

        FORCE_INLINE constexpr bool isEmpty() const {
            return _length == 0;
        }

        FORCE_INLINE constexpr char operator[](size_t i) const {
            return _chars[i];
        }

        FORCE_INLINE constexpr const char* data() const {
            return _chars;
        }

        FORCE_INLINE constexpr size_t length() const {
            return _length;
        }

    protected:
        size_t _length;
        char _chars[MAX_LENGTH + 1];
    };

    struct StrBase {
    public:
        static constexpr size_t MAX_LENGTH = 32;

        constexpr StrBase() : _length(0), _bytes() {}

        constexpr StrBase(const StrBase& other) noexcept : StrBase() {
            _length = other._length;
            for (size_t i = 0; i < MAX_LENGTH; i++) {
                _bytes[i] = other._bytes[i];
            }
            _bytes[_length] = 0;
        }

        FORCE_INLINE StrBase& operator=(const StrBase& other) {
            _length = other._length;
            for (size_t i = 0; i < MAX_LENGTH; i++) {
                _bytes[i] = other._bytes[i];
            }
            _bytes[_length] = 0;
        }

        FORCE_INLINE constexpr bool isEmpty() const {
            return _length == 0;
        }

        FORCE_INLINE constexpr uint8_t operator[](size_t i) const {
            return _bytes[i];
        }

        FORCE_INLINE const char* c_str() const {
            return reinterpret_cast<const char*>(_bytes);
        }

        FORCE_INLINE constexpr size_t length() const {
            return _length;
        }

        FORCE_INLINE constexpr void getData(uint8_t buffer[MAX_LENGTH + 1]) const {
            for (size_t i = 0; i < _length; i++)
            {
                buffer[i] = _bytes[i];
            }
            buffer[_length] = 0;
        }

        FORCE_INLINE constexpr const uint8_t* data() const {
            return _bytes;
        }

    protected:
        size_t _length;
        uint8_t _bytes[MAX_LENGTH + 1];
    };

    template<UI8StrType TYPE>
    struct UI8Str : public StrBase {
    public:
        constexpr UI8Str() : StrBase() {}

        template<size_t bufSize>
        constexpr UI8Str(const char(&INPUT)[bufSize]) : StrBase()
        {
            char buf[2]{ '0','0' };
            constexpr size_t strLen = bufSize - 1;

            switch (TYPE)
            {
            default:
                _length = Math::min(MAX_LENGTH, (strLen + (strLen & 0x1)) >> 1);
                for (size_t i = 0, j = 0; i < _length; i++) {
                    buf[1] = j < strLen ? INPUT[j++] : '0';
                    buf[0] = j < strLen ? INPUT[j++] : '0';
                    _bytes[i] = (detail::fromHex(buf));
                }
                break;
            case HEX_BE:
                _length = Math::min(MAX_LENGTH, (strLen + (strLen & 0x1)) >> 1);
                for (size_t i = 0, j = 0; i < _length; i++) {
                    buf[1] = j < strLen ? INPUT[j++] : '0';
                    buf[0] = j < strLen ? INPUT[j++] : '0';
                    _bytes[(_length - 1) - i] = (detail::fromHex(buf));
                }
                break;

            case RAW_LE:
                _length = Math::min(MAX_LENGTH, strLen);
                for (size_t i = 0, j = 0; i < _length; i++) {
                    _bytes[i] = uint8_t(INPUT[i]);
                }
                break;
            case RAW_BE:
                _length = Math::min(MAX_LENGTH, strLen);
                for (size_t i = 0, j = 0; i < _length; i++) {
                    _bytes[(_length - 1) - i] = uint8_t(INPUT[i]);
                }
                break;
            }
            _bytes[_length] = 0;
        }
    };

    using HexStrLE = UI8Str<HEX_LE>;
    using HexStrBE = UI8Str<HEX_BE>;

    using RawStrLE = UI8Str<RAW_LE>;
    using RawStrBE = UI8Str<RAW_BE>;

    static constexpr HexStrLE NullHexLE{};
    static constexpr HexStrBE NullHexBE{};

    static constexpr RawStrLE NullRawLE{};
    static constexpr RawStrBE NullRawBE{};
}

#define HEX_LE(x) ::JEngine::HexStrLE(x)
#define HEX_BE(x) ::JEngine::HexStrBE(x)
#define RAW_LE(x) ::JEngine::RawStrLE(x)
#define RAW_BE(x) ::JEngine::RawStrBE(x)