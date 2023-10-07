#include <J-Core/IO/IOUtils.h>
#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>
#include <J-Core/Util/StringHelpers.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <J-Core/Rendering/Window.h>
#include <J-Core/Log.h>
#include <shellapi.h>

namespace JCore::IO {
    bool hasAnyExtension(const fs::path& path, const char** extensions, size_t count) {
        if (!extensions || count < 1) { return true; }

        if (!path.has_extension()) {
            for (size_t i = 0; i < count; i++) {
                if (strlen(extensions[i]) < 2) { return true; }
            }
            return false;
        }

        auto ext = path.extension();
        std::string_view extV(reinterpret_cast<const char*>(ext.c_str()));
        for (size_t i = 0; i < count; i++) {
            if (Helpers::strIEquals(extV, extensions[i])) {
                return true;
            }
        }
        return false;
    }

    std::string openFile(const char* filter, const size_t maxPathLen, bool allowCreate, bool noValidate, size_t filterTypes) {
        OPENFILENAMEA ofn;
        CHAR* szFile = reinterpret_cast<CHAR*>(_malloca(maxPathLen));

        if (!szFile) { return ""; }

        ZeroMemory(szFile, sizeof(CHAR) * maxPathLen);
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(Window::getInstance()->getNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile  = DWORD(maxPathLen);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = DWORD(filterTypes);
        ofn.Flags = OFN_PATHMUSTEXIST | (allowCreate ? 0 : OFN_FILEMUSTEXIST) | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE) {
            std::string str = ofn.lpstrFile;
            _freea(szFile);
            return str;
        }
        _freea(szFile);
        return std::string();
    }

    std::string openFolder(const char* title, const size_t maxPathLen) {
        CHAR szPath[MAX_PATH]{0};

        BROWSEINFOA pth;
        ZeroMemory(&pth, sizeof(BROWSEINFOA));

        pth.hwndOwner = glfwGetWin32Window(Window::getInstance()->getNativeWindow());
        pth.pszDisplayName = szPath;
        pth.lpszTitle = title;
        pth.ulFlags = BIF_RETURNONLYFSDIRS;

        LPITEMIDLIST pidl = SHBrowseForFolderA(&pth);

        if (pidl != NULL) {
            if (SHGetPathFromIDListA(pidl, szPath)) {
                return szPath;
            }
        }
        return std::string();
    }

    bool getAll(const char* path, size_t length, uint8_t flags, std::vector<FilePath>& paths, bool recursive) {
        fs::path pPath(path, path + length);
        return getAll(pPath, flags, paths, recursive);
    }

    bool getAll(const fs::path& path, uint8_t flags, std::vector<FilePath>& paths, bool recursive) {
        if ((flags & 0x3) == 0) {
            JCORE_WARN("[J-Core - IOUtils] Warning: Search flags are set to 0!");
            return false;
        }
        size_t startC = paths.size();

        if (recursive) {
            for (const auto& dir : fs::recursive_directory_iterator(path)) {
                if ((flags & F_TYPE_FILE) && dir.is_regular_file()) {
                    paths.emplace_back(dir.path(), F_TYPE_FILE);
                    continue;
                }

                if ((flags & F_TYPE_FOLDER) && dir.is_directory()) {
                    paths.emplace_back(dir.path(), F_TYPE_FOLDER);
                }
            }
        }
        else {
            for (const auto& dir : fs::directory_iterator(path)) {
                if ((flags & F_TYPE_FILE) && dir.is_regular_file()) {
                    paths.emplace_back(dir.path(), F_TYPE_FILE);
                    continue;
                }

                if ((flags & F_TYPE_FOLDER) && dir.is_directory()) {
                    paths.emplace_back(dir.path(), F_TYPE_FOLDER);
                }
            }
        }

        return startC != paths.size();
    }
    struct Entry {
        size_t index{ 0 };
        int32_t value{ 0 };
    };

    static bool sortEntries(const Entry& a, const Entry& b) {
        return a.value > b.value;
    }

