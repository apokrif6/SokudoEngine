#pragma once

#include "assimp/scene.h"
#include "assimp/Importer.hpp"
#include "core/vk-renderer/VkRenderData.h"
#include "core/vk-renderer/Mesh.h"
#include <map>
#include <memory>

// TODO
// create templated class for all singletons
namespace Core::Animations
{
class AnimatorSingleton
{
  public:
    static AnimatorSingleton& getInstance()
    {
        static AnimatorSingleton instance;
        return instance;
    }

    AnimatorSingleton(const AnimatorSingleton&) = delete;
    AnimatorSingleton& operator=(const AnimatorSingleton&) = delete;

    void update(const Renderer::VkRenderData& renderData, Renderer::Mesh* mesh);

    // TODO
    // should be refactored when Scene system will be written. should be stored per mesh
    std::vector<std::string> loadedAnimations{};

  private:
    AnimatorSingleton() = default;
    ~AnimatorSingleton() = default;

    void updateBonesTransform(Renderer::Mesh* mesh, int animationToPlayIndex);

    glm::vec3 interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    glm::quat interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes, float animationTime);

    glm::vec3 interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    // TODO
    // should be refactored via creating custom structs for nodes to not store aiNode* pointers
    void readNodeHierarchyClip(const AnimationClip& clip, float animationTime, const BoneNode& node,
                               const glm::mat4& parentTransform, BonesInfo& bonesInfo, const Skeleton& skeleton);

    float mAnimationTime = 0.f;
};
} // namespace Core::Animations
