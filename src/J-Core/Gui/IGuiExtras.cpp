#include <J-Core/Gui/IGuiExtras.h>
#include <J-Core/IO/IOUtils.h>
#include <J-Core/Rendering/Buffers/FrameBuffer.h>
#include <gl/glew.h>
#include <J-Core/Math/Vertex.h>
#include <J-Core/Math/Math.h>
#include <J-Core/Log.h>
#include <J-Core/Rendering/Window.h>
#include <J-Core/Rendering/Renderer.h>
#include <J-Core/IO/Image.h>
#include <J-Core/Util/StringHelpers.h>

#include <io.h>
#include <J-Core/IO/FileStream.h>
#define F_OK 0
#define access _access


namespace JCore::Gui {
    bool searchDialogCenter(const char* label, uint8_t flags, char path[513], const char* types, const size_t defaultType) {
        ImGui::BeginGroup();
        ImGui::PushID(label);
        float sqSize = ImGui::GetFrameHeight();
        auto size = ImGui::GetWindowSize();
        float inputW = std::max(size.x - sqSize - 5, 120.0f);
        float totW = inputW + sqSize;

        ImGui::SetCursorPos({ (size.x * 0.5f) - totW * 0.5f , (size.y * 0.5f) - sqSize * 0.5f });
        bool press = ImGui::RadioButton("##Search", true);
        ImGui::SameLine();

        ImGui::PushItemWidth(inputW);
        bool changed = ImGui::InputTextWithHint("##Path", label, path, 512);
        if ((flags & 0x1) && !(ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) && ImGui::GetHoveredID() == ImGui::GetItemID()) {
            auto& allPaths = Window::getBufferedFileDrops();
            if (allPaths.size() > 0) {
                memcpy(path, allPaths[0].c_str(), allPaths[0].length());
                path[allPaths[0].length()] = 0;
                changed |= true;
                Window::clearFileDrops();
            }
        }
        ImGui::PopItemWidth();

        if (press) {
            //Check if we're a folder
            if (types == nullptr) {
                auto str = IO::openFolder("Search folder");
                if (str.length() > 0) {
                    memset(path, 0, 513);
                    memcpy(path, str.c_str(), std::min<size_t>(512, str.length()));
                    changed = true;
                }
            }
            else {
                auto str = IO::openFile(types, false, false, defaultType);
                if (str.length() > 0) {
                    memset(path, 0, 513);
                    memcpy(path, str.c_str(), std::min<size_t>(512, str.length()));
                    changed = true;
                }
            }
        }
        ImGui::PopID();
        ImGui::EndGroup();
        return changed;

        return false;
    }

    int32_t getTextureSaveInfoMode(const std::shared_ptr<Texture>& tex) {
        auto format = tex->getFormat();
        switch (format) {
            case JCore::TextureFormat::Indexed8:
            case JCore::TextureFormat::Indexed16:
                return 0;
            default: return 0;
        }
    }

    bool searchDialogLeft(const char* label, uint8_t flags, char path[513], const char* types, const size_t defaultType) {
        ImGui::BeginGroup();
        ImGui::PushID(label);
        float sqSize = ImGui::GetFrameHeight();
        float width = ImGui::GetWindowSize().x;
        bool press = ImGui::RadioButton("##Search", true);
        ImGui::SameLine();

        ImGui::PushItemWidth(std::max(width - sqSize - 5, 120.0f));
        bool changed = ImGui::InputTextWithHint("##Path", label, path, 512);
        if ((flags & 0x1) && !(ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) && ImGui::GetHoveredID() == ImGui::GetItemID()) {
            auto& allPaths = Window::getBufferedFileDrops();
            if (allPaths.size() > 0) {
                memcpy(path, allPaths[0].c_str(), allPaths[0].length());
                path[allPaths[0].length()] = 0;
                changed |= true;
                Window::clearFileDrops();
            }
        }
        ImGui::PopItemWidth();

        if (press) {
            //Check if we're a folder
            if (types == nullptr) {
                auto str = IO::openFolder("Search folder");
                if (str.length() > 0) {
                    memset(path, 0, 513);
                    memcpy(path, str.c_str(), std::min<size_t>(512, str.length()));
                    changed = true;
                }
            }
            else {
                auto str = IO::openFile(types, 260, false, false, defaultType);
                if (str.length() > 0) {
                    memset(path, 0, 513);
                    memcpy(path, str.c_str(), std::min<size_t>(512, str.length()));
                    changed = true;
                }
            }
        }
        ImGui::PopID();
        ImGui::EndGroup();
        return changed;
    }

    bool drawSplitter(bool splitVertical, float thickness, float* size0, float* size1, float minSize0, float minSize1, float splitterAxisSize) {
        return drawSplitter("##Splitter", splitVertical, thickness, size0, size1, minSize0, minSize1, splitterAxisSize);
    }

    bool drawSplitter(const char* idIn, bool splitVertical, float thickness, float* size0, float* size1, float minSize0, float minSize1, float splitterAxisSize) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        ImGuiID id = window->GetID(idIn);
        ImRect bb;
        bb.Min = window->DC.CursorPos + (splitVertical ? ImVec2(*size0, 0.0f) : ImVec2(0.0f, *size0));
        bb.Max = bb.Min + ImGui::CalcItemSize(splitVertical ? ImVec2(thickness, splitterAxisSize) : ImVec2(splitterAxisSize, thickness), 0.0f, 0.0f);
        return ImGui::SplitterBehavior(bb, id, splitVertical ? ImGuiAxis_X : ImGuiAxis_Y, size0, size1, minSize0, minSize1, 0.0f);
    }

    void drawTexture(uint32_t texture, int32_t width, int32_t height, float sizeX, float sizeY, bool keepAspect, float edge) {
        void* texPtr = (void*)size_t(texture);
        if (keepAspect) {
            float size = 1.0f - edge;
            float aspectA = width / float(height);
            ImGui::Image(texPtr, { sizeX * aspectA * size, sizeY * size });
            return;
        }
        ImGui::Image(texPtr, { sizeX, sizeY });
    }

    static void drawTextureInfoGui(std::shared_ptr<Texture>& texture, const ImVec2& size, const ImVec2& frameSize) {
        if (!texture) { return; }
        ImGui::BeginChildFrame(ImGui::GetID("Textxure Info"), frameSize, ImGuiWindowFlags_NoDecoration);

        ImGui::SetCursorPosY(frameSize.y * 0.5f - (size.y * 0.5f));

        int32_t width = texture->getWidth();
        int32_t height = texture->getHeight();
        int32_t paletteSize = texture->getPaletteSize();
        TextureFormat fmt = texture->getFormat();

        ImGui::Text(" - Resolution   : %ix%i", width, height);
        ImGui::Text(" - Format       : %s", getTextureFormatName(fmt));
        ImGui::Text(" - Hash         : 0x%08X", texture->getHash());

        switch (fmt) {
            case JCore::TextureFormat::Indexed8:
            case JCore::TextureFormat::Indexed16:
                ImGui::Text(" - Palette Size : %i", texture->getPaletteSize());
                break;
        }

        char tempBuf[256]{ 0 };
        Helpers::formatDataSize(tempBuf, calculateTextureSize(texture->getWidth(), texture->getHeight(), texture->getFormat(), texture->getPaletteSize()));
        ImGui::Text(" - Memory Usage : %s", tempBuf);
        ImGui::EndChildFrame();
    }

    void drawTexture(std::shared_ptr<Texture>& texture, uint32_t flags, float sizeX, float sizeY, bool keepAspect, float edge, uint8_t* extraFlags, uint64_t* overrideId, uint32_t* overrideHash, Color32* bgColor) {
        if (!texture || !texture->isValid()) {
            return;
        }

        uint32_t infoLvl = flags & GUI_TEX_INFO_MASK;

        TextureFormat fmt = texture->getFormat();
        bool isIndexed = fmt == TextureFormat::Indexed8 || fmt == TextureFormat::Indexed16;

        bool newInstance = false;
        const FrameBuffer* paletteBuf = isIndexed ? Renderer::getInstance()->getFrameBufferPool().retrieve(overrideId ? *overrideId : uint64_t(texture.get()), newInstance) : nullptr;
        static uint32_t prevTexHash = 0;

        uint32_t main = texture->getTextureId();
        uint32_t pale = texture->getPaletteId();

        int32_t width = texture->getWidth();
        int32_t height = texture->getHeight();

        float offX = 0;
        float offY = 0;

        static bool blendIndexed = false;
        bool blending = extraFlags ? (*extraFlags & 0x1) : blendIndexed;

        float cPX = ImGui::GetCursorPosX();

        ImGui::PushID(texture.get());
        bool blendPalette = (flags & GUI_TEX_SHOW_EXTRA) ? blending : (flags & GUI_TEX_BLEND_INDEXED);
        bool useFB = (isIndexed && blendPalette) || texture->getFormat() == TextureFormat::Indexed8;

        bool showPalette = (flags & GUI_TEX_SPLIT_INDEXED) && isIndexed && !blendPalette;
        float infoOffsetX = 0;
        float infoOffsetY = 0;

        static constexpr float BUFFER_INFO = 20;
        static constexpr float WIDTH_INFO = 275;

        float infoHeight = ImGui::GetTextLineHeight() * (isIndexed ? 5 : 4);

        switch (infoLvl) {
            case GUI_TEX_INFO_ABOVE:
            case GUI_TEX_INFO_BELOW:
                infoOffsetY = infoHeight + BUFFER_INFO + 20;
                break;
            case GUI_TEX_INFO_SIDE:
                infoOffsetX = WIDTH_INFO + BUFFER_INFO;
                break;
        }

        if (keepAspect) {
            float size = 1.0f - edge;
            float aspectA = 0;

            aspectA = width / float(height);
            sizeX *= aspectA * size;
            sizeY *= size;

            auto avail = ImGui::GetContentRegionAvail();
            avail.x -= 25 + infoOffsetX;
            if (sizeX > avail.x) {
                aspectA = height / float(width);
                sizeX = avail.x;
                sizeY = avail.x * aspectA;
            }
        }

        float paletteRegionSize = 0;
        if (showPalette) {
            switch (fmt) {
                case JCore::TextureFormat::Indexed8:
                    paletteRegionSize = sizeY * 0.25f;
                    break;
                case JCore::TextureFormat::Indexed16:
                    paletteRegionSize = sizeY * (std::max(0.25f, (texture->getPaletteSize() >> 8) / 256.0f));
                    break;
            }
        }

        float dummyHeight = sizeY + paletteRegionSize + ((flags & GUI_TEX_SHOW_EXTRA) ? ImGui::GetFrameHeight() + 4 : 0);
        float rawH = dummyHeight + infoOffsetY;
        if (bgColor) {
            static constexpr float BYTE_TO_FLOAT = 1.0f / 255.0f;
            auto clr = ImVec4(bgColor->r * BYTE_TO_FLOAT, bgColor->g * BYTE_TO_FLOAT, bgColor->b * BYTE_TO_FLOAT, bgColor->a * BYTE_TO_FLOAT);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, clr);
        }
        ImGui::BeginChildFrame(ImGui::GetID("##TextureFrame"), { sizeX + infoOffsetX, rawH + (isIndexed ? 36 : 12) }, ImGuiWindowFlags_NoDecoration);

        if (bgColor) {
            ImGui::PopStyleColor();
        }

        if (flags & GUI_TEX_SHOW_EXTRA) {
            if (isIndexed) {
                if (extraFlags) {
                    bool bit = *extraFlags & 0x1;
                    ImGui::Checkbox("Blend Indexed##Texture Extra", &bit);
                    *extraFlags = (*extraFlags & ~0x1) | (bit ? 0x1 : 0x0);
                    blending = bit;
                }
                else
                {
                    ImGui::Checkbox("Blend Indexed##Texture Extra", &blendIndexed);
                    blending = blendIndexed;
                }
            }

            bool pressed = ImGui::Button("Export##Texture Extra");
            ImGui::SameLine();
            static int32_t compression = 5;
            ImGui::SliderInt("Compression##TextureExtra", &compression, 0, 9);

            if (pressed) {
                std::string path = IO::openFile("PNG File (*.png)\0*.png", 260, true);
                ImageData data{ width, height, fmt, texture->getPaletteSize(), texture->getPixels() };
                Png::encode(path.c_str(), data, compression);
                data.clear(true);
            }
            ImGui::Separator();
        }

        float cY = ImGui::GetCursorPosY();
        ImVec2 infoSize = { float(WIDTH_INFO) + BUFFER_INFO - 25.0f,  float(infoHeight) + BUFFER_INFO };
        switch (infoLvl)
        {
            default:
                ImGui::SetCursorPos({ 0, cY });
                break;
            case GUI_TEX_INFO_ABOVE:
                ImGui::SetCursorPos({0, cY });
                drawTextureInfoGui(texture, infoSize, { sizeX , infoSize.y });
                ImGui::Separator();
                ImGui::SetCursorPos({ 0, cY });
                break;

            case GUI_TEX_INFO_SIDE: {
                ImGui::SetCursorPos({ sizeX + BUFFER_INFO, cY });
                ImGuiWindow* window = ImGui::GetCurrentWindow();
                auto curL = window->DC.CurrLineSize;
                window->DC.CurrLineSize.y = rawH - 22;

                ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
                window->DC.CurrLineSize = curL;
                ImGui::SameLine();
                drawTextureInfoGui(texture, infoSize, infoSize);
                break; 
            }

            case GUI_TEX_INFO_BELOW:
                ImGui::SetCursorPos({ 0, dummyHeight + BUFFER_INFO });
                ImGui::Separator();
                ImGui::SetCursorPosX(0);
                drawTextureInfoGui(texture, infoSize, { sizeX , infoSize.y });
                ImGui::SetCursorPosX(0);
                break;
        }

        ImGui::SetCursorPos({ 0, cY });
        ImGui::Image((void*)size_t((blendPalette && isIndexed && paletteBuf) ? paletteBuf->getColorAttatchment() : main), { sizeX, sizeY });
        static uint8_t* blendBuffer{ nullptr };
        static size_t curBufSize = 0;

        uint32_t& prevHash = overrideHash ? *overrideHash : prevTexHash;
        if (showPalette || blendPalette) {
            if (blendPalette && paletteBuf) {
                int32_t bpp = texture->getFormat() == TextureFormat::Indexed8 ? 1 : 2;
                int32_t palOffset = texture->getPaletteSize() * 4;
                size_t blendSize = (width * height * sizeof(Color32));
                size_t requiredSize = blendSize + (bpp * width * height + palOffset);

                if (prevHash != texture->getHash() || !blendBuffer || !paletteBuf->isValid() || newInstance) {
                    if (blendBuffer) {
                        if (curBufSize < requiredSize) {
                            void* data = reinterpret_cast<uint8_t*>(realloc(blendBuffer, requiredSize));
                            if (!data) {
                                return;
                            }
                            blendBuffer = reinterpret_cast<uint8_t*>(data);
                            curBufSize = requiredSize;
                        }
                    }
                    else if (curBufSize < requiredSize) {
                        blendBuffer = reinterpret_cast<uint8_t*>(malloc(requiredSize));
                        curBufSize = requiredSize;
                    }

                    uint8_t* ogData = blendBuffer + blendSize;
                    texture->getPixels(ogData);

                    uint8_t* ogPixelData = blendBuffer + blendSize + palOffset;
                    Color32* palette = reinterpret_cast<Color32*>(ogData);
                    Color32* blendData = reinterpret_cast<Color32*>(blendBuffer);
                    int32_t indexBuffer = 0;
                    for (size_t i = 0, j = 0; i < width * height; i++, j += bpp) {
                        memcpy(&indexBuffer, ogPixelData + j, bpp);
                        blendData[i] = palette[indexBuffer];
                    }

                    FrameBufferSpecs specs{};
                    specs.width = width;
                    specs.height = height;
                    specs.colorFormat = GL_RGBA8;
                    specs.pixelData = blendBuffer;

                    paletteBuf->invalidate(specs, true, true);
                    uint32_t fbCa = paletteBuf->getColorAttatchment();
                    prevHash = texture->getHash();
                }
            }
            else {
                if (useFB && paletteBuf) {
                    uint32_t palHash = texture->getHash() ^ 0xFFFFFFFFU;
                    if (prevHash != palHash || !paletteBuf->isValid() || newInstance) {
                        FrameBufferSpecs specs{};
                        specs.width = 256;
                        specs.height = 1;
                        specs.colorFormat = GL_RGBA8;
                        specs.pixelData = nullptr;

                        paletteBuf->invalidate(specs, true, true);
                        uint32_t fbCa = paletteBuf->getColorAttatchment();

                        glCopyImageSubData(pale, GL_TEXTURE_1D, 0, 0, 0, 0,
                            fbCa, GL_TEXTURE_2D, 0, 0, 0, 0,
                            256, 1, 1);

                        prevHash = palHash;

                        glBindTexture(GL_TEXTURE_2D, 0);
                        glBindTexture(GL_TEXTURE_1D, 0);

                    }
                    ImGui::SetCursorPos({ 0, cY + sizeY });
                    ImGui::Image((void*)size_t(paletteBuf->getColorAttatchment()), { sizeX, paletteRegionSize });
                }
                else {
                    ImGui::SetCursorPos({ 0, cY + sizeY });
                    ImGui::Image((void*)size_t(texture->getPaletteId()), { sizeX, paletteRegionSize });
                }
            }
        }

        ImGui::EndChildFrame();
        ImGui::PopID();
    }

    bool drawProgressBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col, const ImU32& hi_col_lhs) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { return false; }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = size_arg;
        size.x -= style.FramePadding.x * 2;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) { return false; }

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + size.x, bb.Max.y), bg_col);
        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + size.x * value, bb.Max.y), fg_col);

        static constexpr int32_t circles = 8;

        float time[circles * 2]{ 0 };
        ImU32 colors[circles]{};

        int32_t totalCirc = circles;
        const float offsetStep = totalCirc > 0 ? 1.0f / totalCirc : 0.0f;
        const float rad = size.y * 0.5f * Math::easeOutCubic(Math::clamp<float>(value, 0, 1));
        float offset = 0;
        for (size_t i = 0; i < totalCirc; i++) {
            float t = std::fmod(std::max<float>(float(g.Time) - offset, 0.0f), 3.0f) * 0.3333f;
            time[i] = Math::easeInOutQuart(t);
            float curve = (t < 0.5f ? t * 2.0f : 1.0f - (t - 0.5f) * 2.0f);
            time[totalCirc + i] = rad * Math::lerp(0.025f, 0.42f, curve);
            offset += offsetStep;

            uint32_t alpha = Math::lerp<uint32_t>(0, 255, Math::easeInOutCubic(curve));
            colors[i] = (hi_col_lhs & 0xFFFFFFU) | (alpha << 24);
        }

        ImVec2 posCMin{ bb.Min.x, bb.Min.y };
        ImVec2 posCMax{ bb.Min.x, bb.Max.y };
        for (size_t i = 0; i < totalCirc; i++) {
            float posX = bb.Min.x + time[i] * (size.x * value);
            posCMin.x = posX - time[totalCirc + i];
            posCMax.x = posX + time[totalCirc + i];
            window->DrawList->AddRectFilled(posCMin, posCMax, colors[i]);
        }
        return true;
    }

    bool drawProgressSpinner(const char* label, float radius, float thickness, const ImU32& color) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) { return false; }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) { return false; }

        window->DrawList->PathClear();

        float time = float(g.Time);
        int num_segments = 30;
        int start = abs(ImSin(time * 1.8f) * (num_segments - 5));

        const float a_min = IM_PI * 2.0f * ((float)start) / (float)num_segments;
        const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

        const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + time * 8) * radius,
                centre.y + ImSin(a + time * 8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
        return true;
    }
}