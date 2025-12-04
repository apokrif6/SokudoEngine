#include "SceneUIWindow.h"

#include "core/engine/Engine.h"
#include "core/scene/SceneEditor.h"
#include "core/scene/Serialization.h"
#include "AnimationUIWindow.h"
#include "imgui_stdlib.h"
#include "glm/gtx/string_cast.hpp"

#include <filesystem>

bool Core::UI::SceneUIWindow::getBody()
{
    if (!ImGui::Begin("Scene"))
    {
        return false;
    }

    if (ImGui::Button("Save Scene"))
    {
        refreshSceneFiles();
        showSaveDialog = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Load Scene"))
    {
        refreshSceneFiles();
        showLoadDialog = true;
        selectedSceneFile.clear();
    }

    if (showSaveDialog)
    {
        ImGui::OpenPopup("Save Scene File");
        if (ImGui::BeginPopupModal("Save Scene File", &showSaveDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::InputText("Filename", &saveFilename);

            if (ImGui::BeginListBox("Existing scenes"))
            {
                for (const auto& file : sceneFileList)
                {
                    if (ImGui::Selectable(file.c_str()))
                    {
                        saveFilename = file;
                    }
                }
                ImGui::EndListBox();
            }

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                if (!saveFilename.empty())
                {
                    if (saveFilename.find(".yaml") == std::string::npos)
                    {
                        saveFilename += ".yaml";
                    }
                    Core::Scene::Serialization::saveSceneToFile(
                            *Core::Engine::getInstance().getSystem<Scene::Scene>(),
                            "assets/scenes/" + saveFilename);
                    showSaveDialog = false;
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showSaveDialog = false;
            }

            ImGui::EndPopup();
        }
    }

    if (showLoadDialog)
    {
        ImGui::OpenPopup("Load Scene File");
        if (ImGui::BeginPopupModal("Load Scene File", &showLoadDialog, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::BeginListBox("Available scenes"))
            {
                for (const auto& file : sceneFileList)
                {
                    if (ImGui::Selectable(file.c_str(), selectedSceneFile == file))
                    {
                        selectedSceneFile = file;
                    }
                }
                ImGui::EndListBox();
            }

            if (ImGui::Button("Load", ImVec2(120, 0)) && !selectedSceneFile.empty())
            {
                Scene::Scene loadedScene = Core::Scene::Serialization::loadSceneFromFile(
                        "assets/scenes/" + selectedSceneFile);
                *Core::Engine::getInstance().getSystem<Scene::Scene>() = loadedScene;
                showLoadDialog = false;
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                showLoadDialog = false;
            }

            ImGui::EndPopup();
        }
    }

    auto loadedObjects = Core::Engine::getInstance().getSystem<Scene::Scene>()->getObjects();
    std::vector<std::string> loadedSceneObjectsNames;
    for (const auto& object : loadedObjects)
    {
        loadedSceneObjectsNames.push_back(object->getName());
    }

    if (loadedSceneObjectsNames.empty())
    {
        ImGui::Text("No objects loaded");
        ImGui::End();
        return true;
    }

    Core::Scene::SceneObjectSelection& sceneObjectSelection =
            Core::Engine::getInstance().getSystem<Scene::Scene>()->getSceneObjectSelection();

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
                sceneObjectSelection.selectedObject = loadedObjects[selectedSceneObjectIndex];
            }

            if (isSceneObjectSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }
    Scene::Transform& sceneObjectTransform = sceneObjectSelection.selectedObject->getTransform();

    ImGui::Text("Position: %s", glm::to_string(sceneObjectTransform.position).c_str());
    ImGui::Text("Rotation: %s", glm::to_string(sceneObjectTransform.rotation).c_str());
    ImGui::Text("Scale: %s", glm::to_string(sceneObjectTransform.scale).c_str());

    if (auto meshObject = std::static_pointer_cast<Core::Renderer::Mesh>(sceneObjectSelection.selectedObject))
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

    ImGui::End();

    return true;
}

void Core::UI::SceneUIWindow::refreshSceneFiles()
{
    sceneFileList.clear();

    const std::filesystem::path scenesDirectory{"assets/scenes"};
    if (!std::filesystem::exists(scenesDirectory))
    {
        std::filesystem::create_directories(scenesDirectory);
    }

    for (const auto& entry : std::filesystem::directory_iterator(scenesDirectory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".yaml")
        {
            sceneFileList.push_back(entry.path().filename().string());
        }
    }
}
