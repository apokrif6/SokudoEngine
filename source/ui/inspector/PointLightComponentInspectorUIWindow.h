#pragma once

#include "components/PointLightComponent.h"
#include "ui/UIWindow.h"
#include "engine/Engine.h"
#include "scene/Scene.h"
#include "tools/ColorConverter.h"

namespace Core::UI
{
class PointLightComponentInspectorUIWindow : public UIWindow<PointLightComponentInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        auto selectedObject =
            Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection().selectedObject.lock();
        if (!selectedObject)
        {
            return true;
        }

        auto* lightComponent = selectedObject->getComponent<Component::PointLightComponent>();
        if (!lightComponent)
        {
            return true;
        }

        glm::vec3 color = lightComponent->getColor();
        if (ImGui::ColorEdit3("Light Color", &color.x))
        {
            lightComponent->setColor(color);
        }

        float intensity = lightComponent->getIntensity();
        if (ImGui::DragFloat("Intensity", &intensity, 0.5f, 0.0f, 1000.0f, "%.1f"))
        {
            lightComponent->setIntensity(intensity);
        }

        float radius = lightComponent->getRadius();
        if (ImGui::DragFloat("Radius", &radius, 0.1f, 0.0f, 100.0f, "%.1f"))
        {
            lightComponent->setRadius(radius);
        }

        return true;
    }
};
} // namespace Core::UI