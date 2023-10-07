#pragma once
#include <string>
#include <vector>
#include <string>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::basic_json<nlohmann::ordered_map>;
namespace fs = std::filesystem;

namespace JCore::IO {

    enum : uint8_t {
        F_TYPE_FILE = 0x1,
        F_TYPE_FOLDER = 0x2,
        F_TYPE_ALL = F_TYPE_FILE | F_TYPE_FOLDER,
    };

    struct FilePath {
        fs::path path{};
        uint8_t type{ 0 };

        FilePath() : path(), type() {}
        FilePath(const fs::path& path, uint8_t type) : path(path), type(type) {}
    };


    static bool hasAnyExtension(const fs::path& path, const char** extensions, size_t count);

    std::string openFile(const char* filter, const size_t maxPathLen = 260, bool allowCreate = false, bool noValidate = false, size_t filterTypes = 1);
    std::string openFolder(const char* title, const size_t maxPathLen = 260);

    bool getAll(const char* path, size_t length, uint8_t flags, std::vector<FilePath>& paths, bool recursive = false);
    bool getAll(const fs::path& path, uint8_t flags, std::vector<FilePath>& paths, bool recursive = false);

    void sortByName(std::vector<fs::path>& paths);

    bool getAllFiles(const char* path, std::vector<fs::path>& paths, bool recursive = false);
    bool getAllFilesByExt(const char* path, std::vector<fs::path>& paths, const char* ext, bool recursive = false);
    bool getAllFilesByExt(const char* path, std::vector<fs::path>& paths, const char** ext, size_t extCount = 1, bool recursive = false);
    bool getAllFilesByExt(const fs::path& path, std::vector<fs::path>& paths, const char** ext, size_t extCount = 1, bool recursive = false);

    bool exists(const char* path);
    bool exists(const fs::path& path);
    void createDirectory(const char* path);


    bool copyTo(const char* src, const char* dest);
    bool copyTo(const fs::path& src, const fs::path& dest);

    void fixPath(char* path);
    void fixPath(char* path, size_t len);

    void writeString(json& jsonF, const std::string& value);
    std::string readString(json& jsonF, const std::string& defaultValue = "");

    json& getObject(const char* key, json& jsonF);
}