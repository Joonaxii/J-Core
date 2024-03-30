#include <J-Core/IO/IOUtils.h>
#include <Windows.h>
#include <commdlg.h>
#include <shlobj_core.h>
#include <J-Core/Util/StringUtils.h>
#include <J-Core/Math/Math.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <J-Core/Rendering/Window.h>
#include <J-Core/Log.h>
#include <shellapi.h>
#include <winuser.h>

namespace JCore::IO {
    namespace {
        inline bool isDivider(char ch) {
            return ch == '/' || ch == '\\';
        }
        inline char fixChar(char ch, bool caseSensitive = false) {
            if (ch == '\\') { return '/'; }
            return caseSensitive ? ch : char(tolower(ch));
        }

        inline bool isDivider(wchar_t ch) {
            return ch == '/' || ch == '\\';
        }
        inline wchar_t fixChar(wchar_t ch, bool caseSensitive = false) {
            if (ch == '\\') { return '/'; }
            return caseSensitive ? ch : wchar_t(towlower(ch));
        }
    }


    static UINT_PTR CALLBACK openfilename_cb(
        HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        NMHDR* nmhdr;
        OFNOTIFY* ofnotify;
        static char dir_now[MAX_PATH], dir_prev[MAX_PATH];
        switch (msg)
        {
        case WM_NOTIFY:
            ofnotify = (OFNOTIFY*)lParam;
            nmhdr = &ofnotify->hdr;
            switch (nmhdr->code)
            {
            case CDN_FOLDERCHANGE:
                SendMessage(nmhdr->hwndFrom, CDM_GETFOLDERPATH, sizeof(dir_now), (LPARAM)dir_now);
                if (ofnotify->lpOFN->lCustData & 0x3)
                {
                    if ((ofnotify->lpOFN->lCustData & 0x2) && IO::pathsMatch(dir_now, dir_prev))
                    {
                        strncpy_s(ofnotify->lpOFN->lpstrFile, MAX_PATH, dir_now, MAX_PATH);
                        ofnotify->lpOFN->lCustData = 0x4;
                        PostMessage(nmhdr->hwndFrom, WM_COMMAND, IDCANCEL, 0);
                        break;
                    }

                    if (ofnotify->lpOFN->lCustData & 0x1) {
                        ofnotify->lpOFN->lCustData = 0x2;
                    }
                    strncpy_s(dir_prev, sizeof(dir_prev), dir_now, sizeof(dir_now));
                }
                break;
            }
            break;
        }
        return 0;
    }

