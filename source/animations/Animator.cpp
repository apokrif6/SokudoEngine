#include "Animator.h"
#include "AnimationsUtils.h"
#include "anim-graph/AnimationContext.h"

void Core::Animations::Animator::update(Renderer::VkRenderData& renderData, float deltaTime)
{
    mAnimationBonesTransformCalculationTimer.start();
    for (Component::MeshComponent* mesh : mMeshes)
    {
        if (!mesh->hasAnimations() || !mesh->shouldPlayAnimation())
        {
            continue;
        }

        const auto& clip = mesh->getAnimations()[mesh->getCurrentAnimationIndex()];
        const float ticksPerSecond = clip.ticksPerSecond != 0 ? clip.ticksPerSecond : 30.0f;

        const float newTime = mesh->getCurrentAnimationTime() + deltaTime * ticksPerSecond;
        mesh->setAnimationTime(fmod(newTime, clip.duration));

        updateBonesTransform(mesh);
    }

    renderData.rdAnimationBonesTransformCalculationTime = mAnimationBonesTransformCalculationTimer.stop();
}

void Core::Animations::Animator::updateBonesTransform(Component::MeshComponent* mesh)
{
    // TODO
    // waiting for AnimInstance
    return;

    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        BonesInfo& bonesInfo = primitive.getBonesInfo();
        const size_t bonesInfoSize = bonesInfo.bones.size();
        bonesInfo.finalTransforms.resize(bonesInfoSize, glm::mat4(1.0));
        bonesInfo.localTransforms.resize(bonesInfoSize, glm::mat4(1.0f));

        const Skeleton& skeleton = mesh->getSkeleton();

        AnimationContext context;
        context.skeletonData = skeleton.getSkeletonData();

        Pose pose;

        buildGlobalTransforms(pose, skeleton.getRootNode(), *skeleton.getSkeletonData(), bonesInfo);

        for (size_t i = 0; i < bonesInfoSize; ++i)
        {
            bonesInfo.finalTransforms[i] = bonesInfo.bones[i].animatedGlobalTransform * bonesInfo.bones[i].offset;
        }
    }
}

void Core::Animations::Animator::buildGlobalTransforms(const Pose& pose, const BoneNode& rootNode,
                                                       const Resources::SkeletonData& skeletonData,
                                                       BonesInfo& bonesInfo)
{
    buildGlobalTransformsRecursive(pose, rootNode, glm::mat4(1.0f), skeletonData, bonesInfo);
}

void Core::Animations::Animator::buildGlobalTransformsRecursive(const Pose& pose, const BoneNode& node,
                                                                const glm::mat4& parentTransform,
                                                                const Resources::SkeletonData& skeletonData,
                                                                BonesInfo& bonesInfo)
{
    glm::mat4 localMatrix = node.localTransform;
    glm::mat4 globalTransform;

    if (const auto it = skeletonData.boneNameToIndexMap.find(node.name); it != skeletonData.boneNameToIndexMap.end())
    {
        const int boneIndex = it->second;

        localMatrix = pose.localTransforms[boneIndex].toMatrix();

        bonesInfo.localTransforms[boneIndex] = localMatrix;

        globalTransform = parentTransform * localMatrix;

        bonesInfo.bones[boneIndex].animatedGlobalTransform = globalTransform;
    }
    else
    {
        globalTransform = parentTransform * localMatrix;
    }

    for (const auto& child : node.children)
    {
        buildGlobalTransformsRecursive(pose, child, globalTransform, skeletonData, bonesInfo);
    }
}

const Core::Animations::AnimationChannel* findChannel(const Core::Animations::AnimationClip& clip,
                                                      const std::string& boneName)
{
    for (const auto& channel : clip.channels)
    {
        if (channel.boneName == boneName)
        {
            return &channel;
        }
    }
    return nullptr;
}

