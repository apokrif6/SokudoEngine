#include "IKSolverFABRIK.h"
#include "animations/AnimationsData.h"
#include "animations/AnimationsUtils.h"
#include "engine/Engine.h"

void Core::Animations::IKSolverFABRIK::solve(const Resources::SkeletonData& skeletonData, Pose& pose)
{
    if (!mTarget)
    {
        return;
    }

    if (mChainIndices.size() < 2)
    {
        return;
    }

    auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
    const glm::vec3 target = mTarget.resolve(scene)->getTargetWorldPosition();

    PoseGlobalData globals;
    globals.globalTransforms.resize(skeletonData.boneNameToIndexMap.size());

    AnimationsUtils::buildPoseGlobalTransforms(pose, skeletonData.rootNode, skeletonData, globals);

    std::vector<glm::vec3> positions;
    positions.reserve(mChainIndices.size());

    for (const int index : mChainIndices)
    {
        positions.push_back(glm::vec3(globals.globalTransforms[index][3]));
    }

    std::vector<float> lengths;
    lengths.reserve(positions.size() - 1);

    float totalLength = 0.0f;

    for (size_t i = 0; i < positions.size() - 1; ++i)
    {
        float length = glm::distance(positions[i], positions[i + 1]);
        lengths.push_back(length);
        totalLength += length;
    }

    if (const glm::vec3 rootPosition = positions.back(); glm::distance(rootPosition, target) > totalLength)
    {
        for (size_t i = positions.size() - 1; i > 0; --i)
        {
            glm::vec3 direction = glm::normalize(target - positions[i]);

            positions[i - 1] = positions[i] + direction * lengths[i - 1];
        }
    }
    else
    {
        for (uint32_t iteration = 0; iteration < mMaxIterations; ++iteration)
        {
            positions[0] = target;

            for (size_t i = 0; i < positions.size() - 1; ++i)
            {
                glm::vec3 direction = glm::normalize(positions[i + 1] - positions[i]);

                positions[i + 1] = positions[i] + direction * lengths[i];
            }

            positions.back() = rootPosition;

            for (size_t i = positions.size() - 1; i > 0; --i)
            {
                glm::vec3 direction = glm::normalize(positions[i - 1] - positions[i]);

                positions[i - 1] = positions[i] + direction * lengths[i - 1];
            }

            if (glm::distance(positions[0], target) < mThreshold)
            {
                break;
            }
        }
    }

    AnimationsUtils::buildPoseGlobalTransforms(pose, skeletonData.rootNode, skeletonData, globals);

    for (size_t i = 1; i < mChainIndices.size(); ++i)
    {
        int joint = mChainIndices[i];
        int child = mChainIndices[i - 1];

        glm::vec3 currentDir = glm::normalize(glm::vec3(globals.globalTransforms[child][3]) -
                                              glm::vec3(globals.globalTransforms[joint][3]));

        glm::vec3 targetDir = glm::normalize(positions[i - 1] - positions[i]);

        float dot = glm::clamp(glm::dot(currentDir, targetDir), -1.0f, 1.0f);

        if (dot > 0.9999f)
        {
            continue;
        }

        float angle = acos(dot);

        glm::vec3 axis = glm::normalize(glm::cross(currentDir, targetDir));

        glm::quat worldDelta = glm::angleAxis(angle, axis);

        int parent = skeletonData.boneParents[joint];

        glm::quat parentRot(1, 0, 0, 0);

        if (parent >= 0)
        {
            parentRot = glm::quat_cast(globals.globalTransforms[parent]);
        }

        glm::quat localDelta = glm::inverse(parentRot) * worldDelta * parentRot;

        pose.localTransforms[joint].rotation = glm::normalize(localDelta * pose.localTransforms[joint].rotation);
    }
}
