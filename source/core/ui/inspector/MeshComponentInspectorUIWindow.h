#pragma once

#include "imgui.h"
#include "core/components/MeshComponent.h"
#include "core/ui/UIWindow.h"
#include "core/engine/Engine.h"

namespace Core::UI
{
class MeshComponentInspectorUIWindow : public UIWindow<MeshComponentInspectorUIWindow>
{
public:
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

        if (meshComponent->hasAnimations())
        {
            AnimationInspectorUIWindow::getBody(meshComponent);
        }

        return true;
    }
};
} // namespace Core::UI
