#pragma once

#include "AnimationInspectorUIWindow.h"
#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "TransformInspectorUIWindow.h"
#include "core/engine/Engine.h"

namespace Core::UI
{
class InspectorUIWindow : public UIWindow<InspectorUIWindow> {
public:
    static bool getBody()
    {
        ImGui::Begin("Inspector");

        ImGui::BeginChild(
            "InspectorScroll",
            ImVec2(0, 0),
            false,
            ImGuiWindowFlags_AlwaysVerticalScrollbar
        );

        auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
        auto objects = scene->getObjects();
        auto& selection = scene->getSceneObjectSelection();

        auto selectedObject = selection.selectedObject.lock();
        if (selectedObject)
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                TransformInspectorUIWindow::getBody();
            }

            if (auto meshObject = static_pointer_cast<Renderer::Mesh>(selectedObject))
            {
                if (ImGui::BeginTabBar("SceneSubTabs"))
                {
                    if (meshObject->hasAnimations())
                    {
                        if (ImGui::CollapsingHeader("Animation", ImGuiTreeNodeFlags_DefaultOpen))
                        {
                            AnimationInspectorUIWindow::getBody();
                        }
                    }

                    ImGui::EndTabBar();
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
}
