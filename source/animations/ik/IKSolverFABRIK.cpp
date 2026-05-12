#include "IKSolverFABRIK.h"
#include "animations/AnimationsData.h"
#include "engine/Engine.h"

void Core::Animations::IKSolverFABRIK::solve(const Resources::SkeletonData& skeletonData, BonesInfo& bonesInfo,
                                             const BoneNode& rootNode)
{
    if (!mTarget)
    {
        return;
    }

    if (mChainIndices.size() < 2)
    {
        return;
    }

    std::vector<glm::vec3> positions;
    std::vector<float> lengths;

    for (const int index : mChainIndices)
    {
        positions.push_back(glm::vec3(bonesInfo.bones[index].animatedGlobalTransform[3]));
    }

    for (size_t i = 0; i < positions.size() - 1; ++i)
    {
        lengths.push_back(glm::distance(positions[i], positions[i + 1]));
    }

    const glm::vec3 rootPosition = positions.back();
    float totalLength = 0;
    for (const float length : lengths)
    {
        totalLength += length;
    }

    // TODO
    // create SceneContext to not access scene via Engine
    auto* scene = Engine::getInstance().getSystem<Scene::Scene>();

    if (glm::distance(positions.back(), mTarget.resolve(scene)->getTargetWorldPosition()) > totalLength)
    {
        for (size_t i = positions.size() - 1; i > 0; --i)
        {
            const float distanceToTarget =
                glm::distance(mTarget.resolve(scene)->getTargetWorldPosition(), positions[i]);
            const float boneLengthRatio = lengths[i - 1] / distanceToTarget;
            positions[i - 1] = (1.0f - boneLengthRatio) * positions[i] +
                               boneLengthRatio * mTarget.resolve(scene)->getTargetWorldPosition();
        }
    }
    else
    {
        for (unsigned int iteration = 0; iteration < mMaxIterations; ++iteration)
        {
            positions[0] = mTarget.resolve(scene)->getTargetWorldPosition();
            for (size_t i = 0; i < positions.size() - 1; ++i)
            {
                const float currentDistance = glm::distance(positions[i + 1], positions[i]);
                const float stretchingRatio = lengths[i] / currentDistance;
                positions[i + 1] = (1.0f - stretchingRatio) * positions[i] + stretchingRatio * positions[i + 1];
            }

            positions.back() = rootPosition;
            for (size_t i = positions.size() - 1; i > 0; --i)
            {
                const float currentDistance = glm::distance(positions[i - 1], positions[i]);
                const float stretchingRatio = lengths[i - 1] / currentDistance;
                positions[i - 1] = (1.0f - stretchingRatio) * positions[i] + stretchingRatio * positions[i - 1];
            }

            if (glm::distance(positions[0], mTarget.resolve(scene)->getTargetWorldPosition()) < mThreshold)
            {
                break;
            }
        }
    }

    for (size_t i = mChainIndices.size() - 1; i > 0; --i)
    {
        const int currentBoneIndex = mChainIndices[i];
        const int childBoneIndex = mChainIndices[i - 1];

        glm::vec3 currentBoneDirection =
            glm::normalize(glm::vec3(bonesInfo.bones[childBoneIndex].animatedGlobalTransform[3]) -
                           glm::vec3(bonesInfo.bones[currentBoneIndex].animatedGlobalTransform[3]));
        glm::vec3 targetBoneDirection = glm::normalize(positions[i - 1] - positions[i]);

        if (const float dot = glm::dot(currentBoneDirection, targetBoneDirection); dot < 0.9999f)
        {
            const float angle = acos(glm::clamp(dot, -1.0f, 1.0f));
            glm::vec3 axis = glm::normalize(glm::cross(currentBoneDirection, targetBoneDirection));
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, axis);

            applyRotationToHierarchy(skeletonData, rootNode, currentBoneIndex, rotation, bonesInfo, false);
        }

        bonesInfo.bones[currentBoneIndex].animatedGlobalTransform[3] = glm::vec4(positions[i], 1.0f);
    }

    const int effectorIdx = mChainIndices.front();
    bonesInfo.bones[effectorIdx].animatedGlobalTransform[3] = glm::vec4(positions[0], 1.0f);

    updateFullHierarchy(skeletonData, rootNode, glm::mat4(1.0f), bonesInfo);
}

void Core::Animations::IKSolverFABRIK::updateFullHierarchy(const Resources::SkeletonData& skeletonData,
                                                           const BoneNode& node, const glm::mat4& parentTransform,
                                                           BonesInfo& bonesInfo)
{
    glm::mat4 currentGlobalTransform;

    if (const auto it = skeletonData.boneNameToIndexMap.find(node.name); it != skeletonData.boneNameToIndexMap.end())
    {
        const int index = it->second;

        currentGlobalTransform = bonesInfo.bones[index].animatedGlobalTransform;

        bool isIKBone = false;
        for (const int chainIndex : mChainIndices)
        {
            if (chainIndex == index)
            {
                isIKBone = true;
                break;
            }
        }

        if (!isIKBone)
        {
            currentGlobalTransform = parentTransform * bonesInfo.localTransforms[index];
            bonesInfo.bones[index].animatedGlobalTransform = currentGlobalTransform;
        }
    }
    else
    {
        currentGlobalTransform = parentTransform * node.localTransform;
    }

    for (const auto& child : node.children)
    {
        updateFullHierarchy(skeletonData, child, currentGlobalTransform, bonesInfo);
    }
}