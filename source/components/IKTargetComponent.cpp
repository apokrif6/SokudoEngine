#include "IKTargetComponent.h"
#include "TransformComponent.h"

glm::vec3 Core::Component::IKTargetComponent::getTargetWorldPosition() const
{
    if (!mOwner)
    {
        return glm::vec3(0.0f);
    }

    if (const auto* transform = getOwner()->getComponent<TransformComponent>())
    {
        return transform->getPosition();
    }

    return glm::vec3(0.0f);
}