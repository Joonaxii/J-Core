#pragma once 
#include <J-Core/IO/Stream.h>

class MemoryStream : public Stream {
public:
    MemoryStream();
    MemoryStream(const size_t capacity);
    MemoryStream(const size_t capacity, const bool isDynamic);

    MemoryStream(const uint8_t* buffer, const size_t length, const size_t capacity);
    MemoryStream(uint8_t* buffer, const size_t length, const size_t capacity);
    ~MemoryStream();

    virtual size_t read(void* buffer, size_t elementSize, size_t count, const bool bigEndian = false) const override;
    virtual size_t write(const void* buffer, const size_t elementSize, const size_t count, const bool bigEndian = false) const override;

    virtual bool flush() const override { return isOpen(); }
    virtual bool close() const override;

    virtual bool isOpen()   const override { return _cBuffer != nullptr; }
    virtual bool canWrite() const override { return Stream::canWrite(); }
    virtual bool canRead()  const override { return Stream::canRead(); }

    virtual size_t seek(int64_t offset, int origin) const override;

    virtual bool tryReserve(const size_t bytes) const override;

protected:
    static constexpr uint8_t DYNAMIC_FLAG = 0x40;
    static constexpr uint8_t OWNING_FLAG = 0x80;

    mutable uint8_t* _buffer;
    mutable const uint8_t* _cBuffer;
};
