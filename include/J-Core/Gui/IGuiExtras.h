#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <J-Core/Gui/IGuiDrawable.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <glm.hpp>
#include <J-Core/Rendering/Texture.h>
#include <memory>

static constexpr uint32_t GUI_TEX_SPLIT_INDEXED = 0x1;
static constexpr uint32_t GUI_TEX_BLEND_INDEXED = 0x2;
static constexpr uint32_t GUI_TEX_SHOW_EXTRA = 0x4;

static constexpr uint32_t GUI_TEX_INFO_MASK = (0x8 | 0x10);

static constexpr uint32_t GUI_TEX_INFO_ABOVE = 0x8;
static constexpr uint32_t GUI_TEX_INFO_SIDE = 0x10;
static constexpr uint32_t GUI_TEX_INFO_BELOW = GUI_TEX_INFO_MASK;

template<>
bool JCore::IGuiDrawable<glm::vec2>::onGui(const char* label, glm::vec2& object, const bool doInline) {
    if (doInline) {
        return ImGui::DragFloat2(label, &object.x, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
    }

    bool changed = false;
    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Indent();
        changed |= ImGui::DragFloat("X", &object.x, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        changed |= ImGui::DragFloat("Y", &object.y, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::Unindent();
    }
    ImGui::PopID();
    return changed;
}

template<>
bool JCore::IGuiDrawable<glm::vec3>::onGui(const char* label, glm::vec3& object, const bool doInline) {
    if (doInline) {
        return ImGui::DragFloat3(label, &object.x, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
    }

    bool changed = false;
    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Indent();
        changed |= ImGui::DragFloat("X", &object.x, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        changed |= ImGui::DragFloat("Y", &object.y, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        changed |= ImGui::DragFloat("Z", &object.z, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::Unindent();
    }
    ImGui::PopID();
    return changed;
}

template<>
bool JCore::IGuiDrawable<glm::vec4>::onGui(const char* label, glm::vec4& object, const bool doInline) {
    if (doInline) {
        return ImGui::DragFloat4(label, &object.x, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
    }

    bool changed = false;
    ImGui::PushID(label);
    if (ImGui::CollapsingHeader(label)) {
        ImGui::Indent();
        changed |= ImGui::DragFloat("X", &object.x, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        changed |= ImGui::DragFloat("Y", &object.y, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        changed |= ImGui::DragFloat("Z", &object.z, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        changed |= ImGui::DragFloat("W", &object.w, 0.0025f, 0, 0, "%.4f", ImGuiSliderFlags_NoRoundToFormat);
        ImGui::Unindent();
    }
    ImGui::PopID();
    return changed;
}

namespace JCore::Gui {
    bool searchDialogCenter(const char* label, uint8_t flags, char path[513], const char* types = nullptr, const size_t defaultType = 1);
    bool searchDialogLeft(const char* label, uint8_t flags, char path[513], const char* types = nullptr, const size_t defaultType = 1);

    template<typename T>
    bool drawEnumList(const char* label, T& value) {
        int32_t valueI = int32_t(value);
        bool changed = false;

        changed = EnumNames<T>::getNextValidIndex(valueI);
        auto values = EnumNames<T>::getEnumNames();

        ImGui::PushID(label);
        if (ImGui::BeginCombo(label, values[valueI])) {
            for (int32_t i = 0; i < EnumNames<T>::Count; i++) {
                if (EnumNames<T>::noDraw(i)) { continue; }
                const bool selected = i == valueI;

                ImGui::PushID(i);
                if (ImGui::Selectable(values[i], selected)) {
                    changed = true;
                    valueI = i;
                }
                if (selected) {
                    ImGui::SetItemDefaultFocus();
                }
                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
        ImGui::PopID();

        if (changed) {
            value = T(valueI);
            return true;
        }
        return false;
    }

    bool drawSplitter(bool splitVertical, float thickness, float* size0, float* size1, float minSize0, float minSize1, float splitterAxisSize = -1.0f);
    bool drawSplitter(const char* id, bool splitVertical, float thickness, float* size0, float* size1, float minSize0, float minSize1, float splitterAxisSize = -1.0f);

    template<typename T>
    bool drawBezierCurve(const char* label, BezierCurve<T>& curve) {
        enum { CURVE_WIDTH = 4 }; // main curved line width
        enum { LINE_WIDTH = 1 }; // handlers: small lines width
        enum { GRAB_RADIUS = 6 }; // handlers: circle radius
        enum { GRAB_BORDER = 2 }; // handlers: circle border width

        const ImGuiStyle& Style = ImGui::GetStyle();
        const ImGuiIO& IO = ImGui::GetIO();
        ImDrawList* DrawList = ImGui::GetWindowDrawList();
        ImGuiWindow* Window = ImGui::GetCurrentWindow();
        if (Window->SkipItems) { return false; }

        ImGui::PushID(label);
        int changed = ImGui::SliderFloat4(label, curve.points, 0, 1, "%.3f");
        int hovered = ImGui::IsItemActive() || ImGui::IsItemHovered();
        ImGui::Dummy(ImVec2(0, 3));

        const float avail = ImGui::GetContentRegionAvail().x;
        const float dim = ImMin(avail, 128.f);
        ImVec2 Canvas(dim, dim);

        ImRect bb(Window->DC.CursorPos, { Window->DC.CursorPos.x + Canvas.x, Window->DC.CursorPos.y + Canvas.y });
        ImGui::ItemSize(bb);
        if (!ImGui::ItemAdd(bb, NULL)) {
            if (changed)
            {
                curve.bake();
            }

            ImGui::PopID();
            return changed;
        }

        const ImGuiID id = Window->GetID(label);
        hovered |= ImGui::GetHoveredID() == id;

        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg, 1), true, Style.FrameRounding);

        for (int i = 0; i <= Canvas.x; i += (Canvas.x / 4)) {
            DrawList->AddLine(
                ImVec2(bb.Min.x + i, bb.Min.y),
                ImVec2(bb.Min.x + i, bb.Max.y),
                ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }
        for (int i = 0; i <= Canvas.y; i += (Canvas.y / 4)) {
            DrawList->AddLine(
                ImVec2(bb.Min.x, bb.Min.y + i),
                ImVec2(bb.Max.x, bb.Min.y + i),
                ImGui::GetColorU32(ImGuiCol_TextDisabled));
        }

        {
            char buf[128];
            sprintf(buf, "0##%s", label);

            for (int i = 0; i < 2; ++i)
            {
                ImGui::PushID(i);
                ImVec2 pos = ImVec2(curve.points[i * 2 + 0], 1 - curve.points[i * 2 + 1]) * (bb.Max - bb.Min) + bb.Min;
                ImGui::SetCursorScreenPos(pos - ImVec2(GRAB_RADIUS, GRAB_RADIUS));
                ImGui::InvisibleButton((buf[0]++, buf), ImVec2(2 * GRAB_RADIUS, 2 * GRAB_RADIUS));
                if (ImGui::IsItemActive() || ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("(%4.3f, %4.3f)", curve.points[i * 2 + 0], curve.points[i * 2 + 1]);
                }
                if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0))
                {
                    curve.points[i * 2 + 0] += ImGui::GetIO().MouseDelta.x / Canvas.x;
                    curve.points[i * 2 + 1] -= ImGui::GetIO().MouseDelta.y / Canvas.y;
                    changed = true;
                }
                ImGui::PopID();
            }

            if (changed)
            {
                curve.bake();
            }

            if (hovered || changed) { DrawList->PushClipRectFullScreen(); }

            {
                ImColor color(ImGui::GetStyle().Colors[ImGuiCol_PlotLines]);
                for (int i = 0; i < 256; ++i) {
                    auto pA = curve.getNormalized(i);
                    auto pB = curve.getNormalized(i + 1);

                    ImVec2 p = { pA.x, 1 - pA.y };
                    ImVec2 q = { pB.x, 1 - pB.y };
                    ImVec2 r(p.x * (bb.Max.x - bb.Min.x) + bb.Min.x, p.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
                    ImVec2 s(q.x * (bb.Max.x - bb.Min.x) + bb.Min.x, q.y * (bb.Max.y - bb.Min.y) + bb.Min.y);
                    DrawList->AddLine(r, s, color, CURVE_WIDTH);
                }
            }

            float luma = ImGui::IsItemActive() || ImGui::IsItemHovered() ? 0.5f : 1.0f;
            ImVec4 pink(1.00f, 0.00f, 0.75f, luma), cyan(0.00f, 0.75f, 1.00f, luma);
            ImVec4 white(ImGui::GetStyle().Colors[ImGuiCol_Text]);
            ImVec2 p1 = ImVec2(curve.points[0], 1 - curve.points[1]) * (bb.Max - bb.Min) + bb.Min;
            ImVec2 p2 = ImVec2(curve.points[2], 1 - curve.points[3]) * (bb.Max - bb.Min) + bb.Min;
            DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), p1, ImColor(white), LINE_WIDTH);
            DrawList->AddLine(ImVec2(bb.Max.x, bb.Min.y), p2, ImColor(white), LINE_WIDTH);
            DrawList->AddCircleFilled(p1, GRAB_RADIUS, ImColor(white));
            DrawList->AddCircleFilled(p1, GRAB_RADIUS - GRAB_BORDER, ImColor(pink));
            DrawList->AddCircleFilled(p2, GRAB_RADIUS, ImColor(white));
            DrawList->AddCircleFilled(p2, GRAB_RADIUS - GRAB_BORDER, ImColor(cyan));

            if (hovered || changed) DrawList->PopClipRect();
            ImGui::SetCursorScreenPos(ImVec2(bb.Min.x, bb.Max.y + GRAB_RADIUS));
        }
        ImGui::PopID();
        return changed;
    }

    void drawTexture(uint32_t texture, int32_t width, int32_t height, float sizeX, float sizeY, bool keepAspect, float edge = 0.1f);
    void drawTexture(std::shared_ptr<Texture>& texture, uint32_t flags, float sizeX, float sizeY, bool keepAspect, float edge = 0.1f, uint8_t* extraFlags = nullptr, uint64_t* overrideId = nullptr, uint32_t* overrideHash = nullptr, Color32* bgColor = nullptr);

    template<typename T>
    bool drawBitMask(const char* label, T& value, int32_t start, int32_t length, const char* const* names, bool allowMultiple = true) {
        static constexpr size_t BITS = (sizeof(T) << 3);
        length = std::min<int32_t>(length, BITS - start);
        if (length <= 0) { return false; }
        uint64_t bits = uint64_t(value);

        bool changed = false;
        char temp[257]{ 0 };
        bool tempToggle[64]{ 0 };
        size_t tempL = 0;
        size_t bitsSet = 0;
        bool wasSet = false;
        for (size_t i = 0, j = 1ULL << start; i < length; i++, j <<= 1) {
            if (tempL >= 256) { break; }
            if (bits & j) {
                if (!wasSet) {
                    bitsSet++;
                    tempToggle[start + i] = true;
                    wasSet = !allowMultiple;

                    if (tempL > 0) {
                        memcpy_s(temp + tempL, 256 - tempL, ", ", 2);
                        tempL += 2;
                    }
                    const char* name = names[i];
                    size_t len = strlen(name);

                    memcpy_s(temp + tempL, 256 - tempL, name, len);
                    tempL += len;
                }
                else {
                    changed |= true;
                }
            }
        }

        if (tempL == 0) {
            sprintf_s(temp, "None");
        }
        else if (bitsSet == length) {
            sprintf_s(temp, "Everything");
        }

        uint64_t mask = ((1ULL << length) - 1) << start;
        if (ImGui::BeginCombo(label, temp, ImGuiComboFlags_HeightLarge)) {
            ImGui::Indent();
            ImGui::PushID("Elements");

            for (size_t i = 0, j = 1ULL << start; i < length; i++, j <<= 1) {
                bool& tempBool = *(tempToggle + start + i);
                ImGui::PushID(int32_t(i));
                if (ImGui::Checkbox(names[i], &tempBool)) {
                    changed = true;
                    if (tempBool) {
                        if (!allowMultiple) {
                            uint64_t negate = (mask & ~j);
                            bits &= ~negate;
                        }
                        bits |= j;
                    }
                    else {
                        bits &= ~j;
                   }
                }
                ImGui::PopID();
            }

            ImGui::PopID();
            ImGui::Unindent();
            ImGui::EndCombo();
        }

        if (changed) {
            value = T(bits);
        }
        return changed;
    }

    template<typename T, size_t instance = 0>
    bool drawBitMask(const char* label, T& value, bool allowMultiple = true) {
        return drawBitMask(label, value, EnumNames<T, instance>::Start, EnumNames<T, instance>::Count, EnumNames<T, instance>::getEnumNames(), allowMultiple);
    }

    bool drawProgressBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col, const ImU32& hi_col_lhs);
    bool drawProgressSpinner(const char* label, float radius, float thickness, const ImU32& color);

    template<typename T>
    bool drawDropdown(const char* label, T& value, const char** names, int32_t itemCount) {
        int32_t valueInt = value;
        if (ImGui::Combo(label, &valueInt, names, itemCount)) {
            value = T(valueInt);
            return true;
        }
        return false;
    }

    template<typename T>
    bool drawInputInt(const char* label, T& value, int32_t step = 1, int32_t stepFast = 100, uint32_t flags = 0) {
        int32_t valueInt = int32_t(value);

        if (ImGui::InputInt(label, &valueInt, step, stepFast, falgs)) {
            value = T(valueInt);
            return true;
        }
        return false;
    }
}
