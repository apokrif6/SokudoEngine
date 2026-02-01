#include "RotatingComponent.h"

#include "core/tools/Logger.h"

void Core::Component::RotatingComponent::onAdded()
{
    if (mOwner)
    {
        if (auto* transformComponent = mOwner->getComponent<TransformComponent>())
        {
            mOwnerTransformComponent = transformComponent;
        }
        else
        {
            Logger::log(1, "%s: RotatingComponent added to SceneObject %s without TransformComponent!", __FUNCTION__,
                        mOwner->getName().c_str());
        }
    }
}

void Core::Component::RotatingComponent::update(Renderer::VkRenderData& renderData)
{
    const float deltaTime = renderData.rdTickDiff;

    if (mOwnerTransformComponent)
    {
        auto& transform = mOwnerTransformComponent->transform;

        const glm::vec3 newRotation = transform.getRotation() + mRotationSpeed * deltaTime;

        transform.setRotation(newRotation);
    }
}

YAML::Node Core::Component::RotatingComponent::serialize() const
{
    YAML::Node node;

    node["rotationSpeed"].push_back(mRotationSpeed.x);
    node["rotationSpeed"].push_back(mRotationSpeed.y);
    node["rotationSpeed"].push_back(mRotationSpeed.z);

    return node;
}

void Core::Component::RotatingComponent::deserialize(const YAML::Node& node)
{
    auto rotationSpeedNode = node["rotationSpeed"];

    mRotationSpeed =
        glm::vec3(rotationSpeedNode[0].as<float>(), rotationSpeedNode[1].as<float>(), rotationSpeedNode[2].as<float>());
}