    void sortByName(std::vector<fs::path>& paths) {
        Entry* entries = reinterpret_cast<Entry*>(_malloca(sizeof(Entry) * paths.size()));
        if (!entries) {
            JCORE_ERROR("[IOUtils] Error: Failed to allocate sorting buffer!");
            return;
        }

        for (size_t i = 0; i < paths.size(); i++) {
            std::string name = paths[i].filename().string();
            ConstSpan<char> spn(name.c_str(), name.length());
            size_t ind = spn.lastIndexOf('_');

            Entry& entry = entries[i];
            entry.index = i;
            entry.value = INT32_MIN;

            int32_t vCount = 0;
            while (ind != ConstSpan<char>::npos) {
                auto tmp = spn.slice(ind + 1);
                tmp = tmp.slice(0, Helpers::indexOfNonNum(tmp.get(), tmp.length()));

                if (Helpers::tryParseInt(tmp, entry.value, Helpers::IBase_10)) {
                    break;
                }
                spn = spn.slice(0, ind);
                ind = spn.lastIndexOf('_');
            }
        }
        std::sort(entries, entries + paths.size(), sortEntries);

        for (size_t i = 0; i < paths.size(); i++) {
            auto& entry = entries[i];
            if (i != entry.index) {
                std::swap(paths[i], paths[entry.index]);
            }
        }
        _freea(entries);
    }

    bool getAllFiles(const char* path, std::vector<fs::path>& paths, bool recursive) {
        size_t init = paths.size();
        if (recursive) {
            for (const auto& dir : fs::recursive_directory_iterator(path)) {
                if (dir.is_regular_file()) {
                    paths.emplace_back(dir.path());
                }
            }
        }
        else {
            for (const auto& dir : fs::directory_iterator(path)) {
                if (dir.is_regular_file()) {
                    paths.emplace_back(dir.path());
                }
            }
        }
        return paths.size() > init;
    }

    bool getAllFilesByExt(const char* path, std::vector<fs::path>& paths, const char* ext, bool recursive) {
        return getAllFilesByExt(path, paths, &ext, 1, recursive);
    }

    static bool containsExt(const fs::path& ext, const char** extensions, size_t extCount) {
        auto str = ext.string();
        for (size_t i = 0; i < extCount; i++) {
            if (Helpers::endsWith(str.c_str(), extensions[i], false)) { return true; }
        }
        return false;
    }

    bool getAllFilesByExt(const char* path, std::vector<fs::path>& paths, const char** ext, size_t extCount, bool recursive) {
        return getAllFilesByExt(fs::path(path), paths, ext, extCount, recursive);
    }

    bool getAllFilesByExt(const fs::path& path, std::vector<fs::path>& paths, const char** ext, size_t extCount, bool recursive) {
        size_t init = paths.size();
        if (recursive) {
            for (const auto& dir : fs::recursive_directory_iterator(path)) {
                if (dir.is_regular_file() && containsExt(dir.path(), ext, extCount)) {
                    paths.emplace_back(dir.path());
                }
            }
        }
        else {
            for (const auto& dir : fs::directory_iterator(path)) {
                if (dir.is_regular_file() && containsExt(dir.path(), ext, extCount)) {
                    paths.emplace_back(dir.path());
                }
            }
        }
        return paths.size() > init;
    }

    bool exists(const char* path) {
        size_t len = strlen(path);
        if (!len) { return false; }

        auto pth = std::filesystem::u8path(path, path + strlen(path));
        return std::filesystem::exists(pth);
    }

    bool exists(const fs::path& path) {
        return std::filesystem::exists(path);
    }

    void createDirectory(const char* path) {
        if (IO::exists(path)) { return; }
        fs::create_directories(fs::path(path));
    }

    bool copyTo(const char* src, const char* dest) {
        return fs::copy_file(fs::path(src), fs::path(dest), fs::copy_options::overwrite_existing);
    }

    bool copyTo(const fs::path& src, const fs::path& dest) {
        return fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    }

    void fixPath(char* path) {
        fixPath(path, strlen(path));
    }

    void fixPath(char* path, size_t size) {
        for (size_t i = 0; i < size; i++) {
            char& c = path[i];
            switch (c) {
                case '\\':
                    c = '/';
                    break;
            }
        }
    }

    void writeString(json& jsonF, const std::string& value) {
        jsonF = value;
    }

    std::string readString(json& jsonF, const std::string& defaultValue) {
        return jsonF.is_string() ? jsonF.get<std::string>() : defaultValue;
    }

    json& getObject(const char* key, json& jsonF) {
        auto& ret = jsonF[key];
        
        if (ret.is_null()) {
            ret = json::object_t();
        }
        return ret;
    }
}