#pragma once

#include "IIKSolver.h"
#include "resources/Mesh.h"
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

namespace Core::Animations
{
struct BoneNode;
struct BonesInfo;
class IKSolverCCD : public IIKSolver
{
public:
    IKSolverCCD(const std::vector<int>& chainIndices, const unsigned maxIterations, const float threshold = 0.001f)
        : IIKSolver(chainIndices, maxIterations), mThreshold(threshold)
    {
    }

    void solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo, const BoneNode& rootNode) override;

    void setTarget(const glm::vec3& target) override { mTargetPosition = target; }

private:
    void applyRotationToHierarchy(const Resources::SkeletonData& skeletonData, const BoneNode& node,
                                  int targetBoneIndex, const glm::mat4& rotation, BonesInfo& bonesInfo, bool found);

    glm::vec3 mTargetPosition{0.0f};
    float mThreshold = 0.001f;
};
} // namespace Core::Animations
