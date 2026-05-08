#pragma once

#include "IIKSolver.h"

namespace Core::Animations
{
struct BoneNode;
struct BonesInfo;
class IKSolverCCD : public IIKSolver
{
public:
    IKSolverCCD(const std::vector<int>& chainIndices, const unsigned maxIterations, const float threshold = 0.001f)
        : IIKSolver(chainIndices, maxIterations, threshold)
    {
    }

    [[nodiscard]] AnimationSolverType getType() const override { return AnimationSolverType::CCD; }

    void solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo, const BoneNode& rootNode) override;
};
} // namespace Core::Animations
