#define _USE_MATH_DEFINES
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"

glm::mat4 Camera::getViewMatrix(VkRenderData& renderData)
{
    float azimRad = glm::radians(renderData.rdViewYaw);
    float elevRad = glm::radians(renderData.rdViewPitch);

    float sinAzim = glm::sin(azimRad);
    float cosAzim = glm::cos(azimRad);
    float sinElev = glm::sin(elevRad);
    float cosElev = glm::cos(elevRad);

    mViewDirection = glm::normalize(glm::vec3(sinAzim * cosElev, -sinElev, -cosAzim * cosElev));

    mRightDirection = glm::normalize(glm::cross(mViewDirection, mWorldUpVector));
    mUpDirection = glm::normalize(glm::cross(mRightDirection, mViewDirection));

    renderData.rdCameraWorldPosition += renderData.rdMoveForward * renderData.rdTickDiff * mViewDirection +
                                        renderData.rdMoveRight * renderData.rdTickDiff * mRightDirection +
                                        renderData.rdMoveUp * renderData.rdTickDiff * mUpDirection;

    return glm::lookAt(renderData.rdCameraWorldPosition, renderData.rdCameraWorldPosition + mViewDirection,
                       mUpDirection);
}
