#pragma once
#include <string>
#include <vector>
#include <string>
#include <filesystem>
#include <functional>
#include <nlohmann/json.hpp>
#include <J-Core/Util/StringUtils.h>

using json = nlohmann::basic_json<nlohmann::ordered_map>;
namespace fs = std::filesystem;

namespace JCore::IO {
    typedef bool (*CheckPath)(const fs::path& path);
    using OnPath = std::function<void(const fs::path&)>;

    enum : uint8_t {
        F_TYPE_FILE = 0x1,
        F_TYPE_FOLDER = 0x2,
        F_TYPE_ALL = F_TYPE_FILE | F_TYPE_FOLDER,
    };

    namespace detail {
        template<typename PT>
        std::basic_string_view<PT> getExtension(std::basic_string_view<PT> path) {
            size_t ind = Utils::detail::lastIndexOf<PT, char>(path, '.');
            return ind == std::basic_string_view<PT>::npos ? std::basic_string_view<PT>{} : path.substr(ind + 1);
        }
    }

    __forceinline std::string_view getExtension(std::string_view path) {
        return detail::getExtension(path);
    }

    __forceinline std::wstring_view getExtension(std::wstring_view path) {
        return detail::getExtension(path);
    }

    __forceinline std::wstring_view getExtension(const fs::path& path) {
        return detail::getExtension(std::wstring_view(path.c_str()));
    }

    template<typename T, size_t bufSize>
    inline void readBuffer(const json& jsonF, T(&buffer)[bufSize], const T& defVal) {
        if (jsonF.is_array()) {
            for (size_t i = 0; i < bufSize; i++) {
                buffer[i] = i >= jsonF.size() ? defVal : jsonF[i].get<T>();
            }
            return;
        }

        for (size_t i = 0; i < bufSize; i++) {
            buffer[i] = defVal;
        }
    }

    template<typename T, size_t bufSize>
    inline void readBuffer(const json& jsonF, T(&buffer)[bufSize], const T(&defBuffer)[bufSize]) {
        if (jsonF.is_array()) {
            for (size_t i = 0; i < bufSize; i++) {
                buffer[i] = i >= jsonF.size() ? defBuffer[i] : jsonF[i].get<T>();
            }
            return;
        }

        for (size_t i = 0; i < bufSize; i++) {
            buffer[i] = defBuffer[i];
        }
    }

    std::string openFile(const char* filter, size_t maxPathLen = 260, bool allowCreate = false, bool noValidate = false, size_t filterTypes = 1, std::string_view initDir = {});
    std::string openFolder(const char* title, size_t maxPathLen = 260, std::string_view initDir = {});

    bool pathsMatch(std::string_view lhs, std::string_view rhs, bool caseSensitive = false);
    bool pathsMatch(std::string_view lhs, std::wstring_view rhs, bool caseSensitive = false);
    void sortByName(std::vector<fs::path>& paths);

    bool getAll(const fs::path& path, uint8_t flags, OnPath callback, bool recursive = false, CheckPath check = nullptr);
    bool getAll(const fs::path& path, uint8_t flags, std::vector<fs::path>& paths, bool recursive = false, CheckPath check = nullptr);

    inline bool getFiles(const fs::path& path, std::vector<fs::path>& paths, bool recursive = false, CheckPath check = nullptr) {
        return getAll(path, F_TYPE_FILE, paths, recursive, check);
    }
    inline bool getDirectories(const fs::path& path, std::vector<fs::path>& paths, bool recursive = false, CheckPath check = nullptr) {
        return getAll(path, F_TYPE_FOLDER, paths, recursive, check);
    }

    void moveFile(std::string_view src, std::string_view dst, bool overwrite);

    bool exists(const char* path);
    bool exists(const fs::path& path);
    void createDirectory(const char* path);

    bool copyTo(const char* src, const char* dest);
    bool copyTo(const fs::path& src, const fs::path& dest);

    void fixPath(char* path);
    void fixPath(char* path, size_t len);
    void unFixPath(char* path, size_t len);

    std::string readString(const json& jsonF, const char* key, const std::string& defaultValue = "");
    const json& getObject(const json& jsonF, const char* key);

    std::string_view getName(std::string_view path, bool noExtension = false);
    std::string_view getDirectory(std::string_view path);
    std::string combine(std::string_view lhs, std::string_view rhs);

    std::string_view eraseRoot(std::string_view input, std::string_view root, bool requireFull = true);
    std::string_view eraseRoot(std::string& input, std::string_view root, bool requireFull = true);

    bool matchFilter(std::string_view input, const char* filter);

}