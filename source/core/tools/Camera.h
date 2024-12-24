#pragma once

#include <glm/glm.hpp>
#include "core/vk-renderer/VkRenderData.h"

class Camera
{
  public:
    glm::mat4 getViewMatrix(Core::Renderer::VkRenderData& renderData);

  private:
    glm::vec3 mViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 mRightDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 mUpDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 mWorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
};
