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

    void setRotation(const glm::vec3& rotation);

    void setScale(const glm::vec3& scale);

    void setWorldDirty();

    [[nodiscard]] glm::mat4 getWorldMatrix();

    [[nodiscard]] const glm::vec3& getPosition() const { return transform.getPosition(); }

    [[nodiscard]] const glm::vec3& getRotation() const { return transform.getRotation(); }

    [[nodiscard]] const glm::vec3& getScale() const { return transform.getScale(); }

private:
    void updateWorldMatrix();

    Scene::Transform transform;
    glm::mat4 mCachedWorldMatrix{1.0f};
    bool bIsWorldDirty{true};
};
} // namespace Core::Component