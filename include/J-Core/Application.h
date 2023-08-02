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
        Application(const AppSpecs specs);
        virtual ~Application();

        bool initialize();

        void run();
        void close();

        void initFont(const wchar_t* charset, const char* fontpath);
        void initFont(const char* fontpath);

        void toggleFont(const bool state);
       
    protected:
        static Application* _instance;

        ImFont* _font;

        AppSpecs _specs;
        Renderer _renderer;

        std::vector<ImGuiStyle> _styles;

        virtual void start() {}
        virtual void stop(){}

        virtual void doGui();
        virtual void setupStyles(ImGuiStyle& defStyle);

        void setStyle(const int32_t index);
    private:
        void setupStyles();
    };
    Application* createApplication(AppArgs args);
}