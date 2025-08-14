#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "string"
#include "core/engine/Engine.h"

namespace Core::UI
{
class SceneUIWindow : public UIWindow<SceneUIWindow>
{
  public:
    static bool getBody()
    {
        if (!ImGui::BeginTabItem("Scene"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Core::Engine::getInstance().getRenderData();

        std::vector<std::string> loadedSceneObjectsNames;
        for (const auto& object : Core::Engine::getInstance().getSystem<Scene::Scene>()->getObjects())
        {
            loadedSceneObjectsNames.push_back(object->getName());
        }

        if (loadedSceneObjectsNames.empty())
        {
            ImGui::Text("No objects loaded");
            ImGui::EndTabItem();
            return true;
        }

        ImGui::Separator();
        ImGui::Text("Selected Scene Object:");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##Loaded objects", loadedSceneObjectsNames[selectedSceneObjectIndex].c_str(),
                              ImGuiComboFlags_WidthFitPreview))
        {
            for (int i = 0; i < loadedSceneObjectsNames.size(); ++i)
            {
                const bool isSceneObjectSelected = (selectedSceneObjectIndex == i);
                if (ImGui::Selectable(loadedSceneObjectsNames[i].c_str(), isSceneObjectSelected))
                {
                    selectedSceneObjectIndex = i;
                }

                if (isSceneObjectSelected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }

            ImGui::EndCombo();
        }

        ImGui::EndTabItem();

        return true;
    }

private:
    static inline int selectedSceneObjectIndex = 0;
};
}

