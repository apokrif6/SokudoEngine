#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"
#include "core/scene/SceneEditor.h"
#include "AnimationUIWindow.h"
#include "ImGuizmo.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

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

        Core::Scene::SceneObjectSelection& sceneObjectSelection =
            Core::Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();

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
        Scene::Transform& sceneObjectTransform = sceneObjectSelection.selectedObject->getTransform();

        ImGui::Text("Position: %s", glm::to_string(sceneObjectTransform.position).c_str());
        ImGui::Text("Rotation: %s", glm::to_string(sceneObjectTransform.rotation).c_str());
        ImGui::Text("Scale: %s", glm::to_string(sceneObjectTransform.scale).c_str());

        if (auto meshObject = std::static_pointer_cast<Core::Renderer::Mesh>(sceneObjectSelection.selectedObject))
        {
            if (ImGui::BeginTabBar("SceneSubTabs"))
            {
                AnimationUIWindow::getBody();

                ImGui::EndTabBar();
            }
        }

        ImGui::EndTabItem();

        if (sceneObjectSelection.selectedObject)
        {
            auto perspectiveViewMatrices =
                Core::Engine::getInstance().getSystem<Renderer::VkRenderer>()->getPerspectiveViewMatrices();
            glm::mat4 view = perspectiveViewMatrices[0];
            glm::mat4 projection = perspectiveViewMatrices[1];

            ImGuizmo::BeginFrame();
            ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());

            ImGuiIO& io = ImGui::GetIO();
            ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

            glm::mat4 objectMatrix = sceneObjectTransform.getMatrix();

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), currentManipulateOperation, ImGuizmo::LOCAL,
                                 glm::value_ptr(objectMatrix));

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, scale, skew;
                glm::vec4 perspective;
                glm::quat rotation;

                glm::decompose(objectMatrix, scale, rotation, translation, skew, perspective);

                sceneObjectTransform.position = translation;
                sceneObjectTransform.rotation = rotation;
                sceneObjectTransform.scale = scale;
            }
        }

        return true;
    }

    static void setOperation(ImGuizmo::OPERATION manipulateOperation) { currentManipulateOperation = manipulateOperation; }

  private:
    static inline int selectedSceneObjectIndex = 0;
    static inline ImGuizmo::OPERATION currentManipulateOperation = ImGuizmo::TRANSLATE;
};
} // namespace Core::UI
