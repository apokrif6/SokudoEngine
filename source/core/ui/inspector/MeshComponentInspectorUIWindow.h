#pragma once

#include "AnimationInspectorUIWindow.h"
#include "imgui.h"
#include "core/components/MeshComponent.h"
#include "core/ui/UIWindow.h"
#include "core/engine/Engine.h"
#include "nfd.hpp"

namespace Core::UI
{
class MeshComponentInspectorUIWindow : public UIWindow<MeshComponentInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* meshComponent = selectedObject->getComponent<Component::MeshComponent>();
        if (!meshComponent)
        {
            return true;
        }

        ImGui::Text("Mesh file path %s", meshComponent->getMeshFilePath().data());

        if (ImGui::Button("Load New Mesh"))
        {
            NFD::Init();

            nfdfilteritem_t filterItem[1] = {{"3D Models", "gltf,glb,obj,fbx"}};

            NFD::UniquePath outPath;

            nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1, nullptr);

            if (result == NFD_OKAY)
            {
                std::string path = outPath.get();

                meshComponent->loadMesh(path);
            }
            else if (result == NFD_CANCEL)
            {
                Logger::log(1, "File Dialog Cancelled");
            }
            else
            {
                Logger::log(1, "File Dialog Error: %s", NFD::GetError());
            }

            NFD::Quit();
        }

        if (meshComponent->hasAnimations())
        {
            ImGui::Separator();
            AnimationInspectorUIWindow::renderBody(meshComponent);
        }

        return true;
    }
};
} // namespace Core::UI
