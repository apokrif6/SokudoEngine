#pragma once

#include <glm/glm.hpp>
#include "VkRenderData.h"

class Camera
{
  public:
    glm::mat4 getViewMatrix(VkRenderData& renderData);

  private:
    glm::vec3 mWorldPosition = glm::vec3(0.5f, 0.25f, 1.0f);

    glm::vec3 mViewDirection = glm::vec3(0.0f, 0.0f, 0.0f);

    glm::vec3 mWorldUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
};
