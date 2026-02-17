#pragma once

#include "core/components/TransformComponent.h"
#include "ui/UIWindow.h"
#include "core/engine/Engine.h"

namespace Core::UI
{
class TransformInspectorUIWindow : public UIWindow<TransformInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* transformComponent = selectedObject->getComponent<Component::TransformComponent>();
        if (!transformComponent)
        {
            return true;
        }

        ImGui::Text("Selected: %s", selectedObject->getName().c_str());
        ImGui::Separator();

        glm::vec3 position = transformComponent->getPosition();
        if (ImGui::DragFloat3("Position", &position.x, 0.1f))
        {
            transformComponent->setPosition(position);
        }
        glm::vec3 rotation = transformComponent->getRotation();
        if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f))
        {
            transformComponent->setRotation(rotation);
        }
        glm::vec3 scale = transformComponent->getScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
        {
            transformComponent->setScale(scale);
        }

        return true;
    }
};
} // namespace Core::UI
