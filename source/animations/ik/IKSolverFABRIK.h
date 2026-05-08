#pragma once

#include "IIKSolver.h"

namespace Core::Animations
{
struct BoneNode;
struct BonesInfo;
class IKSolverFABRIK : public IIKSolver
{
public:
    IKSolverFABRIK(const std::vector<int>& chainIndices, const unsigned maxIterations, const float threshold = 0.001f)
        : IIKSolver(chainIndices, maxIterations, threshold)
    {
    }

    [[nodiscard]] AnimationSolverType getType() const override { return AnimationSolverType::FABRIK; }

    void solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo, const BoneNode& rootNode) override;

private:
    void updateFullHierarchy(const Resources::SkeletonData& skeletonData, const BoneNode& node,
                             const glm::mat4& parentTransform, BonesInfo& bonesInfo);
};
} // namespace Core::Animations
