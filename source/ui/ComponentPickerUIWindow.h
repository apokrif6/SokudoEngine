#pragma once

#include "imgui.h"
#include "scene/Scene.h"
#include <functional>
#include <string>

namespace Core::UI
{
class ComponentPicker
{
public:
    template <typename T>
    static void Render(const std::string& label, const uuids::uuid& currentUUID, Scene::Scene* scene,
                       const std::function<void(const uuids::uuid&)>& onSelectedCallback)
    {
        std::string targetLabel = "None (Click to select)";

        if (!currentUUID.is_nil())
        {
            if (const auto object = scene->findComponentByUUID(currentUUID))
            {
                targetLabel = object->getOwner()->getName();
            }
            else
            {
                targetLabel = "Missing component";
            }
        }

        ImGui::Text("%s", label.c_str());
        ImGui::SameLine();

        const std::string popupID = "PickerPopup_" + label;
        if (ImGui::Button((targetLabel + "##" + label).c_str(), ImVec2(-1, 0)))
        {
            ImGui::OpenPopup(popupID.c_str());
        }

        if (ImGui::BeginPopup(popupID.c_str()))
        {
            ImGui::TextDisabled("Components of type %s", typeid(T).name());
            ImGui::Separator();

            for (const auto components = scene->getAllComponentsOfType<T>(); auto* component : components)
            {
                if (!component || !component->getOwner())
                {
                    continue;
                }

                const auto& owner = component->getOwner();

                const bool selected = component->getUUID() == currentUUID;

                std::string labelText = owner->getName() + " [" + std::string(typeid(T).name()) + "]";

                if (ImGui::Selectable(labelText.c_str(), selected))
                {
                    onSelectedCallback(component->getUUID());
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::Separator();

            if (ImGui::Selectable("Clear", currentUUID.is_nil()))
            {
                onSelectedCallback(uuids::uuid{});
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
};
} // namespace Core::UI