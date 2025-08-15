#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"
#include "core/scene/SceneEditor.h"
#include "AnimationUIWindow.h"

namespace Core::UI
{
class SceneUIWindow : public UIWindow<SceneUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Scene"))
        {
            return false;
        }

        auto loadedObjects = Core::Engine::getInstance().getSystem<Scene::Scene>()->getObjects();
        std::vector<std::string> loadedSceneObjectsNames;
        for (const auto& object : loadedObjects)
        {
            loadedSceneObjectsNames.push_back(object->getName());
        }

        if (loadedSceneObjectsNames.empty())
        {
            ImGui::Text("No objects loaded");
            ImGui::EndTabItem();
            return true;
        }

        Core::Scene::SceneObjectSelection& sceneObjectSelection = Core::Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();

        ImGui::Separator();
        ImGui::Text("Selected Scene Object:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##Loaded objects", loadedSceneObjectsNames[selectedSceneObjectIndex].c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedSceneObjectsNames.size(); ++i)
            {
                const bool isSceneObjectSelected = (selectedSceneObjectIndex == i);
                if (ImGui::Selectable(loadedSceneObjectsNames[i].c_str(), isSceneObjectSelected))
                {
                    selectedSceneObjectIndex = i;
                    sceneObjectSelection.selectedObject = loadedObjects[selectedSceneObjectIndex];
                }

                if (isSceneObjectSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }
        ImGui::Text("Position: %s", glm::to_string(sceneObjectSelection.selectedObject->getTransform().position).c_str());
        ImGui::Text("Rotation: %s", glm::to_string(sceneObjectSelection.selectedObject->getTransform().rotation).c_str());
        ImGui::Text("Scale: %s", glm::to_string(sceneObjectSelection.selectedObject->getTransform().scale).c_str());

        if (auto meshObject = std::static_pointer_cast<Core::Renderer::Mesh>(sceneObjectSelection.selectedObject))
        {
            if (ImGui::BeginTabBar("SceneSubTabs"))
            {
                AnimationUIWindow::getBody();

                ImGui::EndTabBar();
            }
        }

        ImGui::EndTabItem();

        return true;
    }

private:
    static inline int selectedSceneObjectIndex = 0;
};
}

