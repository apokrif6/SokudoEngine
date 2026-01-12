#pragma once

#include "core/components/RotatingComponent.h"
#include"core/ui/UIWindow.h"

namespace Core::UI
{
class RotatingComponentInspectorUIWindow : public UIWindow<RotatingComponentInspectorUIWindow>
{
public:
    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* rotatingComponent = selectedObject->getComponent<Component::RotatingComponent>();
        if (!rotatingComponent)
        {
            ImGui::End();
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
