#include "IKSolverCCD.h"
#include "animations/AnimationsData.h"
#include "animations/AnimationsUtils.h"
#include "engine/Engine.h"
#include "resources/Mesh.h"

void Core::Animations::IKSolverCCD::solve(const Resources::SkeletonData& skeletonData, Pose& pose)
{
    if (!mTarget)
    {
        return;
    }

    if (mChainIndices.size() < 2)
    {
        return;
    }

    const int effectorIndex = mChainIndices.front();

    // TODO
    // create SceneContext to not access scene via Engine
    auto* scene = Engine::getInstance().getSystem<Scene::Scene>();
    const glm::vec3 targetPosition = mTarget.resolve(scene)->getTargetWorldPosition();

    PoseGlobalData globals;
    globals.globalTransforms.resize(skeletonData.boneNameToIndexMap.size());

    for (uint32_t iteration = 0; iteration < mMaxIterations; ++iteration)
    {
        AnimationsUtils::buildPoseGlobalTransforms(pose, skeletonData.rootNode, skeletonData, globals);

        auto effectorPosition = glm::vec3(globals.globalTransforms[effectorIndex][3]);

        if (glm::distance(effectorPosition, targetPosition) < mThreshold)
        {
            break;
        }

        // TODO
        // implement constraints
        for (size_t i = 1; i < mChainIndices.size(); ++i)
        {
            const int jointIndex = mChainIndices[i];

            auto jointPosition = glm::vec3(globals.globalTransforms[jointIndex][3]);
            effectorPosition = glm::vec3(globals.globalTransforms[effectorIndex][3]);

            glm::vec3 toEffector = glm::normalize(effectorPosition - jointPosition);

            glm::vec3 toTarget = glm::normalize(targetPosition - jointPosition);

            float dot = glm::clamp(glm::dot(toEffector, toTarget), -1.0f, 1.0f);

            if (dot > 0.9999f)
            {
                continue;
            }

            float angle = acos(dot);

            glm::vec3 axis = glm::normalize(glm::cross(toEffector, toTarget));

            glm::quat worldDelta = glm::angleAxis(angle, axis);

            int parentIndex = skeletonData.boneParents[jointIndex];

            glm::quat parentWorldRot(1, 0, 0, 0);

            if (parentIndex >= 0)
            {
                glm::mat4 parentGlobal = globals.globalTransforms[parentIndex];

                parentWorldRot = glm::quat_cast(parentGlobal);
            }

            glm::quat localDelta = glm::inverse(parentWorldRot) * worldDelta * parentWorldRot;

            pose.localTransforms[jointIndex].rotation =
                glm::normalize(localDelta * pose.localTransforms[jointIndex].rotation);
        }
    }
}
