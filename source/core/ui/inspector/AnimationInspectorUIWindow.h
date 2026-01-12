#pragma once

#include "core/ui/UIWindow.h"
#include "imgui.h"
#include "string"

namespace Core::UI
{
class AnimationInspectorUIWindow : public UIWindow<AnimationInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody(Component::MeshComponent* meshComponent)
    {
        bool shouldPlayAnimation = meshComponent->shouldPlayAnimation();
        if (ImGui::Checkbox("Should play animation", &shouldPlayAnimation))
        {
            meshComponent->setShouldPlayAnimation(shouldPlayAnimation);
        }

        bool shouldDrawDebugSkeleton = meshComponent->shouldDrawDebugSkeleton();
        if (ImGui::Checkbox("Should draw debug skeleton", &shouldDrawDebugSkeleton))
        {
            meshComponent->setShouldDrawDebugSkeleton(shouldDrawDebugSkeleton);
        }

        const std::vector<Animations::AnimationClip>& loadedAnimations = meshComponent->getAnimations();
        int currentAnimationIndex = meshComponent->getCurrentAnimationIndex();
        if (ImGui::BeginCombo("##Loaded animations", loadedAnimations[currentAnimationIndex].name.c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedAnimations.size(); ++i)
            {
                const bool isAnimationToPlaySelected = currentAnimationIndex == i;
                if (ImGui::Selectable(loadedAnimations[i].name.c_str(), isAnimationToPlaySelected))
                {
                    meshComponent->setCurrentAnimationIndex(currentAnimationIndex);
                }

                if (isAnimationToPlaySelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        return true;
    }
};
} // namespace Core::UI
