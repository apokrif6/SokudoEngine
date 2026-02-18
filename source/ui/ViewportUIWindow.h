#pragma once

#include <ImGuizmo.h>
#include "UIWindow.h"
#include "imgui.h"
#include "engine/Engine.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace Core::UI
{
class ViewportUIWindow : public UIWindow<ViewportUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        if (!ImGui::Begin("Viewport"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Engine::getInstance().getRenderData();

        renderData.rdViewportHovered = ImGui::IsWindowHovered();

        auto viewportSize = ImGui::GetContentRegionAvail();
        if (renderData.rdViewportTarget.size.x != static_cast<int>(viewportSize.x) ||
            renderData.rdViewportTarget.size.y != static_cast<int>(viewportSize.y))
        {
            Engine::getInstance().getSystem<Renderer::VkRenderer>()->resizeViewportTarget(
                {viewportSize.x, viewportSize.y});
        }

        ImGui::Image(reinterpret_cast<ImTextureID>(renderData.rdViewportTarget.descriptorSet), viewportSize);

        Scene::SceneObjectSelection sceneObjectSelection =
            Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();

        if (auto selectedObject = sceneObjectSelection.selectedObject.lock())
        {
            if (!renderData.freeCameraMovement)
            {
                auto globalSceneData = Engine::getInstance().getSystem<Renderer::VkRenderer>()->getGlobalSceneData();

                glm::mat4 view = globalSceneData.view;
                glm::mat4 projection = globalSceneData.projection;

                ImGuizmo::BeginFrame();

                ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

                ImVec2 windowPos = ImGui::GetWindowPos();
                ImGuizmo::SetRect(windowPos.x, windowPos.y, static_cast<float>(renderData.rdViewportTarget.size.x),
                                  static_cast<float>(renderData.rdViewportTarget.size.y));

                auto* transformComponent = selectedObject->getComponent<Component::TransformComponent>();
                if (!transformComponent)
                {
                    ImGui::End();
                    return true;
                }
                auto objectMatrix = transformComponent->getWorldMatrix();

                ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection), currentManipulateOperation,
                                     ImGuizmo::LOCAL, glm::value_ptr(objectMatrix));

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 position, scale, skew;
                    glm::vec4 perspective;
                    glm::quat rotation;

                    glm::decompose(objectMatrix, scale, rotation, position, skew, perspective);

                    if (currentManipulateOperation == ImGuizmo::TRANSLATE)
                    {
                        transformComponent->setPosition(position);
                    }
                    else if (currentManipulateOperation == ImGuizmo::ROTATE)
                    {
                        transformComponent->setRotation(rotation);
                    }
                    else if (currentManipulateOperation == ImGuizmo::SCALE)
                    {
                        transformComponent->setScale(scale);
                    }
                }
            }
        }

        ImGui::End();

        return true;
    }

public:
    static void setSceneObjectManipulationOperation(ImGuizmo::OPERATION manipulateOperation)
    {
        currentManipulateOperation = manipulateOperation;
    }

private:
    static inline ImGuizmo::OPERATION currentManipulateOperation = ImGuizmo::TRANSLATE;
};
} // namespace Core::UI