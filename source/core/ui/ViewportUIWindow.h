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

        ImGui::Image(reinterpret_cast<ImTextureID>(Engine::getInstance().getRenderData().rdViewportTarget.descriptorSet),
            ImGui::GetContentRegionAvail());

        ImGui::End();

        return true;
    }
};
}  // namespace Core::UI