#pragma once
#include <J-Core/IO/Stream.h>

class FileStream : public Stream {
public:
    FileStream();
    FileStream(const char* filepath);
    FileStream(const wchar_t* filepath);
    FileStream(const char* filepath, const char* mode, const int shared = _SH_DENYWR);
    FileStream(const wchar_t* filepath, const char* mode, const int shared = _SH_DENYWR);
    FileStream(const std::string_view& filepath);
    FileStream(const std::wstring_view& filepath);
    FileStream(const std::string_view& filepath, const char* mode, const int shared = _SH_DENYWR);
    FileStream(const std::wstring_view& filepath, const char* mode, const int shared = _SH_DENYWR);
    ~FileStream();

    const std::string& getFilePath() const;

    void setPath(const char* filepath);
    void setPath(const wchar_t* filepath);

    void setPath(const std::string_view& filepath);
    void setPath(const std::wstring_view& filepath);

    bool isOpen() const override;
    bool open(const char* filepath, const char* mode, const int shared = _SH_DENYWR) const;
    bool open(const wchar_t* filepath, const char* mode, const int shared = _SH_DENYWR) const;

    bool open(const std::string_view& filepath, const char* mode, const int shared = _SH_DENYWR) const;
    bool open(const std::wstring_view& filepath, const char* mode, const int shared = _SH_DENYWR) const;

    bool open(const char* mode, const int shared = _SH_DENYWR) const;
    bool flush() const override;
    bool close() const override;

    bool canWrite() const override { return _file != nullptr; }
    bool canRead()  const override { return _file != nullptr; }

    size_t read(void* buffer, size_t elementSize, size_t count, const bool bigEndian = false) const override;
    size_t write(const void* buffer, const size_t elementSize, const size_t count, const bool bigEndian = false) const override;

    size_t seek(int64_t offset, int origin) const override;

private:
    mutable FILE* _file;
    mutable std::string _filepath;
};