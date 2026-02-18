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

    const glm::vec3& scale = transform.getScale();
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

void Core::Component::TransformComponent::setPosition(const glm::vec3& position)
{
    transform.setPosition(position);
    setWorldDirty();
}

void Core::Component::TransformComponent::setRotation(const glm::quat& rotation)
{
    transform.setRotation(rotation);
    setWorldDirty();
}

void Core::Component::TransformComponent::setRotation(const glm::vec3& rotation)
{
    transform.setRotation(rotation);
    setWorldDirty();
}

void Core::Component::TransformComponent::setScale(const glm::vec3& scale)
{
    transform.setScale(scale);
    setWorldDirty();
}

void Core::Component::TransformComponent::setWorldDirty()
{
    if (bIsWorldDirty)
    {
        return;
    }

    bIsWorldDirty = true;

    for (const auto& child : getOwner()->getChildren())
    {
        if (auto* childTransformComponent = child->getComponent<TransformComponent>())
        {
            childTransformComponent->setWorldDirty();
        }
    }
}

glm::mat4 Core::Component::TransformComponent::getWorldMatrix()
{
    if (bIsWorldDirty)
    {
        updateWorldMatrix();
    }
    return mCachedWorldMatrix;
}

void Core::Component::TransformComponent::updateWorldMatrix()
{
    const glm::mat4 local = transform.getMatrix();

    if (const auto* parent = getOwner()->getParent())
    {
        if (auto* parentTransformComponent = parent->getComponent<TransformComponent>())
        {
            mCachedWorldMatrix = parentTransformComponent->getWorldMatrix() * local;
        }
    }
    else
    {
        mCachedWorldMatrix = local;
    }

    bIsWorldDirty = false;
}
