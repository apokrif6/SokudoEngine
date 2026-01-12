#include "TransformComponent.h"

#include <yaml-cpp/node/node.h>

YAML::Node Core::Component::TransformComponent::serialize() const
{
    YAML::Node node;

    const glm::vec3& position = transform.getPosition();
    YAML::Node positionNode;
    positionNode.push_back(position.x);
    positionNode.push_back(position.y);
    positionNode.push_back(position.z);
    node["transform"]["position"] = positionNode;

    const glm::vec3& rotation = transform.getRotation();
    YAML::Node rotationNode;
    rotationNode.push_back(rotation.x);
    rotationNode.push_back(rotation.y);
    rotationNode.push_back(rotation.z);
    node["transform"]["rotation"] = rotationNode;

    const glm::vec3& scale = transform.getRotation();
    YAML::Node scaleNode;
    scaleNode.push_back(scale.x);
    scaleNode.push_back(scale.y);
    scaleNode.push_back(scale.z);
    node["transform"]["scale"] = scaleNode;

    return node;
}

void Core::Component::TransformComponent::deserialize(const YAML::Node& node)
{
    auto transformNode = node["transform"];
    transform.setPosition(glm::vec3(transformNode["position"][0].as<float>(), transformNode["position"][1].as<float>(),
                                    transformNode["position"][2].as<float>()));

    transform.setRotation(glm::vec3(transformNode["rotation"][0].as<float>(), transformNode["rotation"][1].as<float>(),
                                    transformNode["rotation"][2].as<float>()));

    transform.setScale(glm::vec3(transformNode["scale"][0].as<float>(), transformNode["scale"][1].as<float>(),
                                 transformNode["scale"][2].as<float>()));
}