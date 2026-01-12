#pragma once

#include "core/ui/UIWindow.h"
#include "imgui.h"
#include "MeshComponentInspectorUIWindow.h"
#include "RotatingComponentInspectorUIWIndow.h"
#include "TransformInspectorUIWindow.h"
#include "core/engine/Engine.h"

namespace Core::UI
{
class InspectorUIWindow : public UIWindow<InspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        ImGui::Begin("Inspector");

        ImGui::BeginChild("InspectorScroll", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
        auto objects = scene->getObjects();
        auto& selection = scene->getSceneObjectSelection();

        if (auto selectedObject = selection.selectedObject.lock())
        {
            if (auto* transformComponent = selectedObject->getComponent<Component::TransformComponent>())
            {
                if (ImGui::CollapsingHeader("Transform Component", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    TransformInspectorUIWindow::renderBody();
                }
            }

            if (auto* rotatingComponent = selectedObject->getComponent<Component::RotatingComponent>())
            {
                if (ImGui::CollapsingHeader("Rotating Component", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    RotatingComponentInspectorUIWindow::renderBody();
                }
            }

            if (auto* meshComponent = selectedObject->getComponent<Component::MeshComponent>())
            {
                if (ImGui::CollapsingHeader("Mesh Component", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    MeshComponentInspectorUIWindow::renderBody();
                }
            }
        }
        else
        {
            ImGui::TextDisabled("No object selected");
        }

        ImGui::EndChild();

        ImGui::End();

        return true;
    }
};
} // namespace Core::UI
