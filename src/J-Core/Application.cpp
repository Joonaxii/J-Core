#include <J-Core/Application.h>
#include <J-Core/IO/IOUtils.h>
#include <J-Core/Gui/IGuiExtras.h>

#include <J-Core/ThreadManager.h>

#include <Windows.h>
#include <io.h>
#define F_OK 0
#define access _access

namespace JCore {
    Application* Application::_instance{ nullptr };

    Application::Application(const AppSpecs specs) : _renderer(), _font(nullptr) {}

    Application::~Application() {
        close();
    }

    bool Application::initialize() {
        if (_instance) {
            JCORE_WARN("[J-Core - Application] Warning: Application already initialized!");
            return true;
        }

        if (_renderer.initialize(_specs.name.c_str())) {
            _instance = this;
            initFont("res/fonts/MPLUS1-Regular.ttf");
            Window::setAllowFileDrop(true);
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
            TaskManager::updateTask();

            if (!_renderer.update()) { break; }
            _renderer.render({}, this);
        }
    }    
    
    void Application::close() {
        stop();
        _renderer.release();
    }

    void Application::initFont(const wchar_t* charset, const char* fontpath) {
        auto& io = ImGui::GetIO();
        io.Fonts->Clear();
        auto defFont = io.Fonts->AddFontDefault();

        JCORE_INFO("Initializing fonts");

        _font = nullptr;
        if (access(fontpath, F_OK) == 0) {
           _font = io.Fonts->AddFontFromFileTTF(fontpath, 16, 0, reinterpret_cast<const ImWchar*>(charset));
           JCORE_INFO("Added font: '{0}'", fontpath);
        }
        else if (fontpath) {
            JCORE_WARN("Couldn't read font '{0}'", fontpath);
        }
        io.Fonts->Build();
    }

    void Application::initFont(const char* fontpath) {
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

    void Application::toggleFont(const bool state) {
        if (!_font) { return; }
        if (state) {
            ImGui::PushFont(_font);
        }
        else { ImGui::PopFont(); }
    }

    void Application::setupStyles()  {
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

    float calculateTaskWindowHeight(const TaskProgress& progress, float* debugProg) {
        float h = 17;
        static constexpr float SEPARATOR_SIZE = 4;
        static constexpr float TEXT_SIZE = 34;
        static constexpr float BAR_SIZE = 16;

        if (progress.message[0]) {
            h += TEXT_SIZE + SEPARATOR_SIZE;
        }
        h += BAR_SIZE;

        if (progress.flags & TaskProgress::HAS_SUB_TASK) {
            if (progress.subMessage[0]) {
                h += TEXT_SIZE + SEPARATOR_SIZE;
            }
            h += BAR_SIZE + SEPARATOR_SIZE;
        }

        if (debugProg) {
            h += ImGui::GetFrameHeight() * ((progress.flags & TaskProgress::HAS_SUB_TASK) ? 2 : 1) + SEPARATOR_SIZE;
        }
        return h + 12;
    }

    void drawTaskWindow(const TaskProgress& progress, float* debugProg) {
        float progM = debugProg ? *debugProg : progress.getProgress();
        float progS = debugProg ? *(debugProg + 1) : progress.subProgress;
        auto& io = ImGui::GetIO();

        float height = calculateTaskWindowHeight(progress, debugProg);

        ImGui::SetNextWindowSize({ 450, height }, 0);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Task Progress##Task", 0, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration);

        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
        static const ImU32 hiLhs = ImGui::ColorConvertFloat4ToU32({ 0.55f, 0.75f, 0.95f, 0.85f });

        float x = ImGui::GetCursorPosX();
        ImGui::Text("[%s]", progress.title);
        Gui::drawProgressSpinner("##SpinnerTask", 12, 4, col);

        if (progress.message[0]) {
            ImGui::SameLine();
            ImGui::Text("%s\n(%.2f %% Done)", progress.message, progM * 100.0f);
            ImGui::Separator();
        }
        Gui::drawProgressBar("##ProgressTask", progM, { 450, 12 }, bg, col, hiLhs);
  
        if (progress.flags & TaskProgress::HAS_SUB_TASK) {
            if (progress.subMessage[0]) {
                ImGui::SetCursorPosX(x);
                ImGui::Separator();
                ImGui::Text("%s\n(%.2f %% Done)", progress.subMessage, progS * 100.0f);
            }
            ImGui::Separator();
            Gui::drawProgressBar("##ProgressTaskSub", progS, { 450, 12 }, bg, col, hiLhs);
        }

        if (debugProg) {
            ImGui::SliderFloat("Progress Override##MAIN", debugProg, 0, 1.0f);
            if (progress.flags & TaskProgress::HAS_SUB_TASK) {
                ImGui::SliderFloat("Sub-Progress Override##SUB", debugProg + 1, 0, 1.0f);
            }
        }

    }

    void Application::doGui() {
        static bool down[2]{false};
        static bool taskWindowOpen{ false };
        static uint8_t flags{ 0 };
        auto& task = TaskManager::getCurrentTask();

        if (GetKeyState(VK_F1) & 0x8000) {
            if (!down[0]) {
                taskWindowOpen = !taskWindowOpen;
                down[0] = true;
            }
        }
        else if (down[0]) { down[0] = false; }

        if (GetKeyState(VK_F2) & 0x8000) {
            if (!down[1]) {
                if (flags & TaskProgress::HAS_SUB_TASK) {
                    flags &= ~TaskProgress::HAS_SUB_TASK;
                }else{
                    flags |= TaskProgress::HAS_SUB_TASK;
                }
                down[1] = true;
            }
        }
        else if (down[1]) { down[1] = false; }

        if (task.isRunning() || taskWindowOpen) {
            static TaskProgress progTest{"Doing Something", "Something Specific", "Even More Specific"};
            progTest.flags = flags;
            auto& curProg = task.isRunning() ? TaskManager::getProgress() : progTest;
            drawTaskWindow(curProg, /*!task.isRunning() && taskWindowOpen ? &progTest.progress :*/ nullptr);

            ImGui::End();
        }

    }

    void Application::setupStyles(ImGuiStyle& defStyle) {}

    void Application::setStyle(const int32_t index) {
        ImGui::GetStyle() = _styles[index];
    }
}
