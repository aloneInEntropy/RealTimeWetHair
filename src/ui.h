#ifndef UI_H
#define UI_H

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_internal.h"
// #ifndef IMGUI_DISABLE
// #endif
#include "util.h"

// Wrapper class for ImGui. Adds nice-to-haves, such as per-component ranges for sliders
namespace UI {
bool sliderFloatN(const char* label, ImGuiDataType data_type, void* v, int components, const void* v_min, const void* v_max, const char* format, float power) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());
    size_t type_size = sizeof(float);
    for (int i = 0; i < components; i++) {
        ImGui::PushID(i);
        if (i > 0)
            ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
        value_changed |= ImGui::SliderScalar("", data_type, v, v_min, v_max, format, power);
        ImGui::PopID();
        ImGui::PopItemWidth();
        v = (void*)((char*)v + type_size);
        v_min = (void*)((char*)v_min + type_size);
        v_max = (void*)((char*)v_max + type_size);
    }
    ImGui::PopID();

    const char* label_end = ImGui::FindRenderedTextEnd(label);
    if (label != label_end) {
        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
        ImGui::TextEx(label, label_end);
    }

    ImGui::EndGroup();
    return value_changed;
}

// Equivalent of ImGui's SliderFloat2, except that `v_min` and `v_max` are arrays of length 2 for individual component ranges.
// Returns `true` if the value of the slider changed.
bool sliderFloat2(const char* label, float v[2], float v_min[2], float v_max[2], const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
    return sliderFloatN(label, ImGuiDataType_Float, v, 2, v_min, v_max, format, flags);
}
// Equivalent of ImGui's SliderFloat3, except that `v_min` and `v_max` are arrays of length 3 for individual component ranges.
// Returns `true` if the value of the slider changed.
bool sliderFloat3(const char* label, float v[3], float v_min[3], float v_max[3], const char* format = "%.3f", ImGuiSliderFlags flags = 0) {
    return sliderFloatN(label, ImGuiDataType_Float, v, 3, v_min, v_max, format, flags);
}
// Equivalent of ImGui's Text. Can be used anywhere.
template <typename... Args>
void text(const char* fmt, Args... args) {
    ImGui::Text(fmt, args...);
}
// Add a help marker to the same line as the previous statement.
// Taken from imgui_demo.cpp, line 239
void Help(const char* desc) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
}  // namespace UI

#endif /* UI_H */
