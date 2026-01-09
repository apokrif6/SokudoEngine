#include "TransformComponent.h"

#include <yaml-cpp/node/node.h>

YAML::Node Core::Component::TransformComponent::serialize() const
{
    YAML::Node node;

    YAML::Node position;
    position.push_back(transform.position.x);
    position.push_back(transform.position.y);
    position.push_back(transform.position.z);
    node["transform"]["position"] = position;

    YAML::Node rotation;
    rotation.push_back(transform.rotation.w);
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

void Core::Component::TransformComponent::deserialize(const YAML::Node& node)
{
    auto transformNode = node["transform"];
    transform.position = glm::vec3(transformNode["position"][0].as<float>(), transformNode["position"][1].as<float>(),
                                    transformNode["position"][2].as<float>());

    transform.rotation = glm::quat(transformNode["rotation"][0].as<float>(), transformNode["rotation"][1].as<float>(),
                                    transformNode["rotation"][2].as<float>(), transformNode["rotation"][3].as<float>());

    transform.scale = glm::vec3(transformNode["scale"][0].as<float>(), transformNode["scale"][1].as<float>(),
                                 transformNode["scale"][2].as<float>());
}