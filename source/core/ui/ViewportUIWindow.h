#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"
#include "core/profiling/PlotBuffer.h"

namespace Core::UI
{
class ViewportUIWindow : public UIWindow<ViewportUIWindow>
{
public:
    static bool getBody()
    {
        if (!ImGui::Begin("Viewport"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

        auto viewportSize = ImGui::GetContentRegionAvail();
        if (renderData.rdViewportTarget.size.x != static_cast<int>(viewportSize.x) ||
            renderData.rdViewportTarget.size.y != static_cast<int>(viewportSize.y))
        {
            Core::Engine::getInstance().getSystem<Renderer::VkRenderer>()->resizeViewportTarget({viewportSize.x, viewportSize.y});
        }

        ImGui::Image(reinterpret_cast<ImTextureID>(renderData.rdViewportTarget.descriptorSet),
                     viewportSize);

        ImGui::End();

        return true;
    }
};
}  // namespace Core::UI