#pragma once

#include "assimp/scene.h"
#include "core/vk-renderer/VkRenderData.h"
#include "core/scene/objects/Mesh.h"
#include "core/system/System.h"
#include "core/system/Updatable.h"
#include <core/tools/Timer.h>

namespace Core::Animations
{
class Animator : public System::ISystem, public System::IUpdatable
{
  public:
    virtual void update(Renderer::VkRenderData& renderData, float deltaTime) override;

    void addMesh(Renderer::Mesh* mesh)
    {
        mMeshes.push_back(mesh);
    }

  private:
    std::vector<Renderer::Mesh*> mMeshes;

    void updateBonesTransform(Renderer::Mesh* mesh, uint16_t animationToPlayIndex);

    glm::vec3 interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    glm::quat interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes, float animationTime);

    glm::vec3 interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    void readNodeHierarchyClip(const AnimationClip& clip, float animationTime, const BoneNode& node,
                               const glm::mat4& parentTransform, BonesInfo& bonesInfo, const Skeleton& skeleton);

    float mAnimationTime = 0.f;

    Timer mAnimationBonesTransformCalculationTimer;
};
} // namespace Core::Animations
