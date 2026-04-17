#include "Animator.h"
#include "AnimationsUtils.h"

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

        updateBonesTransform(mesh, mesh->getCurrentAnimationIndex());
    }

    renderData.rdAnimationBonesTransformCalculationTime = mAnimationBonesTransformCalculationTimer.stop();
}

void Core::Animations::Animator::updateBonesTransform(Component::MeshComponent* mesh, uint16_t animationToPlayIndex)
{
    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        BonesInfo& bonesInfo = primitive.getBonesInfo();
        size_t bonesInfoSize = bonesInfo.bones.size();
        bonesInfo.finalTransforms.resize(bonesInfoSize, glm::mat4(1.0));

        if (!mesh->hasAnimations())
        {
            continue;
        }

        const auto& animations = mesh->getAnimations();
        const AnimationClip& animation = animations[animationToPlayIndex];

        const float timeInTicks = mesh->getCurrentAnimationTime();

        const Skeleton& skeleton = mesh->getSkeleton();

        readNodeHierarchyClip(animation, timeInTicks, skeleton.getRootNode(), glm::mat4(1.0f), bonesInfo, skeleton);

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
    const std::string nodeName = node.name;
    glm::mat4 nodeTransform;

    if (const AnimationChannel* channel = findChannel(clip, nodeName))
    {
        nodeTransform = getBoneTransform(channel, animationTime).toMatrix();
    }
    else
    {
        nodeTransform = glm::mat4(1.f);
    }

    const glm::mat4 globalTransform = parentTransform * nodeTransform;

    if (bonesInfo.boneNameToIndexMap.contains(nodeName))
    {
        const int boneIndex = bonesInfo.boneNameToIndexMap[nodeName];
        bonesInfo.bones[boneIndex].animatedGlobalTransform = globalTransform;
        bonesInfo.bones[boneIndex].finalTransform = globalTransform * bonesInfo.bones[boneIndex].offset;
    }

    for (const BoneNode& child : node.children)
    {
        readNodeHierarchyClip(clip, animationTime, child, globalTransform, bonesInfo, skeleton);
    }
}

Core::Animations::BoneTransform Core::Animations::Animator::getBoneTransform(const AnimationChannel* channel,
                                                                             float time)
{
    return BoneTransform{interpolatePositionClip(channel->positions, time),
                         interpolateRotationClip(channel->rotations, time),
                         interpolateScaleClip(channel->scalings, time)};
}
