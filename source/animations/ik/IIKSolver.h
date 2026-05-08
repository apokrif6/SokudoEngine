#pragma once

#include <vector>
#include "resources/Mesh.h"
#include <glm/fwd.hpp>
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
    explicit IIKSolver(const std::vector<int>& chainIndices, const unsigned int maxIterations = 10,
                       const float threshold = 0.001f)
    {
        mChainIndices = chainIndices;
        mMaxIterations = maxIterations;
        mThreshold = threshold;
    }

    virtual ~IIKSolver() = default;

    [[nodiscard]] virtual AnimationSolverType getType() const = 0;

    virtual void solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo, const BoneNode& rootNode) = 0;

    void setTarget(const glm::vec3& target) { mTargetPosition = target; }

    void setMaxIterations(const unsigned int maxIterations) { mMaxIterations = maxIterations; }

    [[nodiscard]] unsigned int getMaxIterations() const { return mMaxIterations; }

    [[nodiscard]] std::vector<int> getChainIndices() const { return mChainIndices; }

protected:
    void applyRotationToHierarchy(const Resources::SkeletonData& skeletonData, const BoneNode& node,
                                  const int targetBoneIndex, const glm::mat4& rotation, BonesInfo& bonesInfo,
                                  bool found)
    {
        if (!found)
        {
            if (const auto it = skeletonData.boneNameToIndexMap.find(node.name);
                it != skeletonData.boneNameToIndexMap.end() && it->second == targetBoneIndex)
            {
                found = true;
            }
        }

        if (found)
        {
            if (const auto it = skeletonData.boneNameToIndexMap.find(node.name);
                it != skeletonData.boneNameToIndexMap.end())
            {
                const int index = it->second;
                bonesInfo.bones[index].animatedGlobalTransform =
                    rotation * bonesInfo.bones[index].animatedGlobalTransform;
            }
        }

        for (const auto& child : node.children)
        {
            applyRotationToHierarchy(skeletonData, child, targetBoneIndex, rotation, bonesInfo, found);
        }
    }

    std::vector<int> mChainIndices;
    glm::vec3 mTargetPosition{0.0f};
    unsigned int mMaxIterations = 15;
    float mThreshold = 0.001f;
};
} // namespace Core::Animations
