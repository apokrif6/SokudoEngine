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
            return false;
        }

        std::shared_ptr<Core::Renderer::Mesh> meshObject = std::static_pointer_cast<Core::Renderer::Mesh>(
            Core::Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection().selectedObject);
        if (!meshObject)
        {
            ImGui::Text("No mesh object selected");
            ImGui::EndTabItem();
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

        std::vector<std::string>& loadedAnimations =
            Core::Engine::getInstance().getSystem<Animations::Animator>()->loadedAnimations;
        if (ImGui::BeginCombo("##Loaded animations", loadedAnimations[selectedAnimationIndex].c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedAnimations.size(); ++i)
            {
                const bool isAnimationToPlaySelected = (selectedAnimationIndex == i);
                if (ImGui::Selectable(loadedAnimations[i].c_str(), isAnimationToPlaySelected))
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

        ImGui::EndTabItem();

        return true;
    }

  private:
    static inline int selectedAnimationIndex = 0;
};
} // namespace Core::UI
