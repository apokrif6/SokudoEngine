#pragma once

#include "core/components/TransformComponent.h"
#include "core/ui/UIWindow.h"
#include "core/engine/Engine.h"

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
            return true;
        }

        auto& transform = transformComponent->transform;

        ImGui::Text("Selected: %s", selectedObject->getName().c_str());
        ImGui::Separator();

        glm::vec3 position = transform.getPosition();
        if (ImGui::DragFloat3("Position", &position.x, 0.1f))
        {
            transform.setPosition(position);
        }
        glm::vec3 rotation = transform.getRotation();
        if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f))
        {
            transform.setRotation(rotation);
        }
        glm::vec3 scale = transform.getScale();
        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f))
        {
            transform.setScale(scale);
        }

        return true;
    }
};
} // namespace Core::UI
