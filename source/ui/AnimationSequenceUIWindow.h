#pragma once

#include "ui/UIWindow.h"
#include "imgui.h"
#include "engine/Engine.h"
#include "scene/Scene.h"
#include "components/MeshComponent.h"
#include "animations/AnimationsData.h"

namespace Core::UI
{
class AnimationSequenceUIWindow : public UIWindow<AnimationSequenceUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        if (!ImGui::Begin("Animation Sequence"))
        {
            ImGui::End();
            return false;
        }

        auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
        const auto selectedObject = scene->getSceneObjectSelection().selectedObject.lock();

        if (!selectedObject)
        {
            ImGui::TextDisabled("Select an object in the Scene Hierarchy...");
            ImGui::End();
            return true;
        }

        auto* meshComponent = selectedObject->getComponent<Component::MeshComponent>();
        if (!meshComponent || !meshComponent->hasAnimations())
        {
            ImGui::Text("Selected object '%s' has no mesh or animations.", selectedObject->getName().c_str());
            ImGui::End();
            return true;
        }

        drawGeneralInfo(meshComponent);

        ImGui::Separator();

        bool shouldDrawDebugSkeleton = meshComponent->shouldDrawDebugSkeleton();
        if (ImGui::Checkbox("Should draw debug skeleton", &shouldDrawDebugSkeleton))
        {
            meshComponent->setShouldDrawDebugSkeleton(shouldDrawDebugSkeleton);
        }

        if (ImGui::CollapsingHeader("Animation Masks", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Create New Mask"))
            {
                Animations::AnimationMask newMask;
                newMask.name = "Mask_" + std::to_string(meshComponent->getMasksCount());
                meshComponent->addMask(newMask);
            }

            const int currentMask = meshComponent->getCurrentMaskIndex();
            ImGui::Spacing();
            ImGui::Text("Active Mask:");
            ImGui::PushItemWidth(-1.0f);
            if (ImGui::BeginCombo(
                    "###ActiveMaskCombo",
                    (currentMask == -1 ? "None (Global)" : meshComponent->getMaskName(currentMask)).c_str()))
            {
                if (ImGui::Selectable("None (Global)", currentMask == -1))
                {
                    meshComponent->setMaskIndex(-1);
                }
                for (int i = 0; i < meshComponent->getMasksCount(); i++)
                {
                    if (ImGui::Selectable(meshComponent->getMaskName(i).c_str(), currentMask == i))
                    {
                        meshComponent->setMaskIndex(i);
                    }
                }
                ImGui::EndCombo();
            }
        }

        if (ImGui::CollapsingHeader("Skeleton Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
        {
            const auto& skeleton = meshComponent->getSkeleton();
            drawBoneNodeRecursive(skeleton.getRootNode(), meshComponent);
        }

        ImGui::End();

        return true;
    }

    static void drawGeneralInfo(Component::MeshComponent* meshComponent)
    {
        const auto& animations = meshComponent->getAnimations();
        const int currentIndex = meshComponent->getCurrentAnimationIndex();
        const auto& currentClip = animations[currentIndex];
        float timeInTicks = meshComponent->getCurrentAnimationTime();
        const float duration = currentClip.duration;
        const std::string overlay = std::format("{:.1f} / {:.1f}", timeInTicks, duration);

        ImGui::BulletText("Clip Name: %s", currentClip.name.c_str());
        ImGui::BulletText("Duration: %.2f ticks", duration);
        ImGui::BulletText("Ticks Per Second: %.1f", currentClip.ticksPerSecond);

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
    }

    static void drawBoneNodeRecursive(const Animations::BoneNode& node, Component::MeshComponent* meshComponent)
    {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

        int boneIndex = -1;
        bool isActualBone = false;
        if (!meshComponent->getPrimitives().empty())
        {
            const auto& bonesMap = meshComponent->getSkeleton().getSkeletonData()->boneNameToIndexMap;
            if (const auto it = bonesMap.find(node.name); it != bonesMap.end())
            {
                isActualBone = true;
                boneIndex = it->second;
            }
        }

        if (node.children.empty())
        {
            flags |= ImGuiTreeNodeFlags_Leaf;
        }

        if (isActualBone)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.4f, 1.0f));
        }

        const auto nodeID = reinterpret_cast<void*>(std::hash<std::string>{}(node.name));

        const bool opened = ImGui::TreeNodeEx(nodeID, flags, "%s", node.name.c_str());

        if (isActualBone)
        {
            ImGui::PopStyleColor();
        }

        if (const int currentMaskIdx = meshComponent->getCurrentMaskIndex(); currentMaskIdx != -1)
        {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 380.0f);

            auto& mask = meshComponent->getMask(currentMaskIdx);
            float& weight = mask.boneWeights[node.name];

            ImGui::PushItemWidth(80.0f);

            const std::string sliderLabel = "##w_" + node.name;
            ImGui::SliderFloat(sliderLabel.c_str(), &weight, 0.0f, 1.0f, "%.1f");
            ImGui::PopItemWidth();

            ImGui::SameLine();
            const std::string buttonLabel = "V##btn_" + node.name;
            if (ImGui::Button(buttonLabel.c_str()))
            {
                mask.setWeightRecursively(node, weight, true);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Apply this weight to '%s' and all its children", node.name.c_str());
            }
        }

        if (isActualBone)
        {
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 250.0f);

            const auto& boneData = meshComponent->getPrimitives()[0].getBonesInfo().bones[boneIndex];

            const Animations::BoneTransform transform{boneData.animatedGlobalTransform};

            const glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.rotation));

            ImGui::TextDisabled("P: %.1f %.1f %.1f | R: %.0f %.0f %.0f", transform.position.x, transform.position.y,
                                transform.position.z, euler.x, euler.y, euler.z);

            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Position: %.3f, %.3f, %.3f\nRotation (Euler): %.2f, %.2f, %.2f",
                                  transform.position.x, transform.position.y, transform.position.z, euler.x, euler.y,
                                  euler.z);
            }
        }

        if (opened)
        {
            for (const auto& child : node.children)
            {
                drawBoneNodeRecursive(child, meshComponent);
            }
            ImGui::TreePop();
        }
    }
};
} // namespace Core::UI