#pragma once

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Core::Scene
{
struct Transform
{
    glm::vec3 position {0.0f};
    glm::quat rotation {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale {1.0f};

    glm::mat4 getMatrix() const
    {
        glm::mat4 translate = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rotate = glm::toMat4(rotation);
        glm::mat4 scaling = glm::scale(glm::mat4(1.0f), scale);

        return translate * rotate * scaling;
    }

    void setEulerRotation(const glm::vec3& eulerAngles)
    {
        rotation = glm::quat(glm::radians(eulerAngles));
    }

    glm::vec3 getEulerRotation() const
    {
        return glm::degrees(glm::eulerAngles(rotation));
    }
};
}