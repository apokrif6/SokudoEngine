#pragma once

#include "core/components/Component.h"
#include "core/components/TransformComponent.h"
#include "core/scene/objects/SceneObject.h"

namespace Core::Component
{
class RotatingComponent : public Component
{
public:
    [[nodiscard]] std::string_view getTypeName() const override { return "RotatingComponent"; }

    void setRotationSpeed(const glm::vec3& speed) { mRotationSpeed = speed; }
    [[nodiscard]] const glm::vec3& getRotationSpeed() const { return mRotationSpeed; }

    void onAdded() override;

    void update(Renderer::VkRenderData& renderData) override;

    YAML::Node serialize() const override;
    void deserialize(const YAML::Node& node) override;

private:
    glm::vec3 mRotationSpeed{0.f};

    TransformComponent* mOwnerTransformComponent = nullptr;
};
} // namespace Core::Component