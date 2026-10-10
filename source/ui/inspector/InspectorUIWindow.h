#pragma once

#include "imgui.h"

#include "components/ComponentRegistry.h"
#include "scene/objects/SceneObject.h"
#include "engine/Engine.h"

namespace Core::UI
{
class InspectorUIWindow : public UIPermanentWindow<InspectorUIWindow>
{
    friend class UIWindow;
    friend class UIPermanentWindow;

    static bool getBody()
    {
        ImGui::Begin("Inspector");

        auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
        auto& selection = scene->getSceneObjectSelection();

        if (const auto selectedObject = selection.selectedObject.lock())
        {
            ImGui::TextDisabled("(SceneObject UUID: %s)", uuids::to_string(selectedObject->getUUID()).c_str());

            ImGui::Separator();
            ImGui::Spacing();

            for (const auto& descriptor : Component::ComponentRegistry::getAll())
            {
                if (!descriptor.contains(*selectedObject))
                {
                    continue;
                }

                if (ImGui::CollapsingHeader(descriptor.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    descriptor.renderInspector();
                }
            }

            if (ImGui::BeginPopupContextWindow("AddComponentContext", ImGuiPopupFlags_MouseButtonRight))
            {
                for (const auto& descriptor : Component::ComponentRegistry::getAll())
                {
                    if (!descriptor.addable)
                    {
                        continue;
                    }

                    if (const bool alreadyExists = descriptor.contains(*selectedObject))
                    {
                        continue;
                    }

                    if (ImGui::MenuItem(descriptor.name.c_str()))
                    {
                        descriptor.add(*selectedObject);
                    }
                }

                ImGui::EndPopup();
            }
        }
        else
        {
            ImGui::TextDisabled("No object selected");
        }

        ImGui::End();
        return true;
    }
};
} // namespace Core::UI
