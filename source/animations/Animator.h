#pragma once

#include "vk-renderer/VkRenderData.h"
#include "components/MeshComponent.h"
#include "system/System.h"
#include "system/Updatable.h"
#include "tools/Timer.h"

namespace Core::Animations
{
class Animator : public System::ISystem, public System::IUpdatable
{
public:
    void update(Renderer::VkRenderData& renderData, float deltaTime) override;

    void addMesh(Component::MeshComponent* mesh) { mMeshes.push_back(mesh); }

    void removeMesh(Component::MeshComponent* mesh)
    {
        if (auto it = std::find(mMeshes.begin(), mMeshes.end(), mesh); it != mMeshes.end())
        {
            mMeshes.erase(it);
        }
    }

    [[nodiscard]] static Pose sampleClip(const AnimationClip& clip, float time,
                                         const Resources::SkeletonData& skeletonData, const BoneNode& rootNode);

    [[nodiscard]] static Pose blendPoses(const Pose& poseA, const Pose& poseB, float blendFactor);

private:
    std::vector<Component::MeshComponent*> mMeshes;

    void updateBonesTransform(Component::MeshComponent* mesh);

    void buildGlobalTransforms(const Pose& pose, const BoneNode& rootNode, const Resources::SkeletonData& skeletonData,
                               BonesInfo& bonesInfo);

    void buildGlobalTransformsRecursive(const Pose& pose, const BoneNode& node, const glm::mat4& parentTransform,
                                        const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo);

    static void sampleClipRecursive(const AnimationClip& clip, float time, const BoneNode& node,
                                    const Resources::SkeletonData& skeletonData, Pose& pose);

    static glm::vec3 interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    static glm::quat interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes, float animationTime);

    static glm::vec3 interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    static BoneTransform getBoneTransform(const AnimationChannel* channel, float time);

    static BoneTransform blendTransforms(const BoneTransform& transformA, const BoneTransform& transformB,
                                         float blendFactor);

    Timer mAnimationBonesTransformCalculationTimer;
};
} // namespace Core::Animations
