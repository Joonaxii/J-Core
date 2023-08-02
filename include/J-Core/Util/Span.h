#pragma once
#include <J-Core/Log.h>
#include <J-Core/Util/DataUtilities.h>

namespace JCore {
    template<typename T> 
    class ConstSpan;
    template<typename T>
    class Span {
    public:
        static constexpr size_t npos = SIZE_MAX;

        Span() : _ptr(nullptr), _length() {}
        Span(void* ptr, size_t length) : _ptr(reinterpret_cast<T*>(ptr)), _length(length) {}

        size_t length() const { return _length; }

        const T& operator[](size_t index) const { return *(_ptr + index); }
        T& operator[](size_t index) { return *(_ptr + index); }

        T* get() { return _ptr; }
        const T* get() const { return _ptr; }

        Span<T> slice(int64_t index, size_t length) const {
            JCORE_ASSERT(_length - index >= length, "Invalid index or length!");
            return Span<T>(_ptr + index, length);
        }

        Span<T> slice(int64_t index) const {
            return slice(index, _length - index);
        }

        void copyTo(Span<T>& other) const {
            JCORE_ASSERT(_length <= other._length, "Target Span is too small!");
            memcpy(other._ptr, _ptr, _length * sizeof(T));
        }

        void copyTo(void* other) const {
            memcpy(other, _ptr, _length * sizeof(T));
        }

        bool equals(const ConstSpan<T>& other);
        bool equals(const void* data, size_t size) {
            if (size != (_length * sizeof(T))) { return false; }

            const T* asT = reinterpret_cast<const T*>(data);
            for (size_t i = 0; i < _length; i++) {
                if (asT[i] != _ptr[i]) { return false; }
            }
            return true;
        }

        bool equals(const Span<T>& other) {
            if(_length != other._length) { return false; }
            for (size_t i = 0; i < _length; i++) {
                if (other._ptr[i] != _ptr[i]) { return false; }
            }
            return true;
        }
        size_t indexOf(const T& value) const {
            if (!_ptr) { return npos; }
            for (size_t i = 0; i < _length; i++) {
                if (_ptr[i] == value) { return i; }
            }
            return npos;
        }

        size_t lastIndexOf(const T& value) const {
            if (_length < 1 || !_ptr) { return npos; }
            size_t i = _length;
            while (i > 0)
            {
                if (_ptr[--i] == value) { return i; }
            }
            return npos;
        }

        size_t indexOf(const T* buffer, size_t bufSize) const {
            if (!_ptr) { return npos; }
            size_t j = 0;
            size_t sI = 0;
            for (size_t i = 0; i < _length; i++) {
                if (_ptr[i] == buffer[j++])
                {
                    if (j == 1) {
                        sI = i;
                    }
                    if (j >= bufSize) { return sI; }
                    continue;
                }

                if (j != 0) { j = 0; }
            }
            return npos;
        }

        template<size_t bufSize>
        size_t indexOf(const T(&buffer)[bufSize]) const {
            return indexOf(buffer, bufSize);
        }

        size_t lastIndexOf(const T* buffer, size_t bufSize) const {
            if (_length < 1 || !_ptr) { return npos; }
            size_t i = _length;
            size_t j = bufSize;
            size_t sI = 0;
            while (i > 0)
            {
                if (_ptr[--i] == buffer[--j])
                {
                    if (j <= 0) {
                        return i;
                    }
                    continue;
                }
                if (j != bufSize) { j = bufSize; }
            }
            return npos;
        }

        template<size_t bufSize>
        size_t lastIndexOf(const T(&buffer)[bufSize]) const {
            return lastIndexOf<T>(buffer, bufSize);
        }

