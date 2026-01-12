#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "glm/gtx/string_cast.hpp"

namespace Core::UI
{
class MiscUIWindow : public UIWindow<MiscUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        if (!ImGui::Begin("Misc"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Engine::getInstance().getRenderData();

        ImGui::Checkbox("Should draw skybox", &renderData.shouldDrawSkybox);
        ImGui::Checkbox("Should draw grid", &renderData.shouldDrawGrid);

        ImGui::Separator();

        ImGui::Text("Field Of View");
        ImGui::SameLine();
        ImGui::SliderInt("FOV", &renderData.rdFieldOfView, 40, 150);

        ImGui::Text("Camera Position:");
        ImGui::SameLine();
        ImGui::Text("%s", glm::to_string(renderData.rdCameraWorldPosition).c_str());

        ImGui::Text("View Yaw:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdViewYaw).c_str());

        ImGui::Text("View Pitch:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdViewPitch).c_str());

        std::string windowDims = std::to_string(renderData.rdWidth) + "x" + std::to_string(renderData.rdHeight);
        ImGui::Text("Window Dimensions:");
        ImGui::SameLine();
        ImGui::Text("%s", windowDims.c_str());

        ImGui::End();

        return true;
    }
};
} // namespace Core::UI