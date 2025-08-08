#pragma once

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "core/vk-renderer/VkRenderData.h"
#include "core/vk-renderer/Mesh.h"
#include <map>
#include <memory>

namespace Core::Animations
{
class Animator
{
  public:
    void update(const Renderer::VkRenderData& renderData);

    void addMesh(Renderer::Mesh* mesh)
    {
        mMeshes.push_back(mesh);
    }

    // TODO
    // should be refactored when Scene system will be written. should be stored per mesh
    std::vector<std::string> loadedAnimations{};

  private:
    std::vector<Renderer::Mesh*> mMeshes;

    void updateBonesTransform(Renderer::Mesh* mesh, int animationToPlayIndex);

    glm::vec3 interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    glm::quat interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes, float animationTime);

    glm::vec3 interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    void readNodeHierarchyClip(const AnimationClip& clip, float animationTime, const BoneNode& node,
                               const glm::mat4& parentTransform, BonesInfo& bonesInfo, const Skeleton& skeleton);

    float mAnimationTime = 0.f;
};
} // namespace Core::Animations
