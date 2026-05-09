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
    static void Render(const std::string& label, uint64_t& currentUUID, Scene::Scene* scene,
                       const std::function<void(uint64_t)> onSelectedCallback)
    {
        std::string targetLabel = "None (Click to select)";

        if (currentUUID != 0)
        {
            if (const auto object = scene->findObjectByUUID(currentUUID))
            {
                targetLabel = object->getName();
            }
            else
            {
                targetLabel = "Missing Object (" + std::to_string(currentUUID) + ")";
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
            ImGui::TextDisabled("Select Object with %s", typeid(T).name());
            ImGui::Separator();

            const auto objects = scene->getObjects();

            std::function<void(const std::vector<std::shared_ptr<Scene::SceneObject>>&)> drawLevel;
            drawLevel = [&](const std::vector<std::shared_ptr<Scene::SceneObject>>& levelObjects)
            {
                for (auto& object : levelObjects)
                {
                    if (object->getComponent<T>())
                    {
                        if (const bool isSelected = currentUUID == object->getUUID();
                            ImGui::Selectable(object->getName().c_str(), isSelected))
                        {
                            currentUUID = object->getUUID();
                            onSelectedCallback(currentUUID);
                            ImGui::CloseCurrentPopup();
                        }
                    }

                    if (!object->getChildren().empty())
                    {
                        drawLevel(object->getChildren());
                    }
                }
            };

            drawLevel(objects);

            ImGui::Separator();
            if (ImGui::Selectable("Clear Selection", currentUUID == 0))
            {
                currentUUID = 0;
                onSelectedCallback(currentUUID);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }
};
} // namespace Core::UI