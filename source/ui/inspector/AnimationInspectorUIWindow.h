#pragma once

#include "ui/UIWindow.h"
#include "imgui.h"
#include "string"
#include "nfd.hpp"
#include "animation/AnimationInspectorInverseKinematicsUIWindow.h"
#include "editor/animations/anim-graph/AnimGraphEditorWindow.h"

namespace Core::UI
{
class AnimationInspectorUIWindow : public UIWindow<AnimationInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody(Component::MeshComponent* meshComponent)
    {
        if (ImGui::Button("Import Animation"))
        {
            NFD::Init();

            nfdfilteritem_t filterItem[1] = {{"Animation Files", "fbx,gltf,glb,dae"}};
            NFD::UniquePath outPath;

            const nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1, nullptr);

            if (result == NFD_OKAY)
            {
                const std::string path = outPath.get();

                meshComponent->loadAnimationFromFile(path);
            }
            else if (result == NFD_ERROR)
            {
                Logger::log(1, "NFD Error: %s", NFD::GetError());
            }

            NFD::Quit();
        }

        if (!meshComponent->hasAnimations())
        {
            return true;
        }

        AnimationInspectorInverseKinematicsUIWindow::renderBody(meshComponent);

        if (ImGui::Button("Edit Anim Graph"))
        {
            if (meshComponent->getAnimGraph())
            {
                Editor::Animations::AnimGraphEditorWindow::open(meshComponent);
            }
        }

        return true;
    }
};
} // namespace Core::UI
