#pragma once
#include <string>
#include <cstdint>
#include <stdio.h>
#include <J-Core/Util/DataUtilities.h>

class Stream {
public:
    static constexpr uint8_t READ_FLAG = 0x1;
    static constexpr uint8_t WRITE_FLAG = 0x2;

    Stream() : _position(0), _length(0), _flags(0), _capacity(0) {}
    Stream(const uint8_t flags, const size_t length = 0, const size_t capacity = 0) : _position(0), _length(length), _flags(flags), _capacity(capacity) {}
    virtual ~Stream() {}

    bool isEOF() const { return _position >= _length; }
    bool isEOF(size_t length) const { return (_position + length) >= _length; }

    virtual size_t tell() const { return _position; }
    virtual size_t size() const { return _length; }
    virtual size_t capacity() const { return _capacity; }

    virtual bool isOpen() const = 0;
    virtual bool canWrite() const { return bool(_flags & WRITE_FLAG) && isOpen(); }
    virtual bool canRead()  const { return bool(_flags & READ_FLAG)  && isOpen(); }

    virtual size_t read(void* buffer, size_t elementSize, size_t count, const bool bigEndian) const = 0;
    virtual size_t write(const void* buffer, const size_t elementSize, const size_t count, const bool bigEndian = false) const = 0;

    virtual size_t read(void* buffer, size_t size, const bool bigEndian) const {
        return read(buffer, 1, size, bigEndian);
    }

    virtual size_t write(const void* buffer, const size_t size, const bool bigEndian = false) const {
        return write(buffer, 1, size, bigEndian);
    }

    virtual size_t writeZero(const size_t count) const {
        if (!canWrite()) { return 0; }
        void* buffer = _malloca(count);
        if (!buffer) { return 0; }

        memset(buffer, 0, count);
        size_t ret = write(buffer, 1, count);
        _freea(buffer);
        return ret;
    }

    template<typename T>
    size_t readValue(T* data, const size_t count = 1, const bool bigEndian = false) const {
        if (!canRead() || count < 1) { return 0; }
        return read(data, sizeof(T), count, bigEndian);
    }

    template<typename T>
    size_t readValue(T& data, const bool bigEndian) const {
        return this->readValue<T>(&data, 1, bigEndian);
    }

    template<typename T>
    T readValue(const bool bigEndian) const {
        T temp = {};
        this->readValue<T>(&temp, 1, bigEndian);
        return temp;
    }

    template<typename T>
    size_t writeValue(const T& value, const size_t count = 1, const bool bigEndian = false) const {
        if (!canWrite() || count < 1) { return 0; }

        void* buffer = _malloca(count * sizeof(T));
        if (!buffer) { return 0; }

        T valueTemp = value;
        if (bigEndian) {
            JCore::Data::reverseEndianess(&valueTemp, sizeof(T), 1);
        }

        std::fill_n(reinterpret_cast<T*>(buffer), count, valueTemp);
       
        size_t ret = write(buffer, 1, count * sizeof(T));
        _freea(buffer);
        return ret;
    }

    int32_t readCString(char* str, int32_t maxLen) const {
        if (!canRead()) { return 0; }

        if (maxLen < 1) { return 0; }
        maxLen--;
        size_t pos = _position;
        int32_t len = 0;

        char buffer[256]{ 0 };
        while (!isEOF()) {
            size_t bRead = this->read(buffer, 256, false);
            for (size_t i = 0; i < bRead; i++) {
                char c = buffer[i];
                if (c == 0) { goto end; }

                str[len++] = c;
                if (len >= maxLen) { goto end; }
            }
        }
    end:
        seek(pos + len + 1, SEEK_SET);
        str[len] = 0;
        return len;
    }

    std::string readString(int32_t len) const {
        if (!canRead()) { return std::string(); }
        std::string temp(len, 0);
        this->read(temp.data(), len, false);
        return temp;
    }

    virtual bool flush() const = 0;
    virtual bool close() const = 0;

    virtual size_t seek(int64_t offset, int origin) const = 0;
    virtual bool tryReserve(const size_t bytes) const { return true; }

    void copyTo(const Stream& other) const {
        size_t curPos = _position;
        size_t remain = _length - _position;

        if (remain < 1) { return; }

        size_t curPosTgt = other._position;
        size_t remainTgt = other._length - other._position;

        if (remainTgt < remain && !other.tryReserve(((remain - remainTgt) + other._position))) {
            return;
        }

        void* data = _malloca(remain);
        if (!data) { return; }

        this->read(data, remain, false);
        other.write(data, remain);

        _freea(data);
        this->seek(curPos, SEEK_SET);
    }

protected:

    mutable uint8_t _flags;
    mutable size_t _position;
    mutable size_t _length;
    mutable size_t _capacity;
};