#pragma once

#include "Component.h"
#include <glm/glm.hpp>

namespace Core::Component
{
class PointLightComponent : public Component
{
public:
    PointLightComponent() = default;
    ~PointLightComponent() override = default;

    [[nodiscard]] std::string_view getTypeName() const override { return "PointLightComponent"; }

    void update(Renderer::VkRenderData& renderData) override;

    YAML::Node serialize() const override;
    void deserialize(const YAML::Node& node) override;

    void setColor(const glm::vec3& color) { mColor = color; }
    void setIntensity(const float intensity) { mIntensity = intensity; }
    void setRadius(const float radius) { mRadius = radius; }

    [[nodiscard]] glm::vec3 getColor() const { return mColor; }
    [[nodiscard]] float getIntensity() const { return mIntensity; }
    [[nodiscard]] float getRadius() const { return mRadius; }

private:
    glm::vec3 mColor{1.0f, 1.0f, 1.0f};
    float mIntensity{10.0f};
    float mRadius{15.0f};
};
} // namespace Core::Component