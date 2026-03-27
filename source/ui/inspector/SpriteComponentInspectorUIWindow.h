#pragma once

#include "imgui.h"
#include "ui/UIWindow.h"
#include "engine/Engine.h"
#include "components/SpriteComponent.h"
#include "nfd.hpp"

namespace Core::UI
{
class SpriteComponentInspectorUIWindow : public UIWindow<SpriteComponentInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        auto& objectSelection = Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();
        auto selectedObject = objectSelection.selectedObject.lock();

        auto* spriteComponent = selectedObject->getComponent<Component::SpriteComponent>();
        if (!spriteComponent)
        {
            return true;
        }

        if (ImGui::Button("Import Sprite"))
        {
            NFD::Init();

            nfdfilteritem_t filterItem[1] = {{"Sprite File", "png,jpg"}};
            NFD::UniquePath outPath;

            const nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1, nullptr);

            if (result == NFD_OKAY)
            {
                const std::string path = outPath.get();

                spriteComponent->loadSpriteFromFile(path);
            }
            else if (result == NFD_ERROR)
            {
                Logger::log(1, "NFD Error: %s", NFD::GetError());
            }

            NFD::Quit();
        }

        return true;
    }
};
} // namespace Core::UI
