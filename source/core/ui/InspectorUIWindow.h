#pragma once

#include "AnimationUIWindow.h"
#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"

namespace Core::UI
{
class InspectorUIWindow : public UIWindow<InspectorUIWindow> {
public:
    static bool getBody()
    {
        ImGui::Begin("Inspector");

        auto* scene = Core::Engine::getInstance().getSystem<Scene::Scene>();
        auto objects = scene->getObjects();
        auto& selection = scene->getSceneObjectSelection();

        auto selectedObject = selection.selectedObject.lock();
        if (selectedObject)
        {
            auto& transform = selectedObject->getTransform();

            ImGui::Text("Selected: %s", selectedObject->getName().c_str());
            ImGui::Separator();

            ImGui::DragFloat3("Position", &transform.position.x, 0.1f);
            ImGui::DragFloat4("Rotation", &transform.rotation.x, 0.1f);
            ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f);

            if (auto meshObject = static_pointer_cast<Renderer::Mesh>(selectedObject))
            {
                if (ImGui::BeginTabBar("SceneSubTabs"))
                {
                    if (meshObject->hasAnimations())
                    {
                        AnimationUIWindow::getBody();
                    }

                    ImGui::EndTabBar();
                }
            }
        }

        ImGui::End();

        return true;
    }
};
}
