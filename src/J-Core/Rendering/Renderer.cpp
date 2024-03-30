#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <J-Core/Gui/ImGui.h>
#include <J-Core/Rendering/Renderer.h>
#include <iostream>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <J-Core/Math/Vertex.h>
#include <J-Core/Math/Rect.h>
#include <J-Core/Rendering/Sprite.h>
#include <J-Core/Rendering/ICamera.h>
#include <J-Core/Gui/IGuiDrawable.h>
#include <J-Core/Rendering/Renderable.h>
#include <J-Core/Log.h>
#include <algorithm>

namespace JCore {
    Renderer* Renderer::Instance{ nullptr };

    struct RenderGroup {
        const Texture* texture { nullptr };
        uint32_t length   { 0 };
        uint32_t vertices { 0 };
        uint32_t indices  { 0 };

        RenderGroup() {}
        RenderGroup(const Texture* texture, const uint32_t length, const uint32_t verts, const uint32_t inds)
            : texture(texture), length(length), vertices(verts), indices(inds) {}
    };

    const uint32_t Renderer::QUAD_INDICES[6]{
              0, 1, 2,
              2, 3, 0
    };

    const Vertex Renderer::QUAD[4]{
        { {0, 0}, Color32(255, 255, 255, 255), {0, 0}},
        { {1, 0}, Color32(255, 255, 255, 255), {1, 0}},
        { {1, 1}, Color32(255, 255, 255, 255), {1, 1}},
        { {0, 1}, Color32(255, 255, 255, 255), {0, 1}},
    };

    const char* DEFAULT_SHADER_VERT = 
    "#version 330 core\n"
    "layout(location = 0) in vec2 vertPos;\n"
    "layout(location = 1) in vec4 vertColor;\n"
    "layout(location = 2) in vec2 vertUV;\n"
    "uniform mat4 _MVP;\n"
    "out vec4 _VertexColor;\n"
    "out vec2 _VertexUV;\n"
    "void main()\n"
    "{\n"
    "    gl_Position = _MVP * vec4(vertPos.x, vertPos.y, 0.0, 1.0);\n"
    "    _VertexColor = vertColor;\n"
    "    _VertexUV = vertUV;\n"
    "}; ";

    const char* DEFAULT_SHADER_FRAG =
    "#version 330 core\n"
    "uniform sampler2D _MainTex;\n"
    "in vec4 _VertexColor;\n"
    "in vec2 _VertexUV;\n"
    "out vec4 _FragColor;\n"
    "void main()\n"
    "{\n"
        "vec4 texCol = texture2D(_MainTex, _VertexUV);\n"
        "_FragColor = texCol * _VertexColor;\n"
        "_FragColor.rgb *= _FragColor.a;\n"
    "}";
    const char* INDEXED_SHADER_FRAG =
    "#version 330 core\n"
    "uniform sampler2D _MainTex;\n"
    "uniform sampler1D _MainTex_Pal;\n"
    "in vec4 _VertexColor;\n"
    "in vec2 _VertexUV;\n"
    "out vec4 _FragColor;\n"
    "void main()\n"
    "{\n"
    "vec4 texU = texture(_MainTex, _VertexUV);\n"
    "vec4 color = texture(_MainTex_Pal, texU.x);\n"
    "_FragColor = color * _VertexColor;\n"
    "_FragColor.rgb *= _FragColor.a;\n"
    "}";

    static void calculateFramebufferRects(
        glm::i32vec2& minSrc, glm::i32vec2& sizeSrc,
        glm::i32vec2& minDst, glm::i32vec2& sizeDst,
        const Rect<float>& viewRect, const Rect<float>& screenRect,
        const size_t viewW, const size_t viewH,
        const size_t screenW, const size_t screenH) {

        glm::vec2 viewSize = viewRect.max;
        viewSize.x *= viewW;
        viewSize.y *= viewH;

        glm::vec2 screenSize = screenRect.max;
        screenSize.x *= screenW;
        screenSize.y *= screenH;

        minSrc.x = int32_t(viewRect.min.x * viewW);
        minSrc.y = int32_t(viewRect.min.y * viewH);

        sizeSrc.x = int32_t(viewSize.x);
        sizeSrc.y = int32_t(viewSize.y);

        minDst.x = int32_t(screenRect.min.x * screenW);
        minDst.y = int32_t(screenRect.min.y * screenH);

        sizeDst.x = int32_t(screenSize.x);
        sizeDst.y = int32_t(screenSize.y);
    }

    static bool sortRenderables(const Renderable* a, const Renderable* b) {
        int32_t comp = a->getSortingLayer().compareTo(b->getSortingLayer());
        if (comp == 0) {
            return a->getTexture() < b->getTexture();
        }
        return comp < 0;
    }

    static int32_t textureToShaderType(const Texture* texture) {
        switch (texture->getFormat())
        {
            default: return 0;
            case TextureFormat::Indexed8:  return 1;
        }
    }

    Renderer::Renderer() : _initialized(false), _window(), _shaders{}, _fbPool() { }

    Renderer::~Renderer() {
        release();
    }

    bool Renderer::initialize(const char* title, uint32_t width, uint32_t height, int32_t icon) {
        if (_initialized) {
            JCORE_WARN("Warning: Renderer already initialized!");
            return true;
        }
        const int32_t glfwErr = glfwInit();
        if (glfwErr == GLFW_FALSE) {
            JCORE_ERROR("Error: Failed to initialize GLFW!");
            terminate();
            return false;
        }

        if (!_window.initialize(title, width, height, icon)) {
            JCORE_ERROR("Error: Failed to initialize window!");
            release();
            return false;
        }

        Instance = this;
        _shaders[0].createShader(DEFAULT_SHADER_VERT, DEFAULT_SHADER_FRAG);
        _shaders[1].createShader(DEFAULT_SHADER_VERT, INDEXED_SHADER_FRAG);
  
        glEnable(GL_BLEND);
        glEnable(GL_SCISSOR_TEST);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendEquation(GL_ADD);

        JCORE_TRACE("OpenGL Version: {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(_window.getNativeWindow(), true);
        ImGui_ImplOpenGL3_Init();

        ImGui::StyleColorsDark();
        ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

        BufferLayout bl{};
        bl.push<float>(2);
        bl.push<uint8_t>(4);
        bl.push<float>(2);

        IGuiObject::REGISTER = regiserGuiObject;
        IGuiObject::UNREGISTER = unregiserGuiObject;

        _dynamicBatch.init(bl);
        _initialized = true;
        return true;
    }

    void Renderer::release() {
        if (!_initialized) { return; }
        if (Instance == this) {
            Instance = nullptr;
        }

        for (size_t i = 0; i < 2; i++)  {
            _shaders[i].release();
        }

        _dynamicBatch.release();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        _window.close();
        glfwTerminate();
        _initialized = false;
    }

    bool Renderer::update() { 
        return _window.update();
    }

    void Renderer::render(const std::weak_ptr<ICamera>& camera, IGui* guiDraw) {
        auto cam = camera.lock();

        if (!_window.isMinimized()) {
            _fbPool.tick();
            if (cam) {
                _activeRenderers.clear();
                for (auto rend : _renderables) {
                    if (rend->canRender()) {
                        _activeRenderers.push_back(rend);
                    }
                }
                std::sort(_activeRenderers.begin(), _activeRenderers.end(), sortRenderables);

                if (_activeRenderers.size() > 0) {
                    std::vector<RenderGroup> groups{};

                    const Texture* texture = _activeRenderers[0]->getTexture().get();
                    uint32_t length = 1;
                    uint32_t verts{ _activeRenderers[0]->getVertexCount()}, inds{ _activeRenderers[0]->getIndexCount()};
                    for (int32_t i = 1; i < _activeRenderers.size(); i++) {
                        auto& rend = _activeRenderers[i];
                        auto tex = rend->getTexture().get();
                        auto vertC = rend->getVertexCount();
                        auto indC = rend->getIndexCount();

                        if (tex != texture || (inds + indC) > DynamicBatch::MAX_INDICES || (verts + indC) > DynamicBatch::MAX_VERTS) {
                            groups.emplace_back(texture, length, verts, inds);
                            texture = tex;
                            length = 1;
                            verts = rend->getVertexCount();
                            inds = rend->getIndexCount();
                            continue;
                        }
                        verts += rend->getVertexCount();
                        inds += rend->getIndexCount();
                        length++;
                    }

                    if (length) {
                        groups.emplace_back(texture, length, verts, inds);
                    }

                    glm::i32vec2 srcMin{};
                    glm::i32vec2 srcSiz{};

                    glm::i32vec2 dstMin{};
                    glm::i32vec2 dstSiz{};
                    const FrameBuffer& buffer = cam->getFrameBuffer() ? *cam->getFrameBuffer() : _window.getScreenBuffer();
                    buffer.refresh();

                    auto& specs = buffer.getSpecs();
                    calculateFramebufferRects(
                        srcMin, srcSiz,
                        dstMin, dstSiz,
                        cam->getViewRect(),
                        cam->getScreenRect(),
                        specs.width, specs.height,
                        specs.width, specs.height);

       
                    buffer.bind();
                    _window.updateViewport(srcMin, srcSiz, 0x3);
                    clear(cam->getClearColor());

                    Matrix4f proj = buffer.getProjectionMatrix();
                    cam->getViewMatrix(proj);

                    for (size_t i = 0; i < 2; i++) {
                        auto& shader = _shaders[i];
                        shader.bind();
                        shader.setUniformMat4f("_MVP", proj);
                    }

                    uint32_t ind = 0;
                   for (auto& grp : groups) {
                       if (grp.texture) {
                           Shader& shader = _shaders[textureToShaderType(grp.texture)];
                           _dynamicBatch.setup(&shader);

                           for (size_t i = 0; i < grp.length; i++) {
                               const Renderable* rend = _activeRenderers[ind++];
                               _dynamicBatch.addVerts(
                                   rend->getWorldMatrix(),rend->getColor(),
                                   rend->getVertices(), rend->getVertexCount(),
                                   rend->getIndices(), rend->getIndexCount());
                           }

                           shader.bind();
                           shader.setTextures("_MainTex", grp.texture, 0);
                           grp.texture->bind(0);
                           _dynamicBatch.drawBatch();
                           continue;
                       }
                       ind += grp.length;
                   }
                }
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            _window.resetViewport();
            clear(Color32::Black);

            ImGui_ImplGlfw_NewFrame();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();

            guiDraw->doGui();

            for (auto& guiO : _activeGuiObjects) {
                guiO->drawGuiWindow();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        _window.finalizeFrame();
    }

    void Renderer::addRenderable(Renderable* renderable) {
        if (Instance && renderable) {
            Instance->_renderables.insert(renderable);
        }
    }

    void Renderer::removeRenderable(Renderable* renderable) {
        if (Instance && renderable) {
            Instance->_renderables.erase(renderable);
        }
    }

    void Renderer::clear(const Color32& color) {
        if (_window.isMinimized()) { return; }
        static constexpr float BYTE_TO_FLOAT = 1.0f / 255.0f;
        glClearColor(
            color.r * BYTE_TO_FLOAT, 
            color.g * BYTE_TO_FLOAT, 
            color.b * BYTE_TO_FLOAT, 
            color.a * BYTE_TO_FLOAT);
       glClear(GL_COLOR_BUFFER_BIT);
    }


    void Renderer::regiserGuiObject(IGuiObject* guiObj) {
        if (guiObj && Instance) {
            Instance->_activeGuiObjects.push_back(guiObj);
        }
    }

    void Renderer::unregiserGuiObject(IGuiObject* guiObj) {
        if (guiObj && Instance) {
            auto find = std::find(Instance->_activeGuiObjects.begin(), Instance->_activeGuiObjects.end(), guiObj);
            if (find != Instance->_activeGuiObjects.end()) {
                Instance->_activeGuiObjects.erase(find);
            }
        }
    }
}