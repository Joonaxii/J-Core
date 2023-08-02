#include <J-Core/IO/FileStream.h>
#include <Windows.h>

#include <io.h>
#define F_OK 0
#define access _access

uint8_t getIOFlags(const char* mode) {
    uint8_t io = 0;
    if (mode) {
        for (size_t i = 0; i < strlen(mode); i++) {
            switch (mode[i]) {
                case 'r':
                case 'R': io |= Stream::READ_FLAG; break;
                case 'w':
                case 'W': io |= Stream::WRITE_FLAG; break;
            }
        }
    }
    return io;
}

FileStream::FileStream() : Stream(), _file(nullptr), _filepath() {}
FileStream::FileStream(const char* filepath) :Stream(), _file(nullptr), _filepath(filepath) { }
FileStream::FileStream(const wchar_t* filepath) : Stream(), _file(nullptr), _filepath() {
    if (filepath) {
        int len = WideCharToMultiByte(CP_UTF8, 0, filepath, -1, NULL, 0, NULL, NULL);
        char* temp = reinterpret_cast<char*>(_malloca(len + 1));
        if (temp) {
            temp[len] = 0;
            WideCharToMultiByte(CP_UTF8, 0, filepath, -1, temp, len, NULL, NULL);
            _filepath.reserve(len);
            _filepath.assign(temp);
            _freea(temp);
        }
    }
}

FileStream::FileStream(const char* filepath, const char* mode, const int shared) : Stream(getIOFlags(mode)), _file(nullptr), _filepath() {
    open(filepath, mode, shared);
}

FileStream::FileStream(const wchar_t* filepath, const char* mode, const int shared) : Stream(getIOFlags(mode)), _file(nullptr), _filepath() {
    open(filepath, mode, shared);
}


FileStream::FileStream(const std::string_view& filepath) :Stream(), _file(nullptr), _filepath(filepath) { }
FileStream::FileStream(const std::wstring_view& filepath) : Stream(), _file(nullptr), _filepath() {
    if (filepath.length()) {
        int len = WideCharToMultiByte(CP_UTF8, 0, filepath.data(), int32_t(filepath.length()), NULL, 0, NULL, NULL);
        char* temp = reinterpret_cast<char*>(_malloca(len + 1));
        if (temp) {
            temp[len] = 0;
            WideCharToMultiByte(CP_UTF8, 0, filepath.data(), int32_t(filepath.length()), temp, len, NULL, NULL);
            _filepath.reserve(len);
            _filepath.assign(temp);
            _freea(temp);
        }
    }
}

FileStream::FileStream(const std::string_view& filepath, const char* mode, const int shared) : Stream(getIOFlags(mode)), _file(nullptr), _filepath() {
    open(filepath, mode, shared);
}

FileStream::FileStream(const std::wstring_view& filepath, const char* mode, const int shared) : Stream(getIOFlags(mode)), _file(nullptr), _filepath() {
    open(filepath, mode, shared);
}

FileStream::~FileStream() { close(); }

const std::string& FileStream::getFilePath() const { return _filepath; }

bool FileStream::isOpen() const { return _file != nullptr; }
bool FileStream::open(const char* filepath, const char* mode, const int shared) const {
    if (isOpen()) { return false; }

    _length = 0;
    _capacity = 0;
    _filepath.clear();
    _filepath.append(filepath);

    if (!mode) { return false; }
    return open(mode, shared);
}

bool FileStream::open(const wchar_t* filepath, const char* mode, const int shared) const {
    int len = WideCharToMultiByte(CP_UTF8, 0, filepath, -1, NULL, 0, NULL, NULL);
    char* temp = reinterpret_cast<char*>(_malloca(len + 1));
    if (!temp) { return false; }
    temp[len] = 0;

    WideCharToMultiByte(CP_UTF8, 0, filepath, -1, temp, len, NULL, NULL);
    const bool ret = open(temp, mode);
    _freea(temp);
    return ret;
}

bool FileStream::open(const char* mode, const int shared) const {
    if (isOpen()) { return true; }

    _flags &= uint8_t(~0x3);
    _flags |= getIOFlags(mode);

    const bool isRead = (mode[0] == 'r');
    _length = 0;
    _capacity = 0;
    _position = 0;

    if (isRead) {
        if (access(_filepath.c_str(), F_OK) == 0) {
            fopen_s(&_file, _filepath.c_str(), mode);
            if (_file) {
                _fseeki64_nolock(_file, 0, SEEK_END);
                _length = _ftelli64_nolock(_file);
                _capacity = _length;
                _fseeki64_nolock(_file, 0, SEEK_SET);
                return true;
            }
        }
        return false;
    }

    if (shared) {
        _file = _fsopen(_filepath.c_str(), mode, shared);
        return bool(_file);
    }

    fopen_s(&_file, _filepath.c_str(), mode);
    return bool(_file);
}

