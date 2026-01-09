#pragma once

#include "core/components/TransformComponent.h"
#include"core/ui/UIWindow.h"

namespace Core::UI
{
class TransformInspectorUIWindow : public UIWindow<TransformInspectorUIWindow>
{
public:
    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* transformComponent = selectedObject->getComponent<Component::TransformComponent>();
        if (!transformComponent)
        {
            ImGui::End();
            return true;
        }

        auto& transform = transformComponent->transform;

        ImGui::Text("Selected: %s", selectedObject->getName().c_str());
        ImGui::Separator();

        ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
        ImGui::DragFloat4("Rotation", &transform.rotation.x, 0.1f);
        ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);

        return true;
    }
};
} // namespace Core::UI
