#pragma once

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Core::Scene
{
struct Transform
{
    void setPosition(const glm::vec3& inPosition)
    {
        position = inPosition;
        bIsDirty = true;
    }

    void setRotation(const glm::vec3& inRotation)
    {
        rotation = inRotation;
        bIsDirty = true;
    }

    void setRotation(const glm::quat& inRotation)
    {
        rotation = glm::degrees(glm::eulerAngles(inRotation));
        bIsDirty = true;
    }

    void setScale(const glm::vec3& inScale)
    {
        scale = inScale;
        bIsDirty = true;
    }

    [[nodiscard]] const glm::vec3& getPosition() const
    {
        return position;
    }

    [[nodiscard]] const glm::vec3& getRotation() const
    {
        return rotation;
    }

    [[nodiscard]] const glm::vec3& getScale() const
    {
        return scale;
    }

    [[nodiscard]] const glm::mat4& getMatrix() const
    {
        if (bIsDirty)
        {
            const glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
            const glm::mat4 rotate = glm::toMat4(glm::quat(glm::radians(rotation)));
            const glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

            cachedMatrix = translate * rotate * scaling;
            bIsDirty = false;
        }

        return cachedMatrix;
    }

private:
    glm::vec3 position{0.f};
    glm::vec3 rotation{0.f};
    glm::vec3 scale{1.f};

    mutable glm::mat4 cachedMatrix{1.f};
    mutable bool bIsDirty{true};
};
} // namespace Core::Scene