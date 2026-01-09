#pragma once

#include "assimp/scene.h"
#include "core/vk-renderer/VkRenderData.h"
#include "../components/MeshComponent.h"
#include "core/system/System.h"
#include "core/system/Updatable.h"
#include <core/tools/Timer.h>

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

    void updateBonesTransform(Component::MeshComponent* mesh, uint16_t animationToPlayIndex);

    glm::vec3 interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    glm::quat interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes, float animationTime);

    glm::vec3 interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes, float animationTime);

    void readNodeHierarchyClip(const AnimationClip& clip, float animationTime, const BoneNode& node,
                               const glm::mat4& parentTransform, BonesInfo& bonesInfo, const Skeleton& skeleton);

    float mAnimationTime = 0.f;

    Timer mAnimationBonesTransformCalculationTimer;
};
} // namespace Core::Animations
