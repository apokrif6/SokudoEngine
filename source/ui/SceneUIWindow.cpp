#include "SceneUIWindow.h"
#include "engine/Engine.h"
#include "engine/ScopedEngineState.h"
#include "scene/SceneEditor.h"
#include "scene/Serialization.h"
#include "imgui.h"
#include "imgui_stdlib.h"
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
                    ScopedEngineState pause{EngineState::Loading};

                    if (saveFilename.find(".yaml") == std::string::npos)
                    {
                        saveFilename += ".yaml";
                    }
                    Scene::Serialization::saveSceneToFile(*Engine::getInstance().getSystem<Scene::Scene>(),
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
                ScopedEngineState pause{EngineState::Loading};

                const Scene::Scene loadedScene =
                    Scene::Serialization::loadSceneFromFile("assets/scenes/" + selectedSceneFile);

                Engine::getInstance().getSystem<Scene::Scene>()->cleanup(Engine::getInstance().getRenderData());
                *Engine::getInstance().getSystem<Scene::Scene>() = loadedScene;
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

    ImGui::Separator();

    auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
    auto objects = scene->getObjects();
    auto& selection = scene->getSceneObjectSelection();

    ImGui::Text("Scene Hierarchy:");

    if (objects.empty())
    {
        ImGui::Text("No objects in scene");
    }

    for (auto& obj : objects)
    {
        if (obj->getParent() == nullptr)
        {
            drawSceneObjectNode(obj, selection);
        }
    }

    if (ImGui::BeginPopupContextWindow("InspectorContextMenu", ImGuiPopupFlags_MouseButtonRight))
    {
        if (ImGui::MenuItem("Create Empty Object"))
        {
            auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
            scene->createObject("New Scene Object");
        }

        ImGui::EndPopup();
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

void Core::UI::SceneUIWindow::drawSceneObjectNode(std::shared_ptr<Scene::SceneObject> object,
                                                  Scene::SceneObjectSelection& selection)
{
    // TODO
    // add ensure
    if (!object)
    {
        return;
    }

    auto selectedObject = selection.selectedObject.lock();
    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow | (selectedObject == object ? ImGuiTreeNodeFlags_Selected : 0);

    bool isLeaf = object->getChildren().empty();
    if (isLeaf)
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    bool opened = ImGui::TreeNodeEx((void*)object.get(), flags, "%s", object->getName().c_str());

    if (ImGui::IsItemClicked())
    {
        selection.selectedObject = object;
    }

    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("SCENE_NODE", &object, sizeof(std::shared_ptr<Scene::SceneObject>));
        ImGui::Text("%s", object->getName().c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_NODE"))
        {
            auto dropped = *(std::shared_ptr<Scene::SceneObject>*)payload->Data;

            if (dropped != object)
            {
                if (dropped->getParent())
                {
                    dropped->getParent()->removeChild(dropped.get());
                }

                object->addChild(dropped);
            }
        }
        ImGui::EndDragDropTarget();
    }

    if (!isLeaf && opened)
    {
        for (auto& child : object->getChildren())
        {
            drawSceneObjectNode(child, selection);
        }

        ImGui::TreePop();
    }
}