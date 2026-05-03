#pragma once

#include <vector>
#include <glm/vec3.hpp>

namespace Core::Resources
{
struct SkeletonData;
}

namespace Core::Animations
{
struct BonesInfo;
struct BoneNode;
class IIKSolver
{
public:
    explicit IIKSolver(const std::vector<int>& chainIndices, const unsigned int maxIterations = 10)
    {
        mChainIndices = chainIndices;
        mMaxIterations = maxIterations;
    }

    virtual ~IIKSolver() = default;

    virtual void solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo, const BoneNode& rootNode) = 0;

    virtual void setTarget(const glm::vec3& target) = 0;

    void setMaxIterations(const unsigned int maxIterations) { mMaxIterations = maxIterations; }

    [[nodiscard]] std::vector<int> getChainIndices() const { return mChainIndices; }

protected:
    std::vector<int> mChainIndices;
    unsigned int mMaxIterations = 15;
};
} // namespace Core::Animations