        template<typename U>
        U read(const size_t start, const bool isBigEndian = false) const {
            if ((_length - start) < 1 || !_ptr) { return U(); }
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");

            U value = *reinterpret_cast<U*>(_ptr + start);
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        U& read(const size_t start, U& value, const bool isBigEndian = false) const {
            if ((_length - start) < 1 || !_ptr) {
                value = 0;
                return value;
            }
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");
            value = *reinterpret_cast<U*>(_ptr + start);
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        U read(const bool isBigEndian = false) const {
            if (_length < 1 || !_ptr) { return U(); }
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");

            U value = *reinterpret_cast<U*>(_ptr);
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        U& read(U& value, const bool isBigEndian = false) const {
            if (_length < 1 || !_ptr)  {
                value = 0;
                return value;
            }
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");
            value = *reinterpret_cast<U*>(_ptr);
            
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        void writeAt(const size_t start, const U& value, const bool isBigEndian = false) const {
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U), "Span isn't large enough!");
            memcpy(reinterpret_cast<uint8_t*>(_ptr) + start, &value, sizeof(U));

            if (isBigEndian) {
                Data::reverseEndianess(reinterpret_cast<uint8_t*>(_ptr) + start, sizeof(U), 1);
            }
        }  
        
        template<typename U>
        void writeValuesAt(const size_t start, const U& value, size_t count, const bool isBigEndian = false) const {
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U) * count, "Span isn't large enough!");
            std::fill_n(reinterpret_cast<U*>(reinterpret_cast<uint8_t*>(_ptr) + start), count, value);

            if (isBigEndian) {
                Data::reverseEndianess(reinterpret_cast<uint8_t*>(_ptr) + start, sizeof(U), count);
            }
        }

        template<typename U>
        void writeAt(const size_t start, const U* value, const size_t count, const bool isBigEndian = false) const {
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U) * count, "Span isn't large enough!");
            memcpy(reinterpret_cast<uint8_t*>(_ptr) + start, value, sizeof(U) * count);

