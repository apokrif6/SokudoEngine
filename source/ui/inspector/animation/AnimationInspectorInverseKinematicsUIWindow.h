#pragma once

#include "ui/UIWindow.h"
#include "imgui.h"
#include "animations/ik/IKSolverCCD.h"
#include "animations/ik/IKSolverFABRIK.h"
#include "ui/ComponentPickerUIWindow.h"

#include <ranges>
#include <vector>
#include <string>
#include <unordered_map>

namespace Core::UI
{
class AnimationInspectorInverseKinematicsUIWindow : public UIWindow<AnimationInspectorInverseKinematicsUIWindow>
{
    friend class UIWindow;

    static bool getBody(Component::MeshComponent* meshComponent)
    {
        ImGui::SeparatorText("InverseKinematics");

        if (auto& solvers = meshComponent->getIKSolvers(); solvers.empty())
        {
            ImGui::TextDisabled("No active IK solvers.");
        }
        else
        {
            for (int i = 0; i < solvers.size(); ++i)
            {
                const auto& solver = solvers[i];
                const auto& chain = solver->getChainIndices();

                const std::string solverTypePrefix =
                    solver->getType() == Animations::AnimationSolverType::FABRIK ? "FABRIK" : "CCD";

                const std::string startBone = meshComponent->getSkeleton().getBoneName(chain.back());
                const std::string endBone = meshComponent->getSkeleton().getBoneName(chain.front());

                ImGui::PushID(i);
                if (ImGui::TreeNodeEx(reinterpret_cast<void*>(static_cast<intptr_t>(i)),
                                      ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed,
                                      "%s Solver - %zu bones {%s} -> {%s}", solverTypePrefix.c_str(), chain.size(),
                                      startBone.c_str(), endBone.c_str()))
                {
                    int iterations = static_cast<int>(solver->getMaxIterations());
                    if (ImGui::SliderInt("Iterations", &iterations, 1, 50))
                    {
                        solver->setMaxIterations(iterations);
                    }
                    if (ImGui::Button("Remove Solver", ImVec2(-1, 0)))
                    {
                        meshComponent->removeIKSolver(i);
                        ImGui::TreePop();
                        ImGui::PopID();
                        break;
                    }
                    ComponentPicker::Render<Component::IKTargetComponent>(
                        "Target", solver->getTargetUUID(), Engine::getInstance().getSystem<Scene::Scene>(),
                        [&](const uuids::uuid& selectedUUID)
                        {
                            solver->setTargetUUID(selectedUUID);
                        });
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
            const auto& skeletonData = meshComponent->getSkeleton().getSkeletonData();

            static int startIndex = -1;
            static int endIndex = -1;

            ImGui::Text("Build IK Chain: Select Root and Effector");
            ImGui::Separator();

            if (ImGui::BeginTable("IKSelectionTable", 2,
                                  ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchSame))
            {
                constexpr float childHeight = 400.0f;
                constexpr float childWidth = 500.0f;

                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "Chain Root (Start Bone):");
                ImGui::BeginChild("RootTree", ImVec2(childWidth, childHeight), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);

                const int oldStartIndex = startIndex;
                drawBoneHierarchy(skeletonData->rootNode, startIndex, skeletonData->boneNameToIndexMap);
                if (oldStartIndex != startIndex)
                {
                    endIndex = -1;
                }
                ImGui::EndChild();

                ImGui::TableNextColumn();
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "Effector (End Bone):");
                ImGui::BeginChild("EffectorTree", ImVec2(childWidth, childHeight), true,
                                  ImGuiWindowFlags_HorizontalScrollbar);

                if (startIndex != -1)
                {
                    if (const Animations::BoneNode* startNode =
                            findNodeRecursive(&skeletonData->rootNode, startIndex, skeletonData->boneNameToIndexMap))
                    {
                        drawBoneHierarchy(*startNode, endIndex, skeletonData->boneNameToIndexMap);
                    }
                }
                else
                {
                    ImGui::TextDisabled("Select Root bone first...");
                }

                ImGui::EndChild();

                ImGui::EndTable();
            }

            ImGui::Separator();

            ImGui::Text("Solver:");
            static auto solveType = Animations::AnimationSolverType::FABRIK;
            if (ImGui::RadioButton("FABRIK", solveType == Animations::AnimationSolverType::FABRIK))
            {
                solveType = Animations::AnimationSolverType::FABRIK;
            }
            ImGui::SameLine();
            if (ImGui::RadioButton("CCD", solveType == Animations::AnimationSolverType::CCD))
            {
                solveType = Animations::AnimationSolverType::CCD;
            }

            const bool canCreate = startIndex != -1 && endIndex != -1;
            if (!canCreate)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Create", ImVec2(120, 0)))
            {
                if (const std::vector<int> chain = meshComponent->getSkeleton().buildBonesChain(startIndex, endIndex);
                    !chain.empty())
                {
                    std::unique_ptr<Animations::IIKSolver> newSolver;

                    if (solveType == Animations::AnimationSolverType::FABRIK)
                    {
                        newSolver = std::make_unique<Animations::IKSolverFABRIK>(chain);
                    }
                    else
                    {
                        newSolver = std::make_unique<Animations::IKSolverCCD>(chain);
                    }
                    meshComponent->addIKSolver(std::move(newSolver));

                    startIndex = -1;
                    endIndex = -1;
                    ImGui::CloseCurrentPopup();
                }
            }
            if (!canCreate)
            {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                startIndex = -1;
                endIndex = -1;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return true;
    }

    static void drawBoneHierarchy(const Animations::BoneNode& node, int& selectedIndex,
                                  const std::unordered_map<std::string, int>& nameToIndex)
    {
        int boneIndex = -1;
        if (const auto it = nameToIndex.find(node.name); it != nameToIndex.end())
        {
            boneIndex = it->second;
        }
        if (boneIndex != -1)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                       ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

            if (selectedIndex == boneIndex)
            {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            if (node.children.empty())
            {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            ImGui::PushID(boneIndex);
            const bool open = ImGui::TreeNodeEx(node.name.c_str(), flags, "%s", node.name.c_str());

            if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {
                selectedIndex = boneIndex;
            }

            if (open)
            {
                for (const auto& child : node.children)
                {
                    drawBoneHierarchy(child, selectedIndex, nameToIndex);
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        else
        {
            for (const auto& child : node.children)
            {
                drawBoneHierarchy(child, selectedIndex, nameToIndex);
            }
        }
    }

    static const Animations::BoneNode* findNodeRecursive(const Animations::BoneNode* current, int targetIdx,
                                                         const std::unordered_map<std::string, int>& nameToIndex)
    {
        if (const auto it = nameToIndex.find(current->name); it != nameToIndex.end())
        {
            if (it->second == targetIdx)
            {
                return current;
            }
        }

        for (const auto& child : current->children)
        {
            if (const auto found = findNodeRecursive(&child, targetIdx, nameToIndex))
            {
                return found;
            }
        }
        return nullptr;
    }
};
} // namespace Core::UI