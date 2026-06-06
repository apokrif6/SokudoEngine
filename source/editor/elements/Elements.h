#pragma once

#include <imgui.h>

namespace Editor::UI::Elements
{
void nodeSeparator(const float width)
{
    const ImVec2 position = ImGui::GetCursorScreenPos();

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    const ImU32 color = ImGui::GetColorU32(ImGuiCol_Separator);

    const float thickness =
        ImGui::GetStyle().SeparatorTextBorderSize > 0.0f ? ImGui::GetStyle().SeparatorTextBorderSize : 1.0f;

    ImGui::Dummy(ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));

    drawList->AddLine(ImVec2(position.x, position.y), ImVec2(position.x + width, position.y), color, thickness);

    ImGui::Dummy(ImVec2(0.0f, ImGui::GetStyle().ItemSpacing.y));
}
} // namespace Editor::UI::Elements