            if (isBigEndian) {
                Data::reverseEndianess(reinterpret_cast<uint8_t*>(_ptr) + start, sizeof(U), count);
            }
        }

        template<typename U>
        void write(const U& value, const bool isBigEndian = false) const {
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U), "Span isn't large enough!");
            memcpy(_ptr, &value, sizeof(U));

            if (isBigEndian) {
                Data::reverseEndianess(_ptr, sizeof(U), 1);
            }
        }

        template<typename U>
        void write(const U& value, size_t count, const bool isBigEndian = false) const {
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U) * count, "Span isn't large enough!");
            std::fill_n(reinterpret_cast<U*>(_ptr), count, value);

            if (isBigEndian) {
                Data::reverseEndianess(_ptr, sizeof(U), count);
            }
        }

        template<typename U>
        void write(const U* value, const size_t count, const bool isBigEndian = false) const {
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U) * count, "Span isn't large enough!");
            memcpy(_ptr, value, sizeof(U) * count);

            if (isBigEndian) {
                Data::reverseEndianess(_ptr, sizeof(U), count);
            }
        }

    private:
        T* _ptr;
        size_t _length;
    };


    template<typename T>
    class ConstSpan {
    public:
        static constexpr size_t npos = SIZE_MAX;

        ConstSpan() : _ptr(nullptr), _length() {}
        ConstSpan(const void* ptr, size_t length) : _ptr(reinterpret_cast<const T*>(ptr)), _length(length) {}

        size_t length() const { return _length; }

        const T& operator[](size_t index) const { return *(_ptr + index); }
        const T* get() const { return _ptr; }

        ConstSpan<T> slice(int64_t index, size_t length) const {
            JCORE_ASSERT(_length - index >= length, "Invalid index or length!");
            return ConstSpan<T>(_ptr + index, length);
        }

        ConstSpan<T> slice(int64_t index) const {
            return slice(index, _length - index);
        }

        void copyTo(Span<T>& other) const {
            JCORE_ASSERT(_length <= other._length, "Target Span is too small!");
            memcpy(other._ptr, _ptr, _length * sizeof(T));
        }

        void copyTo(void* other) const {
            memcpy(other, _ptr, _length * sizeof(T));
        }

        size_t indexOf(const T& value) const {
            if (!_ptr) { return npos; }
            for (size_t i = 0; i < _length; i++) {
                if (_ptr[i] == value) { return i; }
            }
            return npos;
        }

        size_t lastIndexOf(const T& value) const {
            if (_length < 1 || !_ptr) { return npos; }
            size_t i = _length;
            while (i > 0)
            {
                if (_ptr[--i] == value) { return i; }
            }
            return npos;
        }

        size_t indexOf(const T* buffer, size_t bufSize) const {
            if (!_ptr) { return npos; }
            size_t j = 0;
            size_t sI = 0;
            for (size_t i = 0; i < _length; i++) {
                if (_ptr[i] == buffer[j++])
                {
                    if (j == 1) {
                        sI = i;
                    }
                    if (j >= bufSize) { return sI; }
                    continue;
                }

                if (j != 0) { j = 0; }
            }
            return npos;
        }

        template<size_t bufSize>
        size_t indexOf(const T(&buffer)[bufSize]) const {
            return indexOf(buffer, bufSize);
        }

        size_t lastIndexOf(const T* buffer, size_t bufSize) const {
            if (_length < 1 || !_ptr) { return npos; }
            size_t i = _length;
            size_t j = bufSize;
            size_t sI = 0;
            while (i > 0)
            {
                if (_ptr[--i] == buffer[--j])
                {
                    if (j <= 0) {
                        return i;
                    }
                    continue;
                }

                if (j != bufSize) { j = bufSize; }
            }
            return npos;
        }

        template<size_t bufSize>
        size_t lastIndexOf(const T(&buffer)[bufSize]) const {
            return lastIndexOf<T>(buffer, bufSize);
        }


        bool equals(const Span<T>& other);
        bool equals(const void* data, size_t size) {
            if (size != (_length * sizeof(T))) { return false; }

            const T* asT = reinterpret_cast<const T*>(data);
            for (size_t i = 0; i < _length; i++) {
                if (asT[i] != _ptr[i]) { return false; }
            }
            return true;
        }

        bool equals(const ConstSpan<T>& other) {
            if (_length != other.length()) { return false; }

            for (size_t i = 0; i < _length; i++) {
                if (other._ptr[i] != _ptr[i]) { return false; }
            }
            return true;
        }

        template<typename U>
        U read(const size_t start, const bool isBigEndian = false) const {
            if ((_length - start) < 1 || !_ptr) { return U(); }
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");

            U value = *reinterpret_cast<U*>(_ptr + start);
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        U& read(const size_t start, U& value, const bool isBigEndian = false) const {
            if ((_length - start) < 1 || !_ptr) {
                value = 0;
                return value;
            }
            JCORE_ASSERT((_length - start) * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");
            value = *reinterpret_cast<U*>(_ptr + start);
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        U read(const bool isBigEndian = false) const {
            if (_length < 1 || !_ptr) { return U(); }
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");

            U value = *reinterpret_cast<U*>(_ptr);
            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

        template<typename U>
        U& read(U& value, const bool isBigEndian = false) const {
            if (_length < 1 || !_ptr) {
                value = 0;
                return value;
            }
            JCORE_ASSERT(_length * sizeof(T) >= sizeof(U), "Span doesn't contain enough data!");
            value = *reinterpret_cast<U*>(_ptr);

            if (isBigEndian) {
                Data::reverseEndianess(&value);
            }
            return value;
        }

    private:
        const T* _ptr;
        size_t _length;
    };
}

template<typename T>
inline bool JCore::Span<T>::equals(const JCore::ConstSpan<T>& other) {
    if (_length != other.length()) { return false; }

    for (size_t i = 0; i < _length; i++) {
        if (other[i] != _ptr[i]) { return false; }
    }
    return true;
}

template<typename T>
inline bool JCore::ConstSpan<T>::equals(const JCore::Span<T>& other) {
    if (_length != other.length()) { return false; }

    for (size_t i = 0; i < _length; i++) {
        if (other[i] != _ptr[i]) { return false; }
    }
    return true;
}