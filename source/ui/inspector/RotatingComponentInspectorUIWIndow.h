#pragma once

#include "components/RotatingComponent.h"
#include "ui/UIWindow.h"
#include "engine/Engine.h"

namespace Core::UI
{
class RotatingComponentInspectorUIWindow : public UIWindow<RotatingComponentInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* rotatingComponent = selectedObject->getComponent<Component::RotatingComponent>();
        if (!rotatingComponent)
        {
            return true;
        }

        glm::vec3 rotationSpeed = rotatingComponent->getRotationSpeed();
        if (ImGui::DragFloat3("Rotation Speed", &rotationSpeed.x, 0.1f))
        {
            rotatingComponent->setRotationSpeed(rotationSpeed);
        }

        return true;
    }
};
} // namespace Core::UI