Core::Animations::Pose Core::Animations::Animator::sampleClip(const AnimationClip& clip, const float time,
                                                              const Resources::SkeletonData& skeletonData,
                                                              const BoneNode& rootNode)
{
    Pose pose;

    const size_t boneCount = skeletonData.boneNameToIndexMap.size();

    pose.localTransforms.resize(boneCount);

    sampleClipRecursive(clip, time, rootNode, skeletonData, pose);

    return pose;
}

Core::Animations::Pose Core::Animations::Animator::blendPoses(const Pose& poseA, const Pose& poseB, float blendFactor)
{
    Pose result;

    const size_t boneCount = poseA.localTransforms.size();

    result.localTransforms.resize(boneCount);

    for (size_t i = 0; i < boneCount; ++i)
    {
        result.localTransforms[i] = blendTransforms(poseA.localTransforms[i], poseB.localTransforms[i], blendFactor);
    }

    return result;
}

void Core::Animations::Animator::sampleClipRecursive(const AnimationClip& clip, const float time, const BoneNode& node,
                                                     const Resources::SkeletonData& skeletonData, Pose& pose)
{
    BoneTransform localTransform;

    if (const AnimationChannel* channel = findChannel(clip, node.name))
    {
        localTransform = getBoneTransform(channel, time);
    }
    else
    {
        localTransform = BoneTransform{node.localTransform};
    }

    if (const auto it = skeletonData.boneNameToIndexMap.find(node.name); it != skeletonData.boneNameToIndexMap.end())
    {
        const int boneIndex = it->second;

        pose.localTransforms[boneIndex] = localTransform;
    }

    for (const BoneNode& child : node.children)
    {
        sampleClipRecursive(clip, time, child, skeletonData, pose);
    }
}

glm::vec3 Core::Animations::Animator::interpolatePositionClip(const std::vector<KeyframeVec3>& keyframes,
                                                              float animationTime)
{
    if (keyframes.size() == 1)
    {
        return keyframes[0].value;
    }

    for (size_t i = 0; i < keyframes.size() - 1; ++i)
    {
        if (animationTime < keyframes[i + 1].time)
        {
            float t = (animationTime - keyframes[i].time) / (keyframes[i + 1].time - keyframes[i].time);

            glm::vec3 start = keyframes[i].value;
            glm::vec3 end = keyframes[i + 1].value;

            return glm::mix(start, end, t);
        }
    }
    return keyframes.back().value;
}

glm::quat Core::Animations::Animator::interpolateRotationClip(const std::vector<KeyframeQuat>& keyframes,
                                                              float animationTime)
{
    if (keyframes.size() == 1)
    {
        return keyframes[0].value;
    }

    for (size_t i = 0; i < keyframes.size() - 1; ++i)
    {
        if (animationTime < keyframes[i + 1].time)
        {
            float t = (animationTime - keyframes[i].time) / (keyframes[i + 1].time - keyframes[i].time);

            glm::quat start = keyframes[i].value;
            glm::quat end = keyframes[i + 1].value;

            return glm::slerp(start, end, t);
        }
    }
    return keyframes.back().value;
}

glm::vec3 Core::Animations::Animator::interpolateScaleClip(const std::vector<KeyframeVec3>& keyframes,
                                                           float animationTime)
{
    return interpolatePositionClip(keyframes, animationTime);
}

Core::Animations::BoneTransform Core::Animations::Animator::getBoneTransform(const AnimationChannel* channel,
                                                                             float time)
{
    return BoneTransform{interpolatePositionClip(channel->positions, time),
                         interpolateRotationClip(channel->rotations, time),
                         interpolateScaleClip(channel->scalings, time)};
}

Core::Animations::BoneTransform Core::Animations::Animator::blendTransforms(const BoneTransform& transformA,
                                                                            const BoneTransform& transformB,
                                                                            float blendFactor)
{
    return BoneTransform{glm::mix(transformA.position, transformB.position, blendFactor),
                         glm::slerp(transformA.rotation, transformB.rotation, blendFactor),
                         glm::mix(transformA.scale, transformB.scale, blendFactor)};
}
