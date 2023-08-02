#pragma once
#include <J-Core/IO/Stream.h>

//TODO: Finish writing this bitstream class.

class BitStream : public Stream {
public:

    BitStream();
    BitStream(const Stream& stream);
    ~BitStream();

    void setStream(const Stream* stream) const {
        _stream = stream;

        if (_stream) {
            _capacity = _stream->capacity();
            _length = _stream->size();
            _position = _stream->tell();
            return;
        }

        _capacity = 0;
        _length = 0;
        _position = 0;
    }

    void byteAlign() const;

    const Stream* getStream() const { return _stream; }

    size_t tell() const override { return _bitPos; }

    virtual bool isOpen() const override { return _stream && _stream->isOpen(); }
    virtual size_t read(void* buffer, size_t elementSize, size_t count, const bool bigEndian = false) const override;
    virtual size_t write(const void* buffer, const size_t elementSize, const size_t count, const bool bigEndian = false) const override;

    size_t readBits(void* buffer, size_t bits) const;
    size_t writeBits(const void* buffer, const size_t bits) const;

    virtual bool flush() const override;
    virtual bool close() const override;

    virtual size_t seek(int64_t offset, int origin) const override;

private:
    static constexpr uint32_t BIT_BUFFER_SIZE = 8192;
    static constexpr uint8_t FLAG_NEED_READ = 0x80;

    mutable size_t _bitPos;
    mutable uint8_t _bitBuffer[BIT_BUFFER_SIZE]{0};
    mutable const Stream* _stream;

    void checkBitBuffer() const;
};
