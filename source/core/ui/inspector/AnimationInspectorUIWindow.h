#pragma once

#include "core/ui/UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"

namespace Core::UI
{
class AnimationInspectorUIWindow : public UIWindow<AnimationInspectorUIWindow>
{
public:
    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        std::shared_ptr<Renderer::Mesh> meshObject =
            std::static_pointer_cast<Renderer::Mesh>(selectedObject);
        if (!meshObject)
        {
            return true;
        }

        bool shouldPlayAnimation = meshObject->shouldPlayAnimation();
        if (ImGui::Checkbox("Should play animation", &shouldPlayAnimation))
        {
            meshObject->setShouldPlayAnimation(shouldPlayAnimation);
        }

        bool shouldDrawDebugSkeleton = meshObject->shouldDrawDebugSkeleton();
        if (ImGui::Checkbox("Should draw debug skeleton", &shouldDrawDebugSkeleton))
        {
            meshObject->setShouldDrawDebugSkeleton(shouldDrawDebugSkeleton);
        }

        const std::vector<Animations::AnimationClip>& loadedAnimations = meshObject->getAnimations();
        if (ImGui::BeginCombo("##Loaded animations", loadedAnimations[selectedAnimationIndex].name.c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedAnimations.size(); ++i)
            {
                const bool isAnimationToPlaySelected = (selectedAnimationIndex == i);
                if (ImGui::Selectable(loadedAnimations[i].name.c_str(), isAnimationToPlaySelected))
                {
                    selectedAnimationIndex = i;

                    meshObject->setCurrentAnimationIndex(selectedAnimationIndex);
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

private:
    static inline int selectedAnimationIndex = 0;
};
} // namespace Core::UI
