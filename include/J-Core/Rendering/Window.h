#pragma once
#include <J-Core/Rendering/Buffers/FrameBuffer.h>
#include <glm.hpp>
#include <J-Core/Math/Matrix4f.h>
#include <string>
#include <vector>
using DragCB = void(*)(int count, const char** paths);

struct GLFWwindow;
namespace JCore {
    struct GLContext {
        GLFWwindow* window;
        bool visible;

        GLContext();
        ~GLContext();

        void initialize(const char* title, int32_t width, int32_t height, bool isVisible);
        void close();
    };

    class Window {
    public:

        static constexpr uint8_t FLAG_VIEW = 0x01;
        static constexpr uint8_t FLAG_SCISSOR = 0x02;

        Window();
        ~Window();

        static Window* getInstance();

        const Matrix4f& getWorldProjection() const { return _worldProjection; }
        const Matrix4f& getScreenProjection() const { return _screenProjection; }

        const FrameBuffer& getScreenBuffer() const { return _screenBuffer; }
        bool isMinimized() const { return _isMinimized; }

        int32_t getWidth()  const { return _size.x; }
        int32_t getHeight() const { return _size.y; }

        bool initialize(const char* title, uint32_t width = 1280, uint32_t height = 720);
        void close();

        void resetViewport(const uint8_t flags = FLAG_VIEW | FLAG_SCISSOR);
        void updateViewport(const glm::i32vec2& viewRect, const glm::i32vec2& viewSize, const uint8_t flags);

        bool update();
        bool finalizeFrame();

        GLFWwindow* getNativeWindow() { return _context.window; }
        const GLFWwindow* getNativeWindow() const { return _context.window; }

        static const std::vector<std::string>& getBufferedFileDrops() { return _fileDrops; }
        static void clearFileDrops() { _fileDrops.clear(); }
        static void setFileDropCallback(DragCB cb);
        static void setAllowFileDrop(bool value);

    private:
        static Window* Instance;
        static DragCB _dropCB;
        static bool _allowDrop;
        static std::vector<std::string> _fileDrops;
        static uint32_t _fileDropTick;

        GLContext _context;
        FrameBuffer _screenBuffer;

        Matrix4f _worldProjection;
        Matrix4f _screenProjection;
        glm::i32vec2 _size;
        glm::i32vec2 _minSize;
        bool _isMinimized;

        void onWindowResize(const int32_t width, const int32_t height);
        static void windowResizeCallback(GLFWwindow* window, const int32_t width, const int32_t height);
        static void windowDropCallback(GLFWwindow* window, int count, const char** paths);
    };
}