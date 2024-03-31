#include <J-Core/Application.h>
#include <J-Core/IO/IOUtils.h>
#include <J-Core/Gui/IGuiExtras.h>
#include <J-Core/IO/Image.h>
#include <J-Core/TaskManager.h>
#include <windows.h>
#include <J-Core/IO/MemoryStream.h>

namespace JCore {
    Application* Application::_instance{ nullptr };

    Application::Application(const AppSpecs& specs) : _specs(specs), _renderer(), _font(nullptr) {}

    Application::~Application() {
        close(); 
    }

    bool Application::initialize() {
        if (_instance) {
            JCORE_WARN("[J-Core - Application] Warning: Application already initialized!");
            return true;
        }

        if (_renderer.initialize(_specs.name.c_str(), 1280, 720, getIconID())) {
            _instance = this;
            Window::setAllowFileDrop(true);
            TaskManager::init();
            setupStyles();
            start();
            return true;
        }

        JCORE_ERROR("[J-Core - Application] Failed to initialize renderer!");
        return false;
    }

    void Application::run() {
        while (true) {
            updateTexGen();
            TaskManager::updateTasks();

            if (!_renderer.update()) 
            { 
                if ((_quitting || tryQuit()) && canQuit()) {
                    break;
                }
            }
            _renderer.render({}, this);
        }
    }

    void Application::close() {
        stop();
        _renderer.release();
        TaskManager::deinit();
    }

    void Application::initFont(const wchar_t* charset, std::string_view fontpath) {
        auto& io = ImGui::GetIO();
        io.Fonts->Clear();
        auto defFont = io.Fonts->AddFontDefault();

        JCORE_INFO("Initializing fonts");

        _font = nullptr;
        if (IO::exists(fontpath) && fs::is_regular_file(fontpath)) {
            char temp[513]{ 0 };
            sprintf_s(temp, "%.*s", int32_t(fontpath.length()), fontpath.data());

            _font = io.Fonts->AddFontFromFileTTF(temp, 16, 0, reinterpret_cast<const ImWchar*>(charset));
            JCORE_INFO("Added font: '{0}'", fontpath);
        }
        else if (fontpath.length() > 0) {
            JCORE_WARN("Couldn't read font '{0}'", fontpath);
        }
        io.Fonts->Build();
    }

    void Application::initFont(std::string_view fontpath) {
        static wchar_t DefaultCharset[]{
            //Same as Full Chinese chars in ImGui
            //Could possibly use a more specialized range
            0x0020, 0x00FF, // Basic Latin + Latin Supplement
            0x2000, 0x206F, // General Punctuation
            0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
            0x31F0, 0x31FF, // Katakana Phonetic Extensions
            0xFF00, 0xFFEF, // Half-width characters
            0xFFFD, 0xFFFD, // Invalid
            0x4e00, 0x9FAF, // CJK Ideograms
            0,
        };
        initFont(DefaultCharset, fontpath);
    }

    void Application::toggleFont(bool state) {
        if (!_font) { return; }
        if (state) {
            ImGui::PushFont(_font);
        }
        else { ImGui::PopFont(); }
    }

    void Application::setupStyles() {
        JCORE_INFO("Setting up GUI styles");

        ImGuiStyle defStyle(ImGui::GetStyle());
        defStyle.Colors[ImGuiCol_Button] = { 0.155f, 0.155f, 0.155f, 1.0f };
        defStyle.Colors[ImGuiCol_WindowBg] = { 0.105f, 0.105f, 0.105f, 1.0f };
        defStyle.Colors[ImGuiCol_ChildBg] = { 0.125f, 0.125f, 0.125f, 1.0f };
        defStyle.Colors[ImGuiCol_Border] = { 0.065f, 0.065f, 0.065f, 1.0f };
        defStyle.Colors[ImGuiCol_FrameBg] = { 0.075f, 0.075f, 0.075f, 1.0f };
        defStyle.Colors[ImGuiCol_FrameBgHovered] = { 0.200f, 0.200f, 0.200f, 1.0f };
        defStyle.Colors[ImGuiCol_FrameBgActive] = { 0.180f, 0.180f, 0.180f, 1.0f };
        defStyle.Colors[ImGuiCol_CheckMark] = { 0.480f, 0.480f, 0.580f, 1.0f };
        defStyle.ChildBorderSize = 4.0f;
        _styles.push_back(defStyle);
        setupStyles(defStyle);
        setStyle(0);
    }

    static float calculateStateHeight(const TaskProgress::State& state) {
        static constexpr float SEPARATOR_SIZE = 4;
        static constexpr float TEXT_SIZE = 34;
        static constexpr float BAR_SIZE = 16;
        float height = 0;
        if (state.users) {
            if (!Utils::isWhiteSpace(state.message)) {
                height += TEXT_SIZE * 2 + SEPARATOR_SIZE;
            }
            height += BAR_SIZE + SEPARATOR_SIZE;
        }
        return height;
    }

    static float calculateTaskWindowHeight(float baseHeight, const std::vector<TaskProgress::State>& states, bool hasButtons) {
        float h = baseHeight + (hasButtons ? ImGui::GetFrameHeight() : 0);
        for (size_t i = 0; i < states.size(); i++) {
            h += calculateStateHeight(states[i]);
        }
        return h;
    }

    static void drawTaskWindow(const Task& task) {
        auto& io = ImGui::GetIO();
        bool isCancelled = TaskManager::isCanceling();
        auto& prog = task.progress;

        bool canCancel  = TaskManager::canCancel();
        bool canSkip    = TaskManager::canSkip();
        std::shared_ptr<Texture> preview = TaskManager::getPreview();

        float height = calculateTaskWindowHeight(48, prog.subStates, canCancel || canSkip);
        bool showTex = preview && preview->isValid();

        float wRatio = showTex ? (preview->getWidth() / float(preview->getHeight())) : 1.0f;
        float texW = showTex ? height * wRatio : 0;
        float winBase = 450;

        ImGui::SetNextWindowSize({ winBase, height }, 0);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Task Progress##Task", 0, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse);

        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
        static const ImU32 hiLhs = ImGui::ColorConvertFloat4ToU32({ 0.55f, 0.75f, 0.95f, 0.85f });

        float x = ImGui::GetCursorPosX();
        float y = ImGui::GetCursorPosY();
        ImGui::Text(isCancelled ? "Canceling... [%s]" : "[%s]", prog.title.c_str());
        Gui::drawProgressSpinner("##SpinnerTask", 12, 4, col);

        float percent = float(prog.getPercent(0));
        if (!Utils::isWhiteSpace(prog.main.message)) {
            ImGui::SameLine();
            ImGui::Text("%s\n(%.2f %% Done)", prog.main.message.c_str(), percent * 100.0f);
            ImGui::Separator();
        }
        Gui::drawProgressBar("##ProgressTask", percent, { 450, 12 }, bg, col, hiLhs);

        int32_t count = 0;
        for (auto& state : prog.subStates) {
            if (state.users == 0) { 
                count++;
                continue; 
            }

            ImGui::Separator();
            ImGui::PushID(count++);
            percent = float(prog.getPercent(uint8_t(count)));
            if (!Utils::isWhiteSpace(state.message)) {
                ImGui::SetCursorPosX(x);
                ImGui::Text("%s\n(%.2f %% Done)", state.message.c_str(), percent * 100.0f);
                ImGui::Separator();
            }
            Gui::drawProgressBar("##ProgressTask", percent, { 450, 12 }, bg, col, hiLhs);
            ImGui::PopID();
        }

        ImGui::BeginDisabled(isCancelled);
        if (canCancel) {
            if (ImGui::Button("Cancel")) {
                TaskManager::cancelCurrentTask();
            }
            ImGui::SameLine();
        }
        ImGui::BeginDisabled(TaskManager::isSkipping());

        if (canSkip) {
            if (ImGui::Button("Skip")) {
                TaskManager::markSkipForTask();
            }
        }
        ImGui::EndDisabled();
        ImGui::EndDisabled();

        auto wPos = ImGui::GetWindowPos();
        ImGui::End();

        if (showTex) {
            ImGui::SetNextWindowPos({ wPos.x + winBase + 5, wPos.y });
            ImGui::SetNextWindowSize({ texW, height });
            ImGui::Begin("##Preview", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
            Gui::drawTexture(preview, 0x00, texW, height, true);
            ImGui::End();
        }
    }

    bool Application::tryQuit() {
        if (_quitting && TaskManager::isRunning()) {
            JCORE_WARN("Waiting on the current task to cancel...");
            return false;
        }

        if (TaskManager::isRunning()) {
            JCORE_WARN("Waiting on the current task to cancel...");
            TaskManager::cancelCurrentTask();
            _quitting = true;
            return false;
        }
        return true;
    }

    bool Application::isQuitting() const { return _quitting; }
    bool Application::canQuit() const { return !TaskManager::isRunning(); }

    void Application::doGui() {
        ImGui::BeginDisabled(_quitting);
        if (TaskManager::isRunning()) {
            drawTaskWindow(TaskManager::get()->getTask());
        }
        ImGui::EndDisabled();
    }

    void Application::setupStyles(ImGuiStyle& defStyle) {}

    void Application::setStyle(const int32_t index) {
        ImGui::GetStyle() = _styles[index];
    }
}
