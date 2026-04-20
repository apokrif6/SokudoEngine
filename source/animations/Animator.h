#pragma once

#include "assimp/scene.h"
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

private:
    std::vector<Component::MeshComponent*> mMeshes;

    void updateBonesTransform(Component::MeshComponent* mesh);

    glm::vec3 interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    glm::quat interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes, float animationTime);

    glm::vec3 interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    void readNodeHierarchyClip(const AnimationClip& clip, float animationTime, const BoneNode& node,
                               const glm::mat4& parentTransform, BonesInfo& bonesInfo);

    void readNodeHierarchyBlend(const AnimationClip& clipA, float animationTimeA, const AnimationClip& clipB,
                                float animationTimeB, Component::MeshComponent* meshComponent, const BoneNode& node,
                                const glm::mat4& parentTransform, BonesInfo& bonesInfo);

    BoneTransform getBoneTransform(const AnimationChannel* channel, float time);

    BoneTransform blendTransforms(const BoneTransform& transformA, const BoneTransform& transformB, float blendFactor);

    Timer mAnimationBonesTransformCalculationTimer;
};
} // namespace Core::Animations
