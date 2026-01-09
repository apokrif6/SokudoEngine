#include "Serialization.h"
#include "../components/MeshComponent.h"
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
    for (const auto& objectNode : node["objects"])
    {
        std::shared_ptr<SceneObject> object;
        object->deserialize(objectNode);
        scene.addObject(object);
    }
    return scene;
}

void Core::Scene::Serialization::saveSceneToFile(const Scene& scene, const std::string& filename)
{
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
    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        return Scene{};
    }

    const YAML::Node sceneNode = YAML::Load(fin);
    return deserializeScene(sceneNode);
}