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

            float blendFactor = meshComponent->getBlendFactor();
            ImGui::Text("Blend Factor (A -> B):");
            if (ImGui::SliderFloat("##BlendFactor", &blendFactor, 0.0f, 1.0f, "%.2f"))
            {
                meshComponent->setBlendFactor(blendFactor);
            }

            ImGui::Unindent();
        }

        return true;
    }
};
} // namespace Core::UI
