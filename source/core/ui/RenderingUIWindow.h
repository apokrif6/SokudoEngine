#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "core/engine/Engine.h"

namespace Core::UI
{
class RenderingUIWindow : public UIWindow<RenderingUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Rendering"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

        ImGui::Checkbox("Should draw skybox", &renderData.shouldDrawSkybox);
        ImGui::Checkbox("Should draw grid", &renderData.shouldDrawGrid);

        ImGui::EndTabItem();

        return true;
    }
};
} // namespace Core::UI