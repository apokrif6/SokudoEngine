#include "IKSolverCCD.h"
#include "animations/AnimationsData.h"
#include "resources/Mesh.h"

void Core::Animations::IKSolverCCD::solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo,
                                          const BoneNode& rootNode)
{
    if (mChainIndices.size() < 2)
    {
        return;
    }

    const int effectorIndex = mChainIndices.front();

    for (unsigned int iteration = 0; iteration < mMaxIterations; ++iteration)
    {
        for (size_t i = 1; i < mChainIndices.size(); ++i)
        {
            const int jointIndex = mChainIndices[i];

            auto jointPosition = glm::vec3(bonesInfo.bones[jointIndex].animatedGlobalTransform[3]);
            auto effectorPosition = glm::vec3(bonesInfo.bones[effectorIndex].animatedGlobalTransform[3]);

            glm::vec3 jointToEffector = glm::normalize(effectorPosition - jointPosition);
            glm::vec3 jointToTarget = glm::normalize(mTargetPosition - jointPosition);

            const float cosAngle = glm::dot(jointToEffector, jointToTarget);
            if (cosAngle > 0.9999f)
            {
                continue;
            }

            const float angle = acos(glm::clamp(cosAngle, -1.0f, 1.0f));
            glm::vec3 rotAxis = glm::normalize(glm::cross(jointToEffector, jointToTarget));

            glm::mat4 rotationWorld = glm::translate(glm::mat4(1.0f), jointPosition) *
                                      glm::rotate(glm::mat4(1.0f), angle, rotAxis) *
                                      glm::translate(glm::mat4(1.0f), -jointPosition);

            applyRotationToHierarchy(skeletonData, rootNode, jointIndex, rotationWorld, bonesInfo, false);
        }

        if (const auto effectorPosition = glm::vec3(bonesInfo.bones[effectorIndex].animatedGlobalTransform[3]);
            glm::distance(effectorPosition, mTargetPosition) < mThreshold)
        {
            break;
        }
    }
}

void Core::Animations::IKSolverCCD::applyRotationToHierarchy(const Resources::SkeletonData& skeletonData,
                                                             const BoneNode& node, int targetBoneIndex,
                                                             const glm::mat4& rotation, BonesInfo& bonesInfo,
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
            bonesInfo.bones[index].animatedGlobalTransform = rotation * bonesInfo.bones[index].animatedGlobalTransform;
        }
    }

    for (const auto& child : node.children)
    {
        applyRotationToHierarchy(skeletonData, child, targetBoneIndex, rotation, bonesInfo, found);
    }
}