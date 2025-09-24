#include "Serialization.h"
#include "core/scene/objects/Mesh.h"
#include "core/engine/ScopedEnginePause.h"
#include <fstream>

YAML::Node Core::Scene::Serialization::serialize(const Scene& scene)
{
    YAML::Node sceneNode;
    for (const auto& object : scene.getObjects())
    {
        sceneNode["objects"].push_back(object->serialize());
    }
    return sceneNode;
}

Core::Scene::Scene Core::Scene::Serialization::deserializeScene(const YAML::Node& node)
{
    Scene scene;
    auto objectsNode = node["objects"];
    for (const auto& objectNode : objectsNode)
    {
        auto type = static_cast<ObjectType>(objectNode["type"].as<int>());
        std::shared_ptr<SceneObject> object;

        switch (type) {
            case ObjectType::Mesh:
                object = std::make_shared<Core::Renderer::Mesh>("", Animations::Skeleton{});
                break;
            case ObjectType::Empty:
            default:
                object = std::make_shared<SceneObject>("");
                break;
        }

        object->deserialize(objectNode);
        scene.addObject(object);
    }
    return scene;
}

void Core::Scene::Serialization::saveSceneToFile(const Scene& scene, const std::string& filename)
{
    Core::ScopedEnginePause pause;

    YAML::Node sceneNode = serialize(scene);
    std::ofstream fout(filename, std::ios::out | std::ios::trunc);
    if (fout.is_open())
    {
        fout << sceneNode;
        fout.close();
    }
}

Core::Scene::Scene Core::Scene::Serialization::loadSceneFromFile(const std::string& filename)
{
    Core::ScopedEnginePause pause;

    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        return Scene{};
    }
    YAML::Node sceneNode = YAML::Load(fin);
    return deserializeScene(sceneNode);
}