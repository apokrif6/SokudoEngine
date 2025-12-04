#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"
#include "core/profiling/PlotBuffer.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace Core::UI
{
class ViewportUIWindow : public UIWindow<ViewportUIWindow>
{
public:
    static bool getBody()
    {
        if (!ImGui::Begin("Viewport"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

        auto viewportSize = ImGui::GetContentRegionAvail();
        if (renderData.rdViewportTarget.size.x != static_cast<int>(viewportSize.x) ||
            renderData.rdViewportTarget.size.y != static_cast<int>(viewportSize.y))
        {
            Core::Engine::getInstance().getSystem<Renderer::VkRenderer>()->resizeViewportTarget({viewportSize.x, viewportSize.y});
        }

        ImGui::Image(reinterpret_cast<ImTextureID>(renderData.rdViewportTarget.descriptorSet),
                     viewportSize);

        Scene::SceneObjectSelection sceneObjectSelection = Core::Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();

        if (sceneObjectSelection.selectedObject)
        {
            auto perspectiveViewMatrices =
                    Core::Engine::getInstance().getSystem<Renderer::VkRenderer>()->getPerspectiveViewMatrices();
            glm::mat4 view = perspectiveViewMatrices[0];
            glm::mat4 projection = perspectiveViewMatrices[1];

            ImGuizmo::BeginFrame();

            ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

            ImVec2 windowPos  = ImGui::GetWindowPos();
            ImGuizmo::SetRect(windowPos.x, windowPos.y, renderData.rdViewportTarget.size.x, renderData.rdViewportTarget.size.y);

            Scene::Transform& sceneObjectTransform = sceneObjectSelection.selectedObject->getTransform();

            glm::mat4 objectMatrix = sceneObjectTransform.getMatrix();

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                                 currentManipulateOperation, ImGuizmo::LOCAL,
                                 glm::value_ptr(objectMatrix));

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 translation, scale, skew;
                glm::vec4 perspective;
                glm::quat rotation;

                glm::decompose(objectMatrix, scale, rotation, translation, skew, perspective);

                sceneObjectTransform.position = translation;
                sceneObjectTransform.rotation = rotation;
                sceneObjectTransform.scale = scale;
            }
        }

        ImGui::End();

        return true;
    }

    static void setSceneObjectManipulationOperation(ImGuizmo::OPERATION manipulateOperation)
    {
        currentManipulateOperation = manipulateOperation;
    }

private:
    static inline ImGuizmo::OPERATION currentManipulateOperation = ImGuizmo::TRANSLATE;
};
}  // namespace Core::UI