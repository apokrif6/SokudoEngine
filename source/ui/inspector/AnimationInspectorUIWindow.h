#pragma once

#include "ui/UIWindow.h"
#include "imgui.h"
#include "string"
#include "nfd.hpp"

namespace Core::UI
{
class AnimationInspectorUIWindow : public UIWindow<AnimationInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody(Component::MeshComponent* meshComponent)
    {
        if (ImGui::Button("Import Animation"))
        {
            NFD::Init();

            nfdfilteritem_t filterItem[1] = {{"Animation Files", "fbx,gltf,glb,dae"}};
            NFD::UniquePath outPath;

            const nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1, nullptr);

            if (result == NFD_OKAY)
            {
                const std::string path = outPath.get();

                meshComponent->loadAnimationFromFile(path);
            }
            else if (result == NFD_ERROR)
            {
                Logger::log(1, "NFD Error: %s", NFD::GetError());
            }

            NFD::Quit();
        }

        if (!meshComponent->hasAnimations())
        {
            return true;
        }

        bool shouldDrawDebugSkeleton = meshComponent->shouldDrawDebugSkeleton();
        if (ImGui::Checkbox("Should draw debug skeleton", &shouldDrawDebugSkeleton))
        {
            meshComponent->setShouldDrawDebugSkeleton(shouldDrawDebugSkeleton);
        }

        const std::vector<Animations::AnimationClip>& loadedAnimations = meshComponent->getAnimations();
        int currentAnimationIndex = meshComponent->getCurrentAnimationIndex();
        ImGui::Text("Active Clip:");
        if (ImGui::BeginCombo("##Loaded animations", loadedAnimations[currentAnimationIndex].name.c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedAnimations.size(); ++i)
            {
                const bool isAnimationToPlaySelected = currentAnimationIndex == i;
                if (ImGui::Selectable(loadedAnimations[i].name.c_str(), isAnimationToPlaySelected))
                {
                    meshComponent->setCurrentAnimationIndex(i);
                    meshComponent->setAnimationTime(0.f);
                }

                if (isAnimationToPlaySelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        const auto& currentClip = loadedAnimations[currentAnimationIndex];
        float timeInTicks = meshComponent->getCurrentAnimationTime();
        const float duration = currentClip.duration;
        const std::string overlay = std::format("{:.1f} / {:.1f}", timeInTicks, duration);

        ImGui::Text("Timeline (Ticks)");
        if (ImGui::SliderFloat("##TimelineSlider", &timeInTicks, 0.0f, duration, overlay.c_str()))
        {
            meshComponent->setShouldPlayAnimation(false);
            meshComponent->setAnimationTime(timeInTicks);
        }

        if (meshComponent->shouldPlayAnimation())
        {
            if (ImGui::Button("Pause"))
            {
                meshComponent->setShouldPlayAnimation(false);
            }
        }
        else
        {
            if (ImGui::Button("Play"))
            {
                meshComponent->setShouldPlayAnimation(true);
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Reset"))
        {
            meshComponent->setAnimationTime(0.0f);
        }

        ImGui::TextDisabled("Ticks Per Second: %.1f", currentClip.ticksPerSecond);

        return true;
    }
};
} // namespace Core::UI
