#include <J-Core/Util/Bitset.h>
#include <J-Core/Math/Math.h>
#include <utility>

namespace JCore {
    static inline void extractIndices(size_t i, size_t& bIndex, size_t& lIndex) {
        size_t nI = i >> 3;
        bIndex = nI;
        lIndex = i - (nI << 3);
    }

    Bitset::Bitset() : Bitset(8) {}
    Bitset::Bitset(size_t bits) : _buffer(nullptr), _bits(0), _capacity(0) {
        resize(bits);
    }

    Bitset::Bitset(const Bitset& other) : Bitset(other._bits) {
        copyFrom(other);
    }
    Bitset::Bitset(Bitset&& other) noexcept :
        _buffer(std::exchange(other._buffer, nullptr)),
        _bits(std::exchange(other._bits, 0)),
        _capacity(std::exchange(other._capacity, 0))
    {
    }

    Bitset::~Bitset() {
        if (_buffer) {
            free(_buffer);
            _buffer = nullptr;
        }
    }

    bool Bitset::operator[](size_t i) const {
        size_t bInd{}, lInd{};
        extractIndices(i, bInd, lInd);
        uint8_t byte = _buffer[bInd];
        return bool(byte & (1 << lInd));
    }

    void Bitset::toString(char* buffer, int32_t width, int32_t height) const {
        for (int32_t y = 0, yP = 0, yPP = 0; y < height; y++, yP += width, yPP += width + 1) {
            for (int32_t x = 0; x < width; x++) {
                buffer[yP + x] = (*this)[size_t(yP) + x] ? '1' : '0';
            }

            buffer[yPP + width] = '\n';
        }
    }

    void Bitset::flip() {
        for (size_t i = 0; i < _capacity; i++) {
            _buffer[i] = ~_buffer[i];
        }
    }
    void Bitset::andWith(const Bitset& other) {
        for (size_t i = 0; i < _capacity; i++) {
            _buffer[i] &= other._buffer[i];
        }
    }
    void Bitset::orWith(const Bitset& other) {
        for (size_t i = 0; i < _capacity; i++) {
            _buffer[i] |= other._buffer[i];
        }
    }
    void Bitset::xorWith(const Bitset& other) {
        for (size_t i = 0; i < _capacity; i++) {
            _buffer[i] ^= other._buffer[i];
        }
    }

    void Bitset::copyFrom(const Bitset& other) {
        resize(other._bits);
        memcpy(_buffer, other._buffer, _capacity);
    }

    void Bitset::set(size_t i, bool value) {
        size_t bInd{}, lInd{};
        extractIndices(i, bInd, lInd);
        uint8_t mask = uint8_t(1 << lInd);

        uint8_t& byte = _buffer[bInd];
        byte = value ? (byte | mask) : (byte & ~mask);
    }

    void Bitset::setAll(bool value) {
        memset(_buffer, value ? 0xFFFFFFFFU : 0x00, _capacity);
    }

    void Bitset::resize(size_t bits) {
        size_t requiredBytes = Math::nextDivByPowOf2<size_t, 8>(bits) >> 3;
        _bits = bits;
        if (_buffer) {
            if (requiredBytes != _capacity) {
                void* reloc = realloc(_buffer, requiredBytes);
                if (reloc) {
                    _buffer = reinterpret_cast<uint8_t*>(reloc);
                    if (_capacity < requiredBytes) {
                        memset(_buffer + _capacity, 0, requiredBytes - int64_t(_capacity));
                    }
                    _capacity = requiredBytes;
                }
            }
            return;
        }

        _buffer = reinterpret_cast<uint8_t*>(malloc(requiredBytes));
        _capacity = requiredBytes;
        if (_buffer) {
            memset(_buffer, 0, _capacity);
        }
    }
}