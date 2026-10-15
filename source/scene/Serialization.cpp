#include "Serialization.h"
#include "components/MeshComponent.h"
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

void Core::Scene::Serialization::deserializeScene(Scene& scene, const YAML::Node& node)
{
    if (const auto objectsNode = node["objects"])
    {
        for (const auto& objectNode : objectsNode)
        {
            const auto sceneObject = std::make_shared<SceneObject>("", &scene);
            sceneObject->deserialize(objectNode);
            scene.addObject(sceneObject);
        }
    }
}

void Core::Scene::Serialization::saveSceneToFile(const Scene& scene, const std::string& filename)
{
    const YAML::Node sceneNode = serialize(scene);
    std::ofstream fout(filename, std::ios::out | std::ios::trunc);
    if (fout.is_open())
    {
        fout << sceneNode;
        fout.close();
    }
}

bool Core::Scene::Serialization::loadSceneFromFile(Scene& scene, const std::string& filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
    {
        return false;
    }

    const YAML::Node sceneNode = YAML::Load(fin);
    deserializeScene(scene, sceneNode);

    return true;
}