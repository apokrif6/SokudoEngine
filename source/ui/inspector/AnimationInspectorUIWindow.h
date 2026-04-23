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

        const std::vector<Animations::AnimationClip>& loadedAnimations = meshComponent->getAnimations();
        const int currentAnimationIndex = meshComponent->getCurrentAnimationIndex();
        ImGui::Spacing();
        ImGui::Text("Current Animation:");
        ImGui::PushItemWidth(-1.0f);
        if (ImGui::BeginCombo("##Loaded animations", loadedAnimations[currentAnimationIndex].name.c_str()))
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

        ImGui::PopItemWidth();

        ImGui::Spacing();
        bool shouldBlend = meshComponent->shouldBlendAnimations();
        if (ImGui::Checkbox("Enable Blending", &shouldBlend))
        {
            meshComponent->setShouldBlendAnimations(shouldBlend);
        }

        if (shouldBlend)
        {
            ImGui::Indent();

            const int targetAnimationIndex = meshComponent->getTargetAnimationIndex();
            ImGui::Text("Target Animation:");
            ImGui::PushItemWidth(-1.0f);
            if (ImGui::BeginCombo("##Target animations", loadedAnimations[targetAnimationIndex].name.c_str()))
            {
                for (int i = 0; i < loadedAnimations.size(); ++i)
                {
                    const bool isAnimationToPlaySelected = targetAnimationIndex == i;
                    if (ImGui::Selectable(loadedAnimations[i].name.c_str(), isAnimationToPlaySelected))
                    {
                        meshComponent->setTargetAnimationIndex(i);
                    }

                    if (isAnimationToPlaySelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Blending Mode:");
            const auto blendingMode = meshComponent->getBlendingMode();
            if (ImGui::RadioButton("Crossfade", blendingMode == Animations::AnimationBlendingMode::Crossfade))
            {
                meshComponent->setBlendingMode(Animations::AnimationBlendingMode::Crossfade);
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("Masked Blend", blendingMode == Animations::AnimationBlendingMode::Masked))
            {
                meshComponent->setBlendingMode(Animations::AnimationBlendingMode::Masked);
            }

            ImGui::Spacing();

            if (blendingMode == Animations::AnimationBlendingMode::Crossfade)
            {
                float blendFactor = meshComponent->getBlendFactor();
                ImGui::Text("Global Fade Factor:");
                if (ImGui::SliderFloat("##BlendFactor", &blendFactor, 0.0f, 1.0f, "%.2f"))
                {
                    meshComponent->setBlendFactor(blendFactor);
                }
            }
            else
            {
                if (const int maskIndex = meshComponent->getCurrentMaskIndex(); maskIndex == -1)
                {
                    ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Warning: No mask selected!");
                    ImGui::TextWrapped("Go to 'Animation Sequence' tab to create/select a mask");
                }
                else
                {
                    ImGui::Text("Active Mask: %s", meshComponent->getMaskName(maskIndex).c_str());
                }
            }
            ImGui::Unindent();
        }

        return true;
    }
};
} // namespace Core::UI