bool FileStream::open(const std::wstring_view& filepath, const char* mode, const int shared) const {
    int len = WideCharToMultiByte(CP_UTF8, 0, filepath.data(), int32_t(filepath.length()), NULL, 0, NULL, NULL);
    char* temp = reinterpret_cast<char*>(_malloca(len + 1));
    if (!temp) { return false; }
    temp[len] = 0;

    WideCharToMultiByte(CP_UTF8, 0, filepath.data(), int32_t(filepath.length()), temp, len, NULL, NULL);
    const bool ret = open(temp, mode);
    _freea(temp);
    return ret;
}

bool FileStream::open(const std::string_view& filepath, const char* mode, const int shared) const {
    if (isOpen()) { return false; }

    _length = 0;
    _capacity = 0;
    _filepath.clear();
    _filepath.append(filepath);

    if (!mode) { return false; }
    return open(mode, shared);
}

bool FileStream::flush() const {
    if (!_file || !(_flags & WRITE_FLAG)) { return false; }
    _fflush_nolock(_file);
    return true;
}

bool FileStream::close() const {
    _position = 0;
    _length = 0;
    _capacity = 0;

    if (_file) {
        if (_flags & WRITE_FLAG) {
            _fflush_nolock(_file);
        }
        _fclose_nolock(_file);
        _file = nullptr;
        return true;
    }
    return false;
}


void FileStream::setPath(const char* filepath) {
    close();

    _filepath.clear();
    _filepath.append(filepath);
}
void FileStream::setPath(const wchar_t* filepath) {
    close();

    int len = WideCharToMultiByte(CP_UTF8, 0, filepath, -1, NULL, 0, NULL, NULL);
    char* temp = reinterpret_cast<char*>(_malloca(len + 1));
    if (!temp) { return; }
    temp[len] = 0;

    WideCharToMultiByte(CP_UTF8, 0, filepath, -1, temp, len, NULL, NULL);
    _filepath.clear();
    _filepath.append(temp);
    _freea(temp);
}

void FileStream::setPath(const std::string_view& filepath) {
    close();

    _filepath.clear();
    _filepath.append(filepath.data(), filepath.length());
}
void FileStream::setPath(const std::wstring_view& filepath) {
    close();

    int len = WideCharToMultiByte(CP_UTF8, 0, filepath.data(), int32_t(filepath.length()), NULL, 0, NULL, NULL);
    char* temp = reinterpret_cast<char*>(_malloca(len + 1));
    if (!temp) { return; }
    temp[len] = 0;

    WideCharToMultiByte(CP_UTF8, 0, filepath.data(), int32_t(filepath.length()), temp, len, NULL, NULL);
    _filepath.clear();
    _filepath.append(temp);
    _freea(temp);
}

size_t FileStream::read(void* buffer, size_t elementSize, size_t count, const bool bigEndian) const {
    if (!_file || !canRead()) { return 0; }

    size_t ret = _fread_nolock(buffer, elementSize, count, _file);
    _position += ret * elementSize;
    _position = _position > _length ? _length : _position;

    if (bigEndian) {
        JCore::Data::reverseEndianess(reinterpret_cast<uint8_t*>(buffer), elementSize, count);
    }
    return ret * elementSize;
}

size_t FileStream::write(const void* buffer, size_t elementSize, const size_t count, const bool bigEndian) const {
    if (!_file || !canWrite()) { return 0; }

    size_t size = count * elementSize;
    _position += size;
    _length = _position > _length ? _position : _length;
    _capacity = _length;

    if (bigEndian) {
        uint8_t* tempBuf = reinterpret_cast<uint8_t*>(_malloca(size));

        if (tempBuf) {
            memcpy(tempBuf, buffer, size);
            JCore::Data::reverseEndianess(tempBuf, elementSize, count);
            size_t ret = _fwrite_nolock(tempBuf, elementSize, count, _file);
            _freea(tempBuf);
            return ret * elementSize;
        }
    }
    return _fwrite_nolock(buffer, elementSize, count, _file) * elementSize;
}
size_t FileStream::seek(int64_t offset, int origin) const {
    if (!_file) { return _position; }

    if (origin == SEEK_SET) {
        _position = offset;
        _fseeki64_nolock(_file, offset, SEEK_SET);
        return _position;
    }
    else if (origin == SEEK_CUR) {
        _position += offset;
        _fseeki64_nolock(_file, offset, SEEK_CUR);
        return _position;
    }

    _position = _length - offset;
    _fseeki64_nolock(_file, _position, SEEK_SET);
    return _position;
}