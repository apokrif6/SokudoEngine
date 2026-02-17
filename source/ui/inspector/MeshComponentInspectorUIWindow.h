#pragma once

#include "AnimationInspectorUIWindow.h"
#include "imgui.h"
#include "core/components/MeshComponent.h"
#include "ui/UIWindow.h"
#include "core/engine/Engine.h"

namespace Core::UI
{
class MeshComponentInspectorUIWindow : public UIWindow<MeshComponentInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* meshComponent = selectedObject->getComponent<Component::MeshComponent>();
        if (!meshComponent)
        {
            return true;
        }

        ImGui::Text("Mesh file path %s", meshComponent->getMeshFilePath().data());

        ImGui::Text("Primitive index %d", meshComponent->getPrimitiveIndex());

        if (meshComponent->hasAnimations())
        {
            ImGui::Separator();
            AnimationInspectorUIWindow::renderBody(meshComponent);
        }

        return true;
    }
};
} // namespace Core::UI
