#include <J-Core/Rendering/Window.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ext.hpp>
#include <J-Core/Log.h>

namespace JCore {
    Window* Window::Instance{ nullptr };
    DragCB  Window::_dropCB{ nullptr };
    bool  Window::_allowDrop{ false };
    std::vector<std::string>  Window::_fileDrops{ };
    uint32_t  Window::_fileDropTick{ };

    Window::Window() : _context(), _isMinimized(true), _size(), _minSize() { Instance = this; }
    Window::~Window() { Instance = Instance == this ? nullptr : Instance; close(); }

    Window* Window::getInstance() {
        return Instance;
    }

    bool Window::initialize(const char* title, uint32_t width, uint32_t height) {
        if (getNativeWindow()) { return true; }

        _minSize.x = width >> 2;
        _minSize.y = height >> 2;

        //Init GLFW window
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

        _context.initialize(title, width, height, true);
        if (!getNativeWindow()) {
            JCORE_ERROR("Error: Failed to initialize GLFW window!");
            return false;
        }

        glfwMakeContextCurrent(getNativeWindow());

        //Init Glew
        if (glewInit() != GLEW_OK) {
            JCORE_ERROR("Error: Failed to initialize GLEW!");
            glfwTerminate();
            return false;
        }

        glfwSetWindowSizeLimits(getNativeWindow(), _minSize.x, _minSize.y, GLFW_DONT_CARE, GLFW_DONT_CARE);

        //Init window resize event update set screen size related values
        onWindowResize(width, height);

        glfwSetWindowSizeCallback(getNativeWindow(), windowResizeCallback);
        glfwSetDropCallback(getNativeWindow(), windowDropCallback);
        return true;
    }

    void Window::close() {
        if (getNativeWindow() == NULL) { return; }

        _fileDrops.clear();
        _screenBuffer.releaseBuffer();
        _screenBuffer.releaseColorAttatchment();

        glfwSetWindowSizeCallback(getNativeWindow(), NULL);
        _context.close();
    }

    void Window::resetViewport(const uint8_t flags) {
        if (flags & FLAG_VIEW) {
            glViewport(0, 0, _size.x, _size.y);
        }

        if (flags & FLAG_SCISSOR) {
            glScissor(0, 0, _size.x, _size.y);
        }
    }

    void Window::updateViewport(const glm::i32vec2& viewRect, const glm::i32vec2& viewSize, const uint8_t flags) {
        if (flags & FLAG_VIEW) {
            glViewport(viewRect.x, viewRect.y, viewSize.x, viewSize.y);
        }

        if (flags & FLAG_SCISSOR) {
            glScissor(viewRect.x, viewRect.y, viewSize.x, viewSize.y);
        }
    }

    bool Window::update() {
        if (_fileDropTick && _fileDrops.size()) {
            --_fileDropTick;
            if (_fileDropTick == 0) {
                _fileDrops.clear();
            }
        }
        return getNativeWindow() && !glfwWindowShouldClose(getNativeWindow());
    }

    bool Window::finalizeFrame() {
        if (!getNativeWindow()) { return false; }
        glfwPollEvents();
        glfwSwapBuffers(getNativeWindow());
        return true;
    }

    void Window::setFileDropCallback(DragCB cb) {
        _dropCB = cb;
    }

    void Window::setAllowFileDrop(bool value) {
        _allowDrop = value;
    }

    void Window::onWindowResize(const int32_t width, const int32_t height) {
        _size.x = width;
        _size.y = height;

        _isMinimized = width <= 0 || height <= 0;
        const float aspect = _size.x / float(_size.y);
        const float w = 0.5f * aspect;

        _worldProjection = Matrix4f::ortho(-w, w, -0.5f, 0.5f);

        _screenProjection = _worldProjection;
        _screenProjection.translate(-w, -0.5f);
        _screenProjection.scale(aspect, 1.0f);

        FrameBufferSpecs specs{};
        specs.width = width;
        specs.height = height;
        specs.colorFormat = GL_RGBA8;

        _screenBuffer.invalidate(specs);
    }

    void Window::windowResizeCallback(GLFWwindow* window, const int32_t width, const int32_t height) {
        if (Instance == nullptr) { return; }
        if (window == Instance->getNativeWindow()) {
            Instance->onWindowResize(width, height);
        }
    }

    void Window::windowDropCallback(GLFWwindow* window, int count, const char** paths) {
        if (_allowDrop) {
            _fileDrops.clear();
            _fileDrops.reserve(count);
            for (size_t i = 0; i < count; i++) {
                _fileDrops.emplace_back(paths[i]);
            }

            _fileDropTick = _fileDrops.size() > 0 ? 4 : 0;
            if (_dropCB) {
                _dropCB(count, paths);
            }
        }
    }
    GLContext::GLContext() : window(nullptr), visible(false) {
    }

    GLContext::~GLContext() {
        close();
    }


    void GLContext::initialize(const char* title, int32_t width, int32_t height, bool isVisible) {
        visible = isVisible;
        if (!isVisible) {
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        }
        window = glfwCreateWindow(width, height, title, NULL, NULL);
    }

    void GLContext::close() {
        if (window == NULL) { return; }
        glfwDestroyWindow(window);
        window = NULL;
    }
}