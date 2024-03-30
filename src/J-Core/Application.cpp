#include <J-Core/Application.h>
#include <J-Core/IO/IOUtils.h>
#include <J-Core/Gui/IGuiExtras.h>
#include <J-Core/IO/Image.h>
#include <J-Core/ThreadManager.h>
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

    static float calculateTaskWindowHeight(const TaskProgress& progress) {
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
        return h + 12 + ImGui::GetFrameHeight();
    }

    static void drawTaskWindow(const TaskProgress& tProg, std::shared_ptr<Texture> preview) {
        float progM = tProg.getProgress();
        float progS = tProg.subProgress.getNormalized();
        auto& io = ImGui::GetIO();
        bool isCancelled = TaskManager::isCancelling();

        float height = calculateTaskWindowHeight(tProg);
        bool showTex = preview && preview->isValid();

        float wRatio = showTex ? (preview->getWidth() / float(preview->getHeight())) : 1.0f;

        float texW = showTex ? height * wRatio : 0;
        float winBase = 450;

        ImGui::SetNextWindowSize({ winBase, height }, 0);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Task Progress##Task", 0, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
        static const ImU32 hiLhs = ImGui::ColorConvertFloat4ToU32({ 0.55f, 0.75f, 0.95f, 0.85f });

        float x = ImGui::GetCursorPosX();
        float y = ImGui::GetCursorPosY();
        ImGui::Text(isCancelled ? "Cancelling... [%s]" : "[%s]", tProg.title.c_str());
        Gui::drawProgressSpinner("##SpinnerTask", 12, 4, col);

        if (tProg.message.length() > 1) {
            ImGui::SameLine();

            if (tProg.progress.isRange() && (tProg.progress.type[0] & PROG_ShowRange)) {
                if (tProg.progress.isFloat()) {
                    ImGui::Text("%s\n(%.2f %% Done) [%zi/%zi]", tProg.message.c_str(), progM * 100.0f, tProg.progress.getValueI(0), tProg.progress.getValueI(1));
                }
                else {
                    ImGui::Text("%s\n(%.2f %% Done) [%.0f/%.0f]", tProg.message.c_str(), progM * 100.0f, tProg.progress.getValueF(0), tProg.progress.getValueF(1));
                }
            }
            else {
                ImGui::Text("%s\n(%.2f %% Done)", tProg.message.c_str(), progM * 100.0f);
            }

            ImGui::Separator();
        }
        Gui::drawProgressBar("##ProgressTask", progM, { 450, 12 }, bg, col, hiLhs);

        if (tProg.flags & TaskProgress::HAS_SUB_TASK) {
            if (tProg.subMessage.length() > 0) {
                ImGui::SetCursorPosX(x);
                ImGui::Separator();
                if (tProg.subProgress.isRange() && (tProg.subProgress.type[0] & PROG_ShowRange)) {
                    if (tProg.subProgress.isFloat()) {
                        ImGui::Text("%s\n(%.2f %% Done) [%i/%i]", tProg.subMessage.c_str(), progS * 100.0f, tProg.subProgress.getValueI(0), tProg.subProgress.getValueI(1));
                    }
                    else {
                        ImGui::Text("%s\n(%.2f %% Done) [%.0f/%.0f]", tProg.subMessage.c_str(), progS * 100.0f, tProg.subProgress.getValueF(0), tProg.subProgress.getValueF(1));
                    }
                }
                else {
                    ImGui::Text("%s\n(%.2f %% Done)", tProg.subMessage.c_str(), progS * 100.0f);
                }
            }
            ImGui::Separator();
            Gui::drawProgressBar("##ProgressTaskSub", progS, { 450, 12 }, bg, col, hiLhs);
        }

        ImGui::BeginDisabled(isCancelled);
        if (ImGui::Button("Cancel")) {
            TaskManager::cancelCurTask();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(TaskManager::isSkipping());
        if (ImGui::Button("Skip")) {
            TaskManager::markForSkip();
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
        if (_quitting && TaskManager::getCurrentTask().isRunning()) {
            JCORE_WARN("Waiting on the current task to cancel...");
            return false;
        }

        if (TaskManager::getCurrentTask().isRunning()) {
            JCORE_WARN("Waiting on the current task to cancel...");
            TaskManager::cancelCurTask();
            _quitting = true;
            return false;
        }
        return true;
    }

    bool Application::isQuitting() const { return _quitting; }
    bool Application::canQuit() const { return !TaskManager::getCurrentTask().isRunning(); }

    void Application::doGui() {
        ImGui::BeginDisabled(_quitting);
        auto& task = TaskManager::getCurrentTask();
        if (task.isRunning()) {
            drawTaskWindow(TaskManager::getProgress(), task.showPreview ? task.previewTex : nullptr);
        }
        ImGui::EndDisabled();
    }

    void Application::setupStyles(ImGuiStyle& defStyle) {}

    void Application::setStyle(const int32_t index) {
        ImGui::GetStyle() = _styles[index];
    }
}
