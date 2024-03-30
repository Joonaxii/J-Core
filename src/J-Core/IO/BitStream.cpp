#include <J-Core/IO/BitStream.h>
#include <J-Core/Math/Math.h>
using namespace JCore;

//TODO: Finish writing this bitstream class.

//bool BitStream::isOpen() const
//{
//    return false;
//}

void BitStream::byteAlign() const {
    if (_stream && (_bitPos & 0x7) != 0) {
        uint64_t next = Math::nextDivByPowOf2<uint64_t, 8>(_bitPos);
        //_bitPos = 
    }
}

size_t BitStream::read(void* buffer, size_t elementSize, size_t count, const bool bigEndian) const {
    if (!_stream || !_stream->canRead()) { return 0; }

    byteAlign();
    size_t len = _stream->read(buffer, elementSize, count);
    _position = _stream->tell();
    return len;
}

size_t BitStream::write(const void* buffer, const size_t elementSize, const size_t count, const bool bigEndian) const {
    if (!_stream || !_stream->canWrite()) { return 0; }
    size_t ret = _stream->write(buffer, elementSize, count);
    _position = _stream->tell();
    return ret; 
}

bool BitStream::flush() const {
    return false;
}

bool BitStream::close() const {
    return false;
}

size_t BitStream::seek(int64_t offset, int origin) const {
    switch (origin) {
        case SEEK_CUR:
            _bitPos += offset;

            size_t bufferPos = tell();
            if (bufferPos < _position || bufferPos > _position + BIT_BUFFER_SIZE) {
                _flags |= FLAG_NEED_READ;
            }

            break;
    }

    return _bitPos;
}

void BitStream::checkBitBuffer() const {
    size_t curPos = (_bitPos >> 3);
    if (curPos < _position || curPos > _position + BIT_BUFFER_SIZE) {
        _position = curPos;
        _flags |= FLAG_NEED_READ;
    }

    if (_stream) {
        if (_flags & FLAG_NEED_READ) {
            _stream->seek(_position, SEEK_SET);
            _stream->read(_bitBuffer, BIT_BUFFER_SIZE, false);

            _flags &= ~BIT_BUFFER_SIZE;
        }
    }
}
