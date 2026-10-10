#pragma once

#include "imgui.h"
#include "string"
#include "nfd.hpp"
#include "animation/AnimationInspectorInverseKinematicsUIWindow.h"
#include "editor/animations/anim-graph/AnimGraphEditorWindow.h"
#include "tools/Logger.h"

namespace Core::UI
{
class AnimationInspectorUIWindow : public UIPermanentWindow<AnimationInspectorUIWindow>
{
    friend class UIWindow;
    friend class UIPermanentWindow;

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
