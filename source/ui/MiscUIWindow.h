#pragma once

#include "UIWindow.h"
#include "imgui.h"
#include "glm/gtx/string_cast.hpp"
#include "nfd.hpp"
#include "scene/SceneImporter.h"

namespace Core::UI
{
class MiscUIWindow : public UIWindow<MiscUIWindow>
{
    friend class UIWindow;

    static bool getBody()
    {
        if (!ImGui::Begin("Misc"))
        {
            return false;
        }

        Renderer::VkRenderData& renderData = Engine::getInstance().getRenderData();

        if (ImGui::Button("Import Mesh Scene"))
        {
            NFD::Init();

            nfdfilteritem_t filterItem[1] = {{"3D Models", "gltf,glb,obj,fbx"}};

            NFD::UniquePath outPath;

            const nfdresult_t result = NFD::OpenDialog(outPath, filterItem, 1, nullptr);

            if (result == NFD_OKAY)
            {
                const std::string path = outPath.get();

                const Utils::MeshData meshData = Utils::loadMeshFromFile(path, renderData);

                const auto rootModelObject =
                    Scene::SceneImporter::createObjectFromNode(meshData.rootNode, meshData.skeleton, path);

                auto* currentScene = Engine::getInstance().getSystem<Scene::Scene>();
                currentScene->addObject(rootModelObject);

                Logger::log(1, "Imported scene from: %s", path.c_str());
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

        ImGui::Separator();

        ImGui::Checkbox("Should draw skybox", &renderData.shouldDrawSkybox);
        ImGui::Checkbox("Should draw grid", &renderData.shouldDrawGrid);

        ImGui::Separator();

        ImGui::Text("Field Of View");
        ImGui::SameLine();
        ImGui::SliderInt("FOV", &renderData.rdFieldOfView, 40, 150);

        ImGui::Text("Camera Position:");
        ImGui::SameLine();
        ImGui::Text("%s", glm::to_string(renderData.rdCameraWorldPosition).c_str());

        ImGui::Text("View Yaw:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdViewYaw).c_str());

        ImGui::Text("View Pitch:");
        ImGui::SameLine();
        ImGui::Text("%s", std::to_string(renderData.rdViewPitch).c_str());

        std::string windowDims = std::to_string(renderData.rdWidth) + "x" + std::to_string(renderData.rdHeight);
        ImGui::Text("Window Dimensions:");
        ImGui::SameLine();
        ImGui::Text("%s", windowDims.c_str());

        ImGui::End();

        return true;
    }
};
} // namespace Core::UI