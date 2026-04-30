#pragma once

#include "IIKSolver.h"
#include "resources/Mesh.h"

#include <vector>
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>

namespace Core::Animations
{
struct BoneNode;
struct BonesInfo;
class IKSolverCCD : public IIKSolver
{
public:
    explicit IKSolverCCD(const std::vector<int>& chainIndices, const unsigned int maxIterations = 10)
        : mChainIndices(chainIndices), mMaxIterations(maxIterations)
    {
    }

    void solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo, const BoneNode& rootNode) override;

    void setTarget(const glm::vec3& target) override { mTargetPosition = target; }

    void setMaxIterations(const unsigned int maxIterations) { mMaxIterations = maxIterations; }

    [[nodiscard]] std::vector<int> getChainIndices() const { return mChainIndices; }

private:
    void applyRotationToHierarchy(const Resources::SkeletonData& skeletonData, const BoneNode& node,
                                  int targetBoneIndex, const glm::mat4& rotation, BonesInfo& bonesInfo, bool found);

    std::vector<int> mChainIndices;
    glm::vec3 mTargetPosition{0.0f};
    unsigned int mMaxIterations = 15;
    float mThreshold = 0.001f;
};
} // namespace Core::Animations