    std::string openFile(const char* filter, size_t maxPathLen, bool allowCreate, bool noValidate, size_t filterTypes, std::string_view initDir) {
        OPENFILENAMEA ofn;
        CHAR* szFile = reinterpret_cast<CHAR*>(_malloca(maxPathLen));

        if (!szFile) { return ""; }

        ZeroMemory(szFile, sizeof(CHAR) * maxPathLen);
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window(Window::getInstance()->getNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.lpfnHook = noValidate ? openfilename_cb : nullptr;
        ofn.lCustData = noValidate ? 0x1 : 0x00;
        ofn.nMaxFile = DWORD(maxPathLen);
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = DWORD(filterTypes);

        std::string iDir = std::string(IO::getDirectory(initDir));
        IO::unFixPath(iDir.data(), iDir.length());
        if (IO::exists(iDir)) {
            ofn.lpstrInitialDir = iDir.c_str();
        }
        ofn.Flags = OFN_PATHMUSTEXIST | (allowCreate || noValidate ? 0 : OFN_FILEMUSTEXIST) | (noValidate ? OFN_NOVALIDATE | OFN_ENABLESIZING | OFN_ENABLEHOOK | OFN_EXPLORER : 0) | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

        BOOL ret = GetOpenFileNameA(&ofn);
        if (ofn.lCustData == 0x4 || ret == TRUE) {
            std::string str = ofn.lpstrFile;
            _freea(szFile);
            return str;
        }
        _freea(szFile);
        return std::string();
    }

    std::string openFolder(const char* title, size_t maxPathLen, std::string_view initDir) {
        return openFile("Folder\0 \0\0", 260, false, true, 1, initDir);
    }

    bool pathsMatch(std::string_view lhs, std::string_view rhs, bool caseSensitive) {
        lhs = Utils::trimEnd(lhs, "/\\");
        rhs = Utils::trimEnd(rhs, "/\\");
        if (lhs.length() != rhs.length()) {
            return false;
        }

        for (size_t i = 0; i < lhs.length(); i++) {
            if (fixChar(lhs[i], caseSensitive) != fixChar(rhs[i], caseSensitive)) {
                return false;
            }
        }
        return true;
    }

    bool pathsMatch(std::string_view lhs, std::wstring_view rhs, bool caseSensitive) {
        lhs = Utils::trimEnd(lhs, "/\\");
        rhs = Utils::trimEnd(rhs, L"/\\");
        if (lhs.length() != rhs.length()) {
            return false;
        }

        for (size_t i = 0; i < lhs.length(); i++) {
            if (fixChar(lhs[i], caseSensitive) != fixChar(rhs[i], caseSensitive)) {
                return false;
            }
        }
        return true;
    }

    bool getAll(const fs::path& path, uint8_t flags, OnPath callback, bool recursive, CheckPath check) {
        if (!callback) {
            JCORE_WARN("[J-Core - IOUtils] Warning: Search callback is null!");
            return false;
        }

        if ((flags & 0x3) == 0) {
            JCORE_WARN("[J-Core - IOUtils] Warning: Search flags are set to 0!");
            return false;
        }
        size_t found = 0;
        if (recursive) {
            for (const auto& dir : fs::recursive_directory_iterator(path)) {
                if ((flags & F_TYPE_FILE) && dir.is_regular_file() && (!check || check(dir))) {
                    callback(dir.path());
                    found++;
                    continue;
                }

                if ((flags & F_TYPE_FOLDER) && dir.is_directory() && (!check || check(dir))) {
                    callback(dir.path());
                    found++;
                }
            }
        }
        else {
            for (const auto& dir : fs::directory_iterator(path)) {
                if ((flags & F_TYPE_FILE) && dir.is_regular_file() && (!check || check(dir))) {
                    callback(dir.path());
                    found++;
                    continue;
                }

                if ((flags & F_TYPE_FOLDER) && dir.is_directory() && (!check || check(dir))) {
                    callback(dir.path());
                    found++;
                }
            }
        }
        return found > 0;
    }

    bool getAll(const fs::path& path, uint8_t flags, std::vector<fs::path>& paths, bool recursive, CheckPath check) {
        return getAll(path, flags, [&paths](const fs::path& path) { paths.emplace_back(path); }, recursive, check);
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
            std::string_view spn{ name };
            size_t ind = spn.find_last_of('_');

            Entry& entry = entries[i];
            entry.index = i;
            entry.value = INT32_MIN;

            int32_t vCount = 0;
            while (ind != std::string_view::npos) {
                auto tmp = spn.substr(ind + 1);
                tmp = tmp.substr(0, Utils::indexOfNonNum(tmp));

                if (Utils::tryParseInt(tmp, entry.value, Utils::IBase_10)) {
                    break;
                }
                spn = spn.substr(0, ind);
                ind = spn.find_last_of('_');
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

    void moveFile(std::string_view src, std::string_view dst, bool overwrite) {
        if (IO::exists(src) && fs::is_regular_file(src)) {
            if (IO::exists(dst)) {
                if (overwrite) {
                    auto copyOption{ std::filesystem::copy_options::update_existing };
                    std::filesystem::copy_file(src, dst, copyOption);
                    std::filesystem::remove(src);
                }
            }
            else {
                fs::rename(src, dst);
            }
        }
    }

    std::string_view getName(std::string_view path, bool noExtension) {
        size_t ind = path.find_last_of("\\/");
        path = ind != std::string_view::npos ? path.substr(ind + 1) : path;

        if (noExtension) {
            ind = path.find_last_of('.');
            path = ind != std::string_view::npos ? path.substr(0, ind) : path;
        }
        return path;
    }
    std::string_view getDirectory(std::string_view path) {
        size_t ind = path.find_last_of("\\/");
        return ind != std::string_view::npos ? path.substr(0, ind) : std::string_view{};
    }

    std::string combine(std::string_view lhs, std::string_view rhs) {

        if (rhs.size() < 1) {
            return std::string(lhs);
        }

        if (lhs.size() < 1 || fs::path(rhs).is_absolute()) {
            return std::string(rhs);
        }

        std::string str{};
        bool lhsEnds = isDivider(lhs[lhs.size() - 1]);
        bool rhsStart = isDivider(rhs[0]);

        size_t cap = lhs.length() + rhs.length();
        if ((lhsEnds && rhsStart)) {
            str.resize(cap - 1);
            memcpy(str.data(), lhs.data(), lhs.length() - 1);
            memcpy(str.data() + (lhs.length() - 1), rhs.data(), rhs.length());
        }
        else
        {
            size_t pos = 0;
            str.resize(cap + 1);
            memcpy(str.data(), lhs.data(), lhs.length());
            pos += lhs.length();
            if (!lhsEnds && !rhsStart)
            {
                str[pos++] = '/';
            }
            memcpy(str.data() + pos, rhs.data(), rhs.length());
        }
        fixPath(str.data(), str.length());
        return str;
    }

    std::string_view eraseRoot(std::string_view input, std::string_view root, bool requireFull) {
        for (size_t i = 0; i < Math::min(root.size(), input.size()); i++) {
            if (fixChar(input[i]) != fixChar(root[i])) {
                return input;
            }
        }
        return input.substr(requireFull && (root.size() >= input.size()) ? 0 : Math::min(root.size(), input.size()));
    }

    std::string_view eraseRoot(std::string& input, std::string_view root, bool requireFull) {
        std::string_view view = eraseRoot(std::string_view(input), root, requireFull);

        if (view.length() < input.length()) {
            input.erase(0, input.length() - view.length());
        }
        return input;
    }

    bool matchFilter(std::string_view input, const char* filter) {
        if (filter == nullptr || filter[0] == 0) { return false; }

        size_t pos = 0;
        size_t extPos = 0;
        bool isExt = false;
        char prev = filter[pos++];
        while (true) {
            char cur = filter[pos];
            if (prev == 0 && cur == 0) { break; }

            if (cur == 0) {
                if (isExt) {
                    std::string_view view{ filter + extPos, (pos - extPos) };
                    size_t ind = view.find_last_of('.');
                    if (ind != std::string_view::npos) {
                        auto vw = view.substr(ind + 1);
                        if (Utils::endsWith(input, vw)) {
                            return true;
                        }
                    }
                }
                isExt = !isExt;
                if (isExt) {
                    extPos = pos;
                }
            }
            pos++;
            prev = cur;
        }
        return false;
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

    void unFixPath(char* path, size_t len) {
        for (size_t i = 0; i < len; i++) {
            char& c = path[i];
            switch (c) {
            case '/':
                c = '\\';
                break;
            }
        }
    }

    std::string readString(const json& jsonF, const char* key, const std::string& defaultValue) {
        if (key == nullptr || strlen(key) < 1) {
            return jsonF.is_string() ? jsonF.get<std::string>() : defaultValue;
        }

        if (jsonF.contains(key)) {
            auto& field = jsonF[key];
            return field.is_string() ? field.get<std::string>() : defaultValue;
        }
        return defaultValue;
    }

    const json& getObject(const json& jsonF, const char* key) {
        static json NULL_VAL = json(nullptr);
        if (jsonF.contains(key)) {
            return jsonF[key];
        }
        return NULL_VAL;
    }
}