#include <J-Core/IO/MemoryStream.h>
#include <cassert>

MemoryStream::MemoryStream() : Stream(READ_FLAG | WRITE_FLAG | DYNAMIC_FLAG | OWNING_FLAG), _buffer(nullptr), _cBuffer(nullptr) {}
MemoryStream::MemoryStream(const size_t capacity) : MemoryStream(capacity, false) { }
MemoryStream::MemoryStream(const size_t capacity, const bool isDynamic) : 
    Stream(READ_FLAG | WRITE_FLAG | OWNING_FLAG | (isDynamic ? DYNAMIC_FLAG : 0), 0, capacity), _buffer(nullptr), _cBuffer(nullptr) {

    _buffer = reinterpret_cast<uint8_t*>(malloc(_capacity));
    _cBuffer = _buffer;
}

MemoryStream::MemoryStream(const uint8_t* buffer, const size_t length, const size_t capacity) : Stream(READ_FLAG, length, capacity), _buffer(nullptr), _cBuffer(buffer) {}
MemoryStream::MemoryStream(uint8_t* buffer, const size_t length, const size_t capacity) : Stream(READ_FLAG | WRITE_FLAG, length, capacity), _buffer(buffer), _cBuffer(buffer) {}

MemoryStream::~MemoryStream() { close(); }

size_t MemoryStream::read(void* buffer, size_t elementSize, size_t count, const bool bigEndian) const {
    if (!canRead()) { return 0; }

    size_t size = std::min(elementSize * count, _length - _position);
    memcpy(buffer, _cBuffer + _position, size);
    if (bigEndian) {
        JCore::Data::reverseEndianess(reinterpret_cast<uint8_t*>(buffer), elementSize, count);
    }
    _position += size;
    return size;
}

size_t MemoryStream::write(const void* buffer, const size_t elementSize, const size_t count, const bool bigEndian) const {
    if (!canWrite()) { return 0; }

    size_t size = elementSize * count;
    if (!tryReserve(_position + size)) { return 0; }

    memcpy(_buffer + _position, buffer, size);

    if (bigEndian) {
        JCore::Data::reverseEndianess(_buffer + _position, elementSize, count);
    }
    _position += size;
    return size;
}

bool MemoryStream::close() const {
    if (isOpen()) {
        if (_flags & OWNING_FLAG) {
            free(_buffer);
        }

        _capacity = 0;
        _position = 0;
        _length = 0;
        _buffer = nullptr;
        _cBuffer = nullptr;
        return true;
    }
    return false;
}

size_t MemoryStream::seek(int64_t offset, int origin) const{
    if (isOpen()) {
        switch (origin) {
            case SEEK_CUR:
                _position = offset < 0 && size_t(-offset) > _position ? 0 : _position + offset;
                break;
            case SEEK_SET:
                assert(offset >= 0 && size_t(offset) <= _length && "Invalid seek offset!");
                _position = offset;
                break;
            case SEEK_END:
                assert(offset >= 0 && size_t(offset) <= _length && "Invalid seek offset!");
                _position = _length - offset;
                break;
        }
    }
    return _position;
}

bool MemoryStream::tryReserve(const size_t bytes) const {
    if (isOpen()) {
        if (bytes <= _capacity) { return true; }
        assert((_flags & DYNAMIC_FLAG) && "Cannot expand non-dynamic Memory Stream!");

        _capacity = bytes;

        if (_buffer) {
            void* reLoc = realloc(_buffer, bytes);
            if (!reLoc) { return false; }

            _buffer = reinterpret_cast<uint8_t*>(reLoc);
            _cBuffer = _buffer;
            return _cBuffer != nullptr;
        }

        _buffer = reinterpret_cast<uint8_t*>(malloc(bytes));
        _cBuffer = _buffer;
        return true;
    }
    return false;
}
