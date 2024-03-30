#pragma once
#include <cstdint>
#include <string>
#include <J-Core/Rendering/Renderer.h>
#include <imgui.h>
#include <J-Core/Log.h>
#include <J-Core/Util/EnumUtils.h>

namespace JCore {
    struct AppArgs {
        int32_t count = 0;
        char** args = nullptr;

        const char* operator[](int32_t index) const { return args[index]; }
    };

    struct AppSpecs {
        std::string name = "J-Core Project";
        AppArgs args;
    };

    class Application : public IGui {
    public:
        Application(const AppSpecs& specs);
        virtual ~Application();

        bool initialize();

        void run();
        void close();

        void initFont(const wchar_t* charset, std::string_view fontpath);
        void initFont(std::string_view fontpath);

        void toggleFont(bool state);

    protected:
        static Application* _instance;

        ImFont* _font;

        AppSpecs _specs;
        Renderer _renderer;

        std::vector<ImGuiStyle> _styles;

        bool tryQuit();
        bool isQuitting() const;
        bool canQuit() const;

        virtual void start() {}
        virtual void stop(){}

        virtual void doGui();
        virtual void setupStyles(ImGuiStyle& defStyle);

        void setStyle(const int32_t index);

        virtual int32_t getIconID() const { return 101; }

    private:
        bool _quitting{};
        void setupStyles();
    };
    Application* createApplication(AppArgs args);
}