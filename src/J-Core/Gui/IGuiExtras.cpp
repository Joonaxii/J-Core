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
#include <J-Core/Util/StringUtils.h>

#include <io.h>
#include <J-Core/IO/FileStream.h>
#define F_OK 0
#define access _access


namespace JCore::Gui {
    static int32_t getTextureSaveInfoMode(const std::shared_ptr<Texture>& tex) {
        auto format = tex->getFormat();
        switch (format) {
        case JCore::TextureFormat::Indexed8:
        case JCore::TextureFormat::Indexed16:
            return 0;
        default: return 0;
        }
    }

    void clearGuiInput(const char* label) {
        const ImGuiID id = ImGui::GetCurrentWindow()->GetID(label);
        auto inputState = ImGui::GetInputTextState(id);

        if (inputState && inputState->ID == id) {
            inputState->ClearSelection();
            inputState->ClearText();
            inputState->CursorAnimReset();
            inputState->ID = 0;
        }
    }

    bool searchDialogCenter(const char* label, uint8_t flags, std::string& path, const char* types, size_t defaultType) {
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

        bool changed = ImGui::InputTextWithHint("##Path", label, &path);
        if ((flags & 0x1) && !(ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) && ImGui::GetHoveredID() == ImGui::GetItemID()) {
            auto& allPaths = Window::getBufferedFileDrops();
            if (allPaths.size() > 0) {

                if (types == nullptr) {
                    for (size_t i = 0; i < allPaths.size(); i++) {
                        if (fs::is_directory(allPaths[i])) {
                            path = allPaths[i];
                            changed |= true;
                            break;
                        }
                    }
                }
                else {
                    for (size_t i = 0; i < allPaths.size(); i++) {
                        if (fs::is_regular_file(allPaths[i]) && IO::matchFilter(allPaths[i], types)) {
                            path = allPaths[i];
                            changed |= true;
                            break;
                        }
                    }
                }
                Window::clearFileDrops();
            }
        }
        ImGui::PopItemWidth();

        if (press) {
            //Check if we're a folder
            if (types == nullptr) {
                auto str = IO::openFolder("Search folder");
                if (str.length() > 0) {
                    path = str;
                    changed = true;
                }
            }
            else {
                auto str = IO::openFile(types, 260, false, false, defaultType);
                if (str.length() > 0) {
                    path = str;
                    changed = true;
                }
            }
        }
        ImGui::PopID();
        ImGui::EndGroup();
        return changed;
    }
    bool searchDialogLeft(const char* label, uint8_t flags, std::string& path, const char* types, size_t defaultType) {
        ImGui::BeginGroup();
        ImGui::PushID(label);
        float sqSize = ImGui::GetFrameHeight();
        float width = ImGui::GetWindowSize().x;
        bool press = ImGui::RadioButton("##Search", true);
        ImGui::SameLine();

        ImGui::PushItemWidth(std::max(width - sqSize - 5, 120.0f));

        bool changed = ImGui::InputTextWithHint("##Path", label, &path);
        if (changed) {
            IO::fixPath(path.data(), path.length());
        }
        if ((flags & 0x1) && !(ImGui::GetCurrentContext()->CurrentItemFlags & ImGuiItemFlags_Disabled) && ImGui::GetHoveredID() == ImGui::GetItemID()) {
            auto& allPaths = Window::getBufferedFileDrops();
            if (allPaths.size() > 0) {
                if (types == nullptr) {
                    for (size_t i = 0; i < allPaths.size(); i++) {
                        if (fs::is_directory(allPaths[i])) {
                            clearGuiInput("##Path");

                            path = allPaths[i];
                            IO::fixPath(path.data(), path.length());
                            changed |= true;
                            break;
                        }
                    }
                }
                else {
                    for (size_t i = 0; i < allPaths.size(); i++) {
                        if (fs::is_regular_file(allPaths[i]) && IO::matchFilter(allPaths[i], types)) {
                            clearGuiInput("##Path");
                            path = allPaths[i];
                            IO::fixPath(path.data(), path.length());
                            changed |= true;
                            break;
                        }
                    }
                }
                Window::clearFileDrops();
            }
        }
        ImGui::PopItemWidth();

        if (press) {
            //Check if we're a folder
            if (types == nullptr) {
                auto str = IO::openFolder("Search folder");
                if (str.length() > 0) {
                    path = str;
                    IO::fixPath(path.data(), path.length());
                    changed = true;
                }
            }
            else {
                auto str = IO::openFile(types, 260, false, false, defaultType, path);
                if (str.length() > 0) {
                    path = str;
                    IO::fixPath(path.data(), path.length());
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
        Utils::formatDataSize(tempBuf, calculateTextureSize(texture->getWidth(), texture->getHeight(), texture->getFormat(), texture->getPaletteSize()));
        ImGui::Text(" - Memory Usage : %s", tempBuf);
        ImGui::EndChildFrame();
    }

    struct TexDrawData {
        int8_t mipLevel{ -1 };
        uint32_t glId[2]{ 0, 0 };
        bool isTex1D{ false };
        TextureFormat format{};
        TextureFormat rawFormat{};
    };

    static void drawTextureCB(const ImDrawCmd* cmd, TextureFormat format, TextureFormat rawFormat, int8_t mipLevel, uint32_t glID0, uint32_t glID1) {
        auto rend = Renderer::getInstance();
        const Shader* shader = nullptr;
        switch (format)
        {
            case TextureFormat::Indexed8:
                shader = &rend->getShader(format);
                break;
        }

        if (shader) {
            shader->bind();
        }

        Texture::bind(rawFormat, 0, glID0, glID1, mipLevel, rawFormat == TextureFormat::Indexed8 ? 0 : -1);
        (glDrawElements(GL_TRIANGLES, (GLsizei)cmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, (void*)(intptr_t)(cmd->IdxOffset * sizeof(ImDrawIdx))));
        Texture::unbind(rawFormat, 0, true, rawFormat == TextureFormat::Indexed8 ? 0 : -1);
        if (shader) {
            shader->unbind();
        }
    }

    static bool setupDrawCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        auto draw_data = ImGui::GetDrawData();
        int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
        ImVec2 clip_off = draw_data->DisplayPos;
        ImVec2 clip_scale = draw_data->FramebufferScale;

        // Project scissor/clipping rectangles into framebuffer space
        ImVec2 clip_min((cmd->ClipRect.x - clip_off.x) * clip_scale.x, (cmd->ClipRect.y - clip_off.y) * clip_scale.y);
        ImVec2 clip_max((cmd->ClipRect.z - clip_off.x) * clip_scale.x, (cmd->ClipRect.w - clip_off.y) * clip_scale.y);
        if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
            return true;

        (glScissor((int)clip_min.x, (int)((float)fb_height - clip_max.y), (int)(clip_max.x - clip_min.x), (int)(clip_max.y - clip_min.y)));
        return false;
    }

    static void drawTextureCallback(const ImDrawList* parent_list, const ImDrawCmd* cmd) {
        TexDrawData* texPtr = reinterpret_cast<TexDrawData*>(cmd->UserCallbackData);
        if (texPtr) {
            if (setupDrawCallback(parent_list, cmd)) { return; }
            drawTextureCB(cmd, texPtr->format, texPtr->rawFormat, texPtr->mipLevel, texPtr->glId[0], texPtr->glId[1]);
        }
    }

    static void drawTextureInfoGui(const Texture* texture, const ImVec2& size, const ImVec2& frameSize) {
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
        Utils::formatDataSize(tempBuf, calculateTextureSize(texture->getWidth(), texture->getHeight(), texture->getFormat(), texture->getPaletteSize()));
        ImGui::Text(" - Memory Usage : %s", tempBuf);
        ImGui::EndChildFrame();
    }

    static void drawTextureExport(const Texture* tex, ImageEncodeParams& encodeParams, DataFormat& format) {
        const char* IMAGE_FMT = nullptr;
        bool pressed = ImGui::Button("Export##TexturePreview");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120);
        Gui::drawEnumList<DataFormat, 0>("Format##TexturePreview", format);
        switch (format) {
        case DataFormat::FMT_PNG:
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 10);
            ImGui::SliderInt("Compression##TexturePreview", &encodeParams.compression, 0, 9);
            IMAGE_FMT = "PNG File (*.png)\0*.png\0";
            break;
        case DataFormat::FMT_BMP:
            ImGui::SameLine();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 10);
            ImGui::SliderInt("DPI##TexturePreview", &encodeParams.dpi, 32, 300);
            IMAGE_FMT = "BMP File (*.bmp)\0*.bmp\0";
            break;
        case DataFormat::FMT_DDS:
            IMAGE_FMT = "DDS File (*.dds)\0*.dds\0";
            break;
        case DataFormat::FMT_JTEX:
            IMAGE_FMT = "JTEX File (*.jtex)\0*.jtex\0";
            break;
        }

