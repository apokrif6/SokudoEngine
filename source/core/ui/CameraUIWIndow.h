#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include <string>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"
#include "core/engine/Engine.h"

namespace Core::UI
{
class CameraUIWindow : public UIWindow<CameraUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Camera"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

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

        ImGui::EndTabItem();

        return true;
    }
};
}
