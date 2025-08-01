#include "Animator.h"
#include "AnimationsUtils.h"

void Core::Animations::Animator::update(const Renderer::VkRenderData& renderData, Renderer::Mesh* mesh)
{
    updateBonesTransform(mesh, renderData.selectedAnimationIndexToPlay);
    mAnimationTime += renderData.rdTickDiff;
}

void Core::Animations::Animator::updateBonesTransform(Renderer::Mesh* mesh, int animationToPlayIndex)
{
    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        Animations::BonesInfo& bonesInfo = primitive.getBonesInfo();
        size_t bonesInfoSize = bonesInfo.bones.size();
        bonesInfo.finalTransforms.resize(bonesInfoSize, glm::mat4(1.0));

        if (!mesh->hasAnimations())
        {
            continue;
        }

        const auto& animations = mesh->getAnimations();
        const Core::Animations::AnimationClip& animation = animations[animationToPlayIndex];

        float duration = animation.duration;
        float ticksPerSecond = animation.ticksPerSecond != 0 ? animation.ticksPerSecond : 30.f;
        float timeInTicks = fmod(mAnimationTime * ticksPerSecond, duration);

        const Core::Animations::Skeleton& skeleton = mesh->getSkeleton();

        readNodeHierarchyClip(animation, timeInTicks, skeleton.getRootNode(),
                              glm::mat4(1.0f),bonesInfo, skeleton);

        for (size_t i = 0; i < bonesInfoSize; ++i)
        {
            bonesInfo.finalTransforms[i] = bonesInfo.bones[i].finalTransform;
        }
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

void Core::Animations::Animator::readNodeHierarchyClip(const AnimationClip& clip, float animationTime,
                                                                const BoneNode& node, const glm::mat4& parentTransform,
                                                                BonesInfo& bonesInfo, const Skeleton& skeleton)
{
    std::string nodeName = node.name;
    glm::mat4 nodeTransform = node.localTransform;

    const AnimationChannel* channel = findChannel(clip, nodeName);
    if (channel)
    {
        glm::vec3 position = interpolatePositionClip(channel->positions, animationTime);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

        glm::quat rotation = interpolateRotationClip(channel->rotations, animationTime);
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);

        glm::vec3 scale = interpolateScaleClip(channel->scalings, animationTime);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

        nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
    }
    else
    {
        nodeTransform = glm::mat4(1.f);
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (bonesInfo.boneNameToIndexMap.contains(nodeName))
    {
        int boneIndex = bonesInfo.boneNameToIndexMap[nodeName];
        bonesInfo.bones[boneIndex].animatedGlobalTransform = globalTransform;
        bonesInfo.bones[boneIndex].finalTransform = globalTransform * bonesInfo.bones[boneIndex].offset;
    }

    for (const BoneNode& child : node.children)
    {
        readNodeHierarchyClip(clip, animationTime, child, globalTransform, bonesInfo, skeleton);
    }
}
