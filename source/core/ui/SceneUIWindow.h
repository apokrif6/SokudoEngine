#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"

namespace Core::UI
{
class SceneUIWindow : public UIWindow<SceneUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Scene"))
        {
            return false;
        }

        Renderer::VkRenderData renderData = Core::Engine::getInstance().getRenderData();

        ImGui::Text("Field Of View");
        ImGui::SameLine();
        ImGui::SliderInt("FOV", &renderData.rdFieldOfView, 40, 150);

        if (ImGui::Button("Reset Rotation"))
        {
            renderData.rdResetAngles = true;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        ImGui::Text("X Rotation");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::SliderInt("##ROTX", &renderData.rdRotXAngle, 0, 360);

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
        ImGui::Text("Y Rotation");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::SliderInt("##ROTY", &renderData.rdRotYAngle, 0, 360);

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 255, 255));
        ImGui::Text("Z Rotation");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::SliderInt("##ROTZ", &renderData.rdRotZAngle, 0, 360);

        ImGui::EndTabItem();

        return true;
    }
};
}

