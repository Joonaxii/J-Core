#pragma once
#include <J-Core/Rendering/Window.h>
#include <J-Core/Math/Vertex.h>
#include <J-Core/Math/Color32.h>
#include <J-Core/Rendering/Shader.h>
#include <J-Core/Rendering/Buffers/FrameBuffer.h>
#include <J-Core/Rendering/DynamicBatch.h>
#include <set>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/Rendering/Buffers/FrameBufferPool.h>

namespace JCore {

    static constexpr uint32_t textureFormatToGLFormat(const TextureFormat& fmt, const bool isMain = true) {
        switch (fmt)
        {
            default:                       return isMain ? GL_RGBA8 : GL_RGBA;
            case TextureFormat::RGB24:     return isMain ? GL_RGB8 : GL_RGB;

            case TextureFormat::RGB48:     return isMain ? GL_RGB16 : GL_RGB;
            case TextureFormat::RGBA64:    return isMain ? GL_RGBA16 : GL_RGBA;

            case TextureFormat::R8:        return isMain ? GL_R8 : GL_RED;
            case TextureFormat::Indexed8:  return isMain ? GL_R8 : GL_RED;
            case TextureFormat::Indexed16: return isMain ? GL_RG8 : GL_RG;
        }
    }

    static constexpr uint32_t textureFormatToGLType(const TextureFormat& fmt) {
        switch (fmt)
        {
            default:                       return GL_UNSIGNED_BYTE;
            case TextureFormat::RGB48:     
            case TextureFormat::RGBA64:    return GL_UNSIGNED_SHORT;
        }
    }

    static constexpr uint32_t getGLPixelAlignment(const TextureFormat& fmt) {
        switch (fmt)
        {
            default:                        return 4;
            case TextureFormat::RGB48:
            case TextureFormat::Indexed16:  return 2;

            case TextureFormat::R8:
            case TextureFormat::Indexed8:
            case TextureFormat::RGB24:      return 1;
        }
    }

    static constexpr uint32_t getGLPixelAlignment(const uint32_t fmt) {
        switch (fmt)
        {
            default:                    return 4;
            case GL_RED:
            case GL_RGB:
            case GL_RGB8:
            case GL_R8:  return 1;

            case GL_RGB16:
            case GL_RG8:  return 2;
        }
    }

    class IGui {
    public:
        virtual void doGui() = 0;
    };

    class Sprite;
    class Renderable;
    class ICamera;
    class IGuiObject;
    class Renderer {
    public:
        static const uint32_t QUAD_INDICES[6];
        static const Vertex QUAD[4];

        Renderer();
        ~Renderer();

        bool initialize(const char* title, uint32_t width = 1280, uint32_t height = 720);
        void release();

        bool update();
        void render(const std::weak_ptr<ICamera>& camera, IGui* guiDraw);

        Window& getWindow() { return _window; }
        const Window& getWindow() const { return _window; }

        FrameBufferPool& getFrameBufferPool() { return _fbPool; }

        const FrameBuffer& getScreenBuffer() const { return _window.getScreenBuffer(); }
        static Renderer* getInstance() { return Instance; }

        static void addRenderable(Renderable* renderable);
        static void removeRenderable(Renderable* renderable);

    private:
        static Renderer* Instance;

        std::set<Renderable*> _renderables;
        std::vector<Renderable*> _activeRenderers;
        std::vector<IGuiObject*> _activeGuiObjects;

        DynamicBatch _dynamicBatch;
        FrameBufferPool _fbPool;

        Window _window;
        bool _initialized;

        Shader _shaders[3];

        void clear(const Color32& color);

        static void regiserGuiObject(IGuiObject* guiObj);
        static void unregiserGuiObject(IGuiObject* guiObj);
    };
}