        if (pressed && IMAGE_FMT) {
            std::string path = IO::openFile(IMAGE_FMT, 260, true);
            ImageData data{ tex->getWidth(), tex->getHeight(), tex->getFormat(), tex->getPaletteSize(), tex->getPixels() };
            Image::tryEncode(path.c_str(), data, format, encodeParams);
            data.clear(true);
        }

    }

    static bool drawTexturePreview(const Texture* tex, bool& isOpen, ImageEncodeParams& encodeParams, DataFormat& format) {
        if (ImGui::Begin("Texture Preview##TexturePreview", &isOpen)) {
            drawTextureExport(tex, encodeParams, format);
            ImGui::Separator();
            auto avail = ImGui::GetContentRegionAvail();
            float maxS = std::max(avail.x, avail.y);
            Gui::drawTexture(tex, 0, maxS, maxS, true, 0.1f);
        }
        ImGui::End();
        return isOpen;
    }

    void drawTexture(uint32_t texture, int32_t width, int32_t height, float sizeX, float sizeY, bool keepAspect, float edge, int8_t mipLevel) {
        static TexDrawData TexData{};

        TexData.glId[0] = texture;
        TexData.format = TextureFormat::RGBA32;
        TexData.mipLevel = mipLevel;
        auto drawList = ImGui::GetWindowDrawList();
        drawList->AddCallback(drawTextureCallback, &TexData);

        void* texPtr = (void*)size_t(texture);
        if (keepAspect) {
            float size = 1.0f - edge;
            float aspectA = width / float(height);
            ImGui::Image(texPtr, { sizeX * aspectA * size, sizeY * size });
            return;
        }
        ImGui::Image(texPtr, { sizeX, sizeY });
    }
    void drawTexture(std::shared_ptr<Texture> texture, uint32_t flags, float sizeX, float sizeY, bool keepAspect, float edge, uint8_t* extraFlags, uint64_t* overrideId, uint32_t* overrideHash, Color32* bgColor, int8_t* mipLevel) {
        drawTexture(texture.get(), flags, sizeX, sizeY, keepAspect, edge, extraFlags, overrideId, overrideHash, bgColor, mipLevel);
    }
    void drawTexture(const Texture* texture, uint32_t flags, float sizeX, float sizeY, bool keepAspect, float edge, uint8_t* extraFlags, uint64_t* overrideId, uint32_t* overrideHash, Color32* bgColor, int8_t* mipLevel) {
        if (!texture || !texture->isValid()) {
            return;
        }
        static TexDrawData TexData{};
        auto drawList = ImGui::GetWindowDrawList();
        TexData.rawFormat = texture->getFormat();

        uint32_t infoLvl = flags & GUI_TEX_INFO_MASK;

        TextureFormat fmt = texture->getFormat();
        bool isIndexed = fmt == TextureFormat::Indexed8 || fmt == TextureFormat::Indexed16;

        static int32_t gMipLevel = -1;
        uint32_t main = texture->getTextureId();
        uint32_t pale = texture->getPaletteId();

        int32_t width = texture->getWidth();
        int32_t height = texture->getHeight();

        float offX = 0;
        float offY = 0;

        static bool blendIndexed = false;
        bool blending = extraFlags ? (*extraFlags & 0x1) : blendIndexed;

        float cPX = ImGui::GetCursorPosX();

        ImGui::PushID(texture);
        bool blendPalette = (flags & GUI_TEX_SHOW_EXTRA) ? blending : (flags & GUI_TEX_BLEND_INDEXED);
        bool useFB = (isIndexed && blendPalette) || texture->getFormat() == TextureFormat::Indexed8;

        bool showPalette = (flags & GUI_TEX_SPLIT_INDEXED) && isIndexed && !blendPalette;
        float infoOffsetX = 0;
        float infoOffsetY = 0;

        static constexpr float BUFFER_INFO = 20;
        static constexpr float WIDTH_INFO = 275;

        float infoHeight = ImGui::GetTextLineHeight() * (isIndexed ? 6 : 5);

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
            case TextureFormat::Indexed8:
                paletteRegionSize = sizeY * 0.25f;
                break;
            case TextureFormat::Indexed16:
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

        TexData.mipLevel = -1;

        float cY = ImGui::GetCursorPosY();
        ImVec2 infoSize = { float(WIDTH_INFO) + BUFFER_INFO - 25.0f,  float(infoHeight) + BUFFER_INFO };
        switch (infoLvl)
        {
        default:
            ImGui::SetCursorPos({ 0, cY });
            break;
        case GUI_TEX_INFO_ABOVE:
            ImGui::SetCursorPos({ 0, cY });
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

        if (isIndexed) {
            if (blendPalette) {
                TexData.format = texture->getFormat();
                TexData.glId[0] = texture->getTextureId();
                TexData.glId[1] = texture->getPaletteId();
                drawList->AddCallback(drawTextureCallback, &TexData);
                ImGui::Image((void*)size_t(texture->getTextureId()), { sizeX, sizeY });
            }
            else {
                TexData.format = TextureFormat::R8;
                TexData.glId[0] = texture->getTextureId();
                TexData.glId[1] = texture->getPaletteId();
                drawList->AddCallback(drawTextureCallback, &TexData);
                ImGui::Image((void*)size_t(texture->getTextureId()), { sizeX, sizeY });
            }

            if (showPalette) {
                TexData.format = TextureFormat::RGBA32;
                TexData.glId[0] = texture->getPaletteId();
                TexData.glId[1] = 0;
                ImGui::SetCursorPos({ 0, cY + sizeY });
                drawList->AddCallback(drawTextureCallback, &TexData);
                ImGui::Image((void*)size_t(texture->getPaletteId()), { sizeX, paletteRegionSize });
            }
        }
        else {
            TexData.format = texture->getFormat();
            TexData.glId[0] = texture->getTextureId();
            TexData.glId[1] = texture->getPaletteId();
            drawList->AddCallback(drawTextureCallback, &TexData);
            ImGui::Image((void*)size_t(texture->getTextureId()), { sizeX, sizeY });
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
        float num_segments = 30;
        float start = abs(ImSin(time * 1.8f) * (num_segments - 5));

        const float a_min = IM_PI * 2.0f * (start) / num_segments;
        const float a_max = IM_PI * 2.0f * (num_segments - 3) / num_segments;

        const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

        for (int i = 0; i < num_segments; i++) {
            const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
            window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + time * 8) * radius,
                centre.y + ImSin(a + time * 8) * radius));
        }

        window->DrawList->PathStroke(color, false, thickness);
        return true;
    }

    bool drawBitMask_Raw(std::string_view label, void* value, size_t size, uint64_t start, uint64_t length, Enum::GetEnumName nameFunc, bool allowMultiple, bool displayAll) {
        size = Math::min(size, 8ULL);
        uint64_t bitCount = (size << 3);
        length = Math::min(length, bitCount - start);
        if (length <= 0) { return false; }

        uint64_t bits = 0;
        memcpy(&bits, value, size);

        bool changed = false;
        char temp[257]{ 0 };
        uint64_t tempL = 0;
        uint64_t bitsSet = 0;
        bool wasSet = false;
        for (uint64_t i = 0, j = 1ULL << start; i < length; i++, j <<= 1) {
            if (tempL >= 256) { break; }
            if (bits & j) {
                if (!wasSet) {
                    bitsSet++;
                    wasSet = !allowMultiple;

                    if (tempL > 0) {
                        memcpy(temp + tempL, ", ", 2);
                        tempL += 2;
                    }
                    std::string_view name = nameFunc(&j);
                    memcpy(temp + tempL, name.data(), name.length());
                    tempL += name.length();
                }
                else { changed |= true; }
            }
        }

        if (tempL == 0) {
            sprintf_s(temp, "None");
        }
        else if (bitsSet == length && displayAll) {
            sprintf_s(temp, "Everything");
        }

        uint64_t mask = ((1ULL << length) - 1) << start;
        if (ImGui::BeginCombo(label, temp, ImGuiComboFlags_HeightLarge)) {
            ImGui::Indent();
            ImGui::PushID("Elements");

            for (uint64_t i = 0, j = 1ULL << start; i < length; i++, j <<= 1) {
                bool tempBool = (bits & j) != 0;

                ImGui::PushID(&i);
                if (ImGui::Checkbox(nameFunc(&j), &tempBool)) {
                    changed = true;
                    if (tempBool) {
                        if (!allowMultiple) {
                            uint64_t negate = (mask & ~j);
                            bits &= ~negate;
                        }
                        bits |= j;
                    }
                    else { bits &= ~j; }
                }
                ImGui::PopID();
            }

            ImGui::PopID();
            ImGui::Unindent();
            ImGui::EndCombo();
        }

        if (changed) {
            memcpy(value, &bits, size);
        }
        return changed;
    }

    bool drawDropdown(std::string_view label, void* value, size_t size, uint64_t start, uint64_t length, Enum::GetEnumName nameFunc) {
        bool changed = false;
        uint64_t selectI = 0;
        size = Math::min(size, sizeof(uint64_t));
        memcpy(&selectI, value, size);

        if (ImGui::BeginCombo(label, nameFunc(&selectI)))
        {
            for (uint64_t i = 0, j = start; i < length; i++, j++) {
                std::string_view name = nameFunc(&j);
                if (name.length() < 1) { continue; }

                ImGui::PushID(&i);
                if (ImGui::Selectable(name, bool(j == selectI), 0)) {
                    selectI = j;
                    memcpy(value, &selectI, size);
                    changed = true;
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        return changed;
    }
}


namespace ImGui {
    bool CollapsingHeaderNoId(ImStrv label, ImStrv idStr, bool* p_visible, ImGuiTreeNodeFlags flags) {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) {
            return false;
        }
   
        if (p_visible && !*p_visible) {
            return false;
        }
          
        ImGuiID id = window->GetID(idStr);
        flags |= ImGuiTreeNodeFlags_CollapsingHeader;
        if (p_visible) {
            flags |= ImGuiTreeNodeFlags_AllowOverlap | (ImGuiTreeNodeFlags)ImGuiTreeNodeFlags_ClipLabelForTrailingButton;
        }
 
        bool is_open = TreeNodeBehavior(id, flags, label);
        if (p_visible != NULL)
        {
            // Create a small overlapping close button
            // FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars, headers, etc.
            // FIXME: CloseButton can overlap into text, need find a way to clip the text somehow.
            ImGuiContext& g = *GImGui;
            ImGuiLastItemData last_item_backup = g.LastItemData;
            float button_size = g.FontSize;
            float button_x = ImMax(g.LastItemData.Rect.Min.x, g.LastItemData.Rect.Max.x - g.Style.FramePadding.x - button_size);
            float button_y = g.LastItemData.Rect.Min.y + g.Style.FramePadding.y;
            ImGuiID close_button_id = GetIDWithSeed("#CLOSE", NULL, id);
            if (CloseButton(close_button_id, ImVec2(button_x, button_y))) {
                *p_visible = false;
            }         
            g.LastItemData = last_item_backup;
        }

        return is_open;
    }
}
