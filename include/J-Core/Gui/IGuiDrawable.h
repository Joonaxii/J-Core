#pragma once
#include <imgui.h>
#include <string>

namespace JCore {
    template<typename E>
    struct IGuiMeta {
    public:
        E& getExtraMeta() { return _extra; }
    private:
        E _extra{};
    };

    template<typename T>
    struct IGuiDrawable {
        static bool onGui(const char* label, T& value, const bool doInline = false) {
            return false;
        }

        static bool onGui(T& value, const bool doInline = false) {
            return onGui(nullptr, value, doInline);
        }
    };

    class IGuiPanel {
    public:
        IGuiPanel(const char* title) : _title(title) { }

        const char* getTitleC_Str() const { return _title.c_str(); }
        const std::string& getTitle() const { return _title; }

        virtual void init() {};
        virtual void draw() {};

    private:
        std::string _title;
    };

    namespace Gui {
        template<typename T>
        static bool drawGui(T& value) {
            return IGuiDrawable<T>::onGui(value);
        }

        template<typename T>
        static bool drawGui(const char* label, T& value) {
            return IGuiDrawable<T>::onGui(label, value);
        }

        template<typename T>
        static bool drawGuiInline(T& value) {
            return IGuiDrawable<T>::onGui(value, true);
        }

        template<typename T>
        static bool drawGuiInline(const char* label, T& value) {
            return IGuiDrawable<T>::onGui(label, value, true);
        }
    }

    class IGuiObject {
    public:
        using GuiObjCall = void(*)(IGuiObject* obj);
        static GuiObjCall REGISTER;
        static GuiObjCall UNREGISTER;

        IGuiObject() { if (REGISTER) { REGISTER(this); } }
        virtual ~IGuiObject() { if (UNREGISTER) { UNREGISTER(this); } }
        virtual void drawGuiWindow() = 0;
    };
    inline IGuiObject::GuiObjCall IGuiObject::REGISTER = nullptr;
    inline IGuiObject::GuiObjCall IGuiObject::UNREGISTER = nullptr;
}