#pragma once
#include <cstdint>

namespace JCore {
    class Bitset {
    public:

        Bitset();
        Bitset(size_t bits);
        Bitset(const Bitset& other);
        Bitset(Bitset&& other) noexcept;
        ~Bitset();

        bool operator[](size_t i) const;

        size_t size() const { return _bits; }

        void copyFrom(const Bitset& other);

        void set(size_t i, bool value);
        void setAll(bool value);
        void resize(size_t bits);

        void flip();
        void andWith(const Bitset& other);
        void orWith(const Bitset& other);
        void xorWith(const Bitset& other);

        void toString(char* buffer, int32_t width, int32_t height) const;

    private:
        uint8_t* _buffer;
        size_t _capacity;
        size_t _bits;
    };
}