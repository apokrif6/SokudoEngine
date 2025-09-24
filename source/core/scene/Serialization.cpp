#include "Serialization.h"
#include <fstream>

YAML::Node Core::Scene::Serialization::serialize(const Scene& scene)
{
    YAML::Node sceneNode;
    for (const auto& object : scene.getObjects())
    {
        sceneNode["objects"].push_back(serialize(*object));
    }
    return sceneNode;
}

Core::Scene::Scene Core::Scene::Serialization::deserializeScene(const YAML::Node& node)
{
    Scene scene;
    auto objectsNode = node["objects"];
    for (const auto& objectNode : objectsNode)
    {
        auto object = deserializeSceneObject(objectNode);
        scene.addObject(std::make_shared<SceneObject>(object));
    }
    return scene;
}

YAML::Node Core::Scene::Serialization::serialize(const SceneObject& object)
{
    YAML::Node node;
    node["name"] = object.getName();
    const Transform& transform = object.getTransform();

    YAML::Node position;
    position.push_back(transform.position.x);
    position.push_back(transform.position.y);
    position.push_back(transform.position.z);
    node["transform"]["position"] = position;

    YAML::Node rotation;
    rotation.push_back(transform.rotation.x);
    rotation.push_back(transform.rotation.y);
    rotation.push_back(transform.rotation.z);
    node["transform"]["rotation"] = rotation;

    YAML::Node scale;
    scale.push_back(transform.scale.x);
    scale.push_back(transform.scale.y);
    scale.push_back(transform.scale.z);
    node["transform"]["scale"] = scale;

    return node;
}

Core::Scene::SceneObject Core::Scene::Serialization::deserializeSceneObject(const YAML::Node& node)
{
    std::string name = node["name"].as<std::string>();
    auto object = std::make_shared<SceneObject>(name);
    auto transformNode = node["transform"];
    object->getTransform().position = glm::vec3(transformNode["position"][0].as<float>(), transformNode["position"][1].as<float>(), transformNode["position"][2].as<float>());
    object->getTransform().rotation = glm::quat(transformNode["rotation"][0].as<float>(), transformNode["rotation"][1].as<float>(), transformNode["rotation"][2].as<float>(), transformNode["rotation"][3].as<float>());
    object->getTransform().scale = glm::vec3(transformNode["scale"][0].as<float>(), transformNode["scale"][1].as<float>(), transformNode["scale"][2].as<float>());
    return *object;
}

void Core::Scene::Serialization::saveSceneToFile(const Scene& scene, const std::string& filename)
{
    YAML::Node sceneNode = serialize(scene);
    std::ofstream fout(filename);
    fout << sceneNode;
}

Core::Scene::Scene Core::Scene::Serialization::loadSceneFromFile(const std::string& filename)
{
    std::ifstream fin(filename);
    YAML::Node sceneNode = YAML::Load(fin);
    return deserializeScene(sceneNode);
}