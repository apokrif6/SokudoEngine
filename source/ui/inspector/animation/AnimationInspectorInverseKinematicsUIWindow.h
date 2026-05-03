#pragma once

#include "ui/UIWindow.h"
#include "imgui.h"
#include <ranges>

namespace Core::UI
{
class AnimationInspectorInverseKinematicsUIWindow : public UIWindow<AnimationInspectorInverseKinematicsUIWindow>
{
    friend class UIWindow;
    static bool getBody(Component::MeshComponent* meshComponent)
    {
        ImGui::SeparatorText("InverseKinematics");

        auto& solvers = meshComponent->getIKSolvers();
        if (solvers.empty())
        {
            ImGui::TextDisabled("No active IK solvers.");
        }
        else
        {
            for (int i = 0; i < solvers.size(); ++i)
            {
                ImGui::PushID(i);
                if (ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(i)),
                                      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed,
                                      "CCD Solver - %zu bones", solvers[i]->getChainIndices().size()))
                {
                    int iterations = solvers[i]->getMaxIterations();
                    if (ImGui::SliderInt("Iterations", &iterations, 1, 50))
                    {
                        solvers[i]->setMaxIterations(iterations);
                    }
                    if (ImGui::Button("Remove Solver", ImVec2(-1, 0)))
                    {
                        meshComponent->removeIKSolver(i);
                        ImGui::TreePop();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
        }
        ImGui::Spacing();
        if (ImGui::Button("Add New IK Chain...", ImVec2(-1, 0)))
        {
            ImGui::OpenPopup("CreateIKSolverPopup");
        }
        if (ImGui::BeginPopupModal("CreateIKSolverPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            const auto& boneMap = meshComponent->getSkeleton().getSkeletonData()->boneNameToIndexMap;
            static std::vector<const char*> boneNames;
            if (boneNames.empty())
            {
                for (const auto& name : boneMap | std::views::keys)
                {
                    boneNames.push_back(name.c_str());
                }
            }
            static int startBoneIndex = 0;
            static int endBoneIndex = 0;
            ImGui::Text("Build IK Chain from hierarchy:");
            ImGui::Combo("Start (Root)", &startBoneIndex, boneNames.data(), boneNames.size());
            ImGui::Combo("End (Effector)", &endBoneIndex, boneNames.data(), boneNames.size());
            ImGui::Separator();
            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                int realStart = boneMap.at(boneNames[startBoneIndex]);
                int realEnd = boneMap.at(boneNames[endBoneIndex]);
                std::vector<int> chain = meshComponent->getSkeleton().buildBonesChain(realStart, realEnd);
                if (!chain.empty())
                {
                    auto newSolver = std::make_unique<Animations::IKSolverCCD>(chain, 15);
                    meshComponent->addIKSolver(std::move(newSolver));
                    ImGui::CloseCurrentPopup();
                }
                else
                {
                    Logger::log(1, "IK Error: Bones are not in the same branch!");
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGui::Button("Find and set IK Target"))
        {
            meshComponent->TEST_findAndSetIKTarget();
        }

        return true;
    }
};
} // namespace Core::UI