#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <J-Core/Log.h>
#include <J-Core/Gui/IGuiDrawable.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <misc/cpp/imgui_stdlib.h>
#include <J-Core/Util/EnumUtils.h>
#include <glm.hpp>
#include <J-Core/Rendering/Texture.h>
#include <J-Core/Util/StringUtils.h>
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
    namespace detail {
        static constexpr std::string_view DEFAULT_BIT_NAMES[64]{
                 "Bit 0" , "Bit 1" , "Bit 2" , "Bit 3" , "Bit 4" , "Bit 5" , "Bit 6" , "Bit 7" ,
                 "Bit 8" , "Bit 9" , "Bit 10", "Bit 11", "Bit 12", "Bit 13", "Bit 14", "Bit 15",
                 "Bit 16", "Bit 17", "Bit 18", "Bit 19", "Bit 20", "Bit 21", "Bit 22", "Bit 23",
                 "Bit 24", "Bit 25", "Bit 26", "Bit 27", "Bit 28", "Bit 29", "Bit 30", "Bit 31",
                 "Bit 32", "Bit 33", "Bit 34", "Bit 35", "Bit 36", "Bit 37", "Bit 38", "Bit 39",
                 "Bit 40", "Bit 41", "Bit 42", "Bit 43", "Bit 44", "Bit 45", "Bit 46", "Bit 47",
                 "Bit 48", "Bit 49", "Bit 50", "Bit 51", "Bit 52", "Bit 53", "Bit 54", "Bit 55",
                 "Bit 56", "Bit 57", "Bit 58", "Bit 59", "Bit 60", "Bit 61", "Bit 62", "Bit 63",
        };

        template<typename U>
        size_t indexOfName(const char* value, const U* values, size_t count) {
            for (size_t i = 0; i < count; i++) {
                if (values[i] == value) {
                    return i;
                }
            }
            return SIZE_MAX;
        }
    }

    void clearGuiInput(const char* label);

    bool searchDialogCenter(const char* label, uint8_t flags, std::string& path, const char* types = nullptr, size_t defaultType = 1);
    bool searchDialogLeft(const char* label, uint8_t flags, std::string& path, const char* types = nullptr, size_t defaultType = 1);

    bool drawBitMask(std::string_view label, void* value, size_t size, uint64_t start, uint64_t length, JCore::Enum::GetEnumName nameFunc, bool allowMultiple = true, bool displayAll = true);

    template<typename T, typename STR>
    bool drawBitMask(std::string_view label, T& value, uint64_t start, uint64_t length, const STR* names, bool allowMultiple = true, bool displayAll = true) {
        static_assert(Enum::isUnsigned<T>(), "Bitmask must be an unsigned type!");
        static_assert(sizeof(T) <= 8, "Given type is too big to be a bitmask! (8 bytes max)");

        static constexpr uint64_t BITS = (sizeof(T) << 3);
        JCORE_ASSERT(BITS >= start, "Given start bit is higher than total number of bits!");

        return drawBitMask(label, &value, sizeof(T), start, length,
            [names, &start](const void* ptr)
            {
                uint64_t index = 0;
                JE_COPY(&index, ptr, Math::min(sizeof(T), sizeof(uint64_t)));
                return std::string_view{ names[Math::log2(index) - start] };
            });
    }


    template<typename T, size_t ID = 0>
    bool drawBitMask(std::string_view label, T& value, bool allowMultiple = true, bool displayAll = true) {
        if constexpr (std::is_enum<T>::value && EnumInfo<T, ID>::IsDefined) {
            return Gui::drawBitMask<T, std::string_view>(label, value, EnumInfo<T, ID>::MinValue, EnumInfo<T, ID>::Count, EnumInfo<T, ID>::Names, allowMultiple, displayAll);
        }
        return Gui::drawBitMask<T, std::string_view>(label, value, 0, sizeof(T) << 3, detail::DEFAULT_BIT_NAMES, allowMultiple, displayAll);
    }

    bool drawDropdown(std::string_view label, void* value, size_t size, uint64_t start, uint64_t length, Enum::GetEnumName nameFunc);

    template<typename T, typename STR, T StartValue = T{} >
    bool drawDropdown(std::string_view label, T& value, const STR* names, T itemCount) {
        static_assert(Enum::isUnsigned<T>(), "Values used in dropdowns must be unsigned!");
        return drawDropDown(label, &value, sizeof(T), uint64_t(StartValue), uint64_t(itemCount), [names](const void* ptr)
            {
                uint64_t index = 0;
                memcpy(&index, ptr, Math::min(sizeof(T), sizeof(uint64_t)));
                return std::string_view{ names[index - uint64_t(StartValue)] };
            });
    }

    template<typename T, size_t ID = 0>
    bool drawDropdown(std::string_view label, T& value) {
        static_assert(IS_ENUM_DEFINED(T, ID), "Given type isn't an enum or the enum is undefined!");
        return Gui::drawDropdown<T, std::string_view, EnumInfo<T, ID>::MinValue>(label, value, EnumInfo<T, ID>::Names, T(EnumInfo<T, ID>::Count))
    }

    template<typename T, size_t type = 0>
    bool drawEnumList(const char* label, T& value, bool allowSearch = false, bool ignoreNoDraw = false) {
        static constexpr int64_t OFFSET = (EnumNames<T, type>::Start < 0 ? -EnumNames<T, type>::Start : 0);
        int32_t valueI = int32_t(value) + OFFSET;
        bool changed = false;

        changed = EnumNames<T, type>::getNextValidIndex(valueI, ignoreNoDraw);
        auto values = EnumNames<T, type>::getEnumNames();

        static char buffer[257]{ 0 };

        ImGui::PushID(label);

        bool combo;
        if (allowSearch) {
            float avail = ImGui::GetContentRegionAvail().x;
            float rest = avail * 0.75f;
            float filter = avail - rest;

            ImGui::PushItemWidth(filter * 0.5f);
            ImGui::Text(label);
            ImGui::SameLine();

            ImGui::InputTextWithHint("##Filter", "Filter", buffer, 256);
            ImGui::PopItemWidth();
            ImGui::SameLine();

            ImGui::SetNextItemWidth(rest);
            combo = ImGui::BeginCombo("##EnumItems", values[valueI]);
        }
        else {
            combo = ImGui::BeginCombo(label, values[valueI]);
        }

        if (combo) {
 
            for (int32_t i = 0; i < EnumNames<T, type>::Count; i++) {
                if (EnumNames<T, type>::noDraw(i) || (buffer[0] != 0 && (allowSearch && !Utils::strIContains(values[i], buffer)))) { continue; }
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
            value = T(valueI - OFFSET);
            return true;
        }
        return false;
    }

    bool drawSplitter(bool splitVertical, float thickness, float* size0, float* size1, float minSize0, float minSize1, float splitterAxisSize = -1.0f);
    bool drawSplitter(const char* id, bool splitVertical, float thickness, float* size0, float* size1, float minSize0, float minSize1, float splitterAxisSize = -1.0f);

    void drawTexture(uint32_t texture, int32_t width, int32_t height, float sizeX, float sizeY, bool keepAspect, float edge = 0.1f);
    void drawTexture(std::shared_ptr<Texture>& texture, uint32_t flags, float sizeX, float sizeY, bool keepAspect, float edge = 0.1f, const glm::vec2& uvMin = { 0, 0 }, const glm::vec2& uvMax = { 1, 1 }, uint8_t* extraFlags = nullptr, uint64_t* overrideId = nullptr, uint32_t* overrideHash = nullptr, Color32* bgColor = nullptr);
    void drawTexture(const Texture* texture, uint32_t flags, float sizeX, float sizeY, bool keepAspect, float edge = 0.1f, const glm::vec2& uvMin = { 0, 0 }, const glm::vec2& uvMax = { 1, 1 }, uint8_t* extraFlags = nullptr, uint64_t* overrideId = nullptr, uint32_t* overrideHash = nullptr, Color32* bgColor = nullptr);


    bool drawProgressBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col, const ImU32& hi_col_lhs);
    bool drawProgressSpinner(const char* label, float radius, float thickness, const ImU32& color);

    template<typename T>
    bool drawInputInt(const char* label, T& value, int32_t step = 1, int32_t stepFast = 100, uint32_t flags = 0) {
        int32_t valueInt = int32_t(value);

        if (ImGui::InputInt(label, &valueInt, step, stepFast, flags)) {
            value = T(valueInt);
            return true;
        }
        return false;
    }
}
