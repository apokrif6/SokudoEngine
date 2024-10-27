#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

glm::mat4 Camera::getViewMatrix(VkRenderData& renderData)
{
    mViewDirection = glm::normalize(
        glm::vec3(sin(renderData.rdViewYaw / 180.0 * M_PI) * cos(renderData.rdViewPitch / 180.0 * M_PI),
                  -sin(renderData.rdViewPitch / 180.0 * M_PI),
                  -cos(renderData.rdViewYaw / 180.0 * M_PI) * cos(renderData.rdViewPitch / 180.0 * M_PI)));

    return glm::lookAt(mWorldPosition, mWorldPosition + mViewDirection, mWorldUpVector);
}
