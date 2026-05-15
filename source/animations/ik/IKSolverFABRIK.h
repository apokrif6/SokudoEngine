#pragma once

#include "IIKSolver.h"

namespace Core::Animations
{
struct BoneNode;
struct BonesInfo;
class IKSolverFABRIK : public IIKSolver
{
public:
    explicit IKSolverFABRIK(const std::vector<int>& chainIndices, const unsigned maxIterations = 10,
                            const float threshold = 0.001f)
        : IIKSolver(chainIndices, maxIterations, threshold)
    {
    }

    [[nodiscard]] AnimationSolverType getType() const override { return AnimationSolverType::FABRIK; }

    void solve(const Resources::SkeletonData& skeletonData, Pose& pose) override;
};
} // namespace Core::Animations
