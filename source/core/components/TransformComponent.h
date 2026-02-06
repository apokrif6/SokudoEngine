#pragma once

#include <yaml-cpp/node/node.h>

#include "Component.h"
#include "core/scene/Transform.h"
#include "core/scene/objects/SceneObject.h"

namespace Core::Component
{
class TransformComponent : public Component
{
public:
    [[nodiscard]] std::string_view getTypeName() const override { return "TransformComponent"; }

    YAML::Node serialize() const override;
    void deserialize(const YAML::Node& node) override;

    void setPosition(const glm::vec3& position);

    void setRotation(const glm::quat& rotation);

    void setScale(const glm::vec3& scale);

    void setWorldDirty();

    [[nodiscard]] glm::mat4 getWorldMatrix();

    [[nodiscard]] Scene::Transform getTransform() const { return transform; }

private:
    void updateWorldMatrix()
    {
        const glm::mat4 local = transform.getMatrix();

        if (auto* parent = getOwner()->getParent())
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

    Scene::Transform transform;
    glm::mat4 mCachedWorldMatrix{1.0f};
    bool bIsWorldDirty{true};
};
} // namespace Core::Component