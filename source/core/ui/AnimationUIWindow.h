#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"

namespace Core::UI
{
class AnimationUIWindow : public UIWindow<AnimationUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Animation"))
        {
            return true;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

        ImGui::Checkbox("Should play animation", &renderData.shouldPlayAnimation);
        ImGui::Checkbox("Should draw debug skeleton", &renderData.shouldDrawDebugSkeleton);

        std::vector<std::string>& loadedAnimations =
            Core::Engine::getInstance().getSystem<Animations::Animator>()->loadedAnimations;
        if (ImGui::BeginCombo("##Loaded animations", loadedAnimations[renderData.selectedAnimationIndexToPlay].c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedAnimations.size(); ++i)
            {
                const bool isAnimationToPlaySelected = (renderData.selectedAnimationIndexToPlay == i);
                if (ImGui::Selectable(loadedAnimations[i].c_str(), isAnimationToPlaySelected))
                {
                    renderData.selectedAnimationIndexToPlay = i;
                }

                if (isAnimationToPlaySelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::EndTabItem();

        return true;
    }
};
} // namespace Core::UI
