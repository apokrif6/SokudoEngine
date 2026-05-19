#include "AnimGraphClipNode.h"

#include "animations/Animator.h"
#include "animations/anim-graph/AnimationContext.h"

Core::Animations::Pose Core::Animations::AnimGraphClipNode::evaluate(AnimationContext& context) const
{
    auto& runtime = context.instance->getRuntime(getUUID());

    if (!context.animations || context.animations->empty())
    {
        return {};
    }

    int clipIndex = -1;
    if (auto* property = getProperty("clipIndex"))
    {
        clipIndex = std::get<int>(*property);
    }

    if (clipIndex < 0 || clipIndex >= context.animations->size())
    {
        return {};
    }

    const auto& clip = (*context.animations)[clipIndex];

    const float ticksPerSecond = clip.ticksPerSecond != 0 ? clip.ticksPerSecond : 30.f;

    runtime.time += context.deltaTime * ticksPerSecond;
    runtime.time = fmod(runtime.time, clip.duration);

    return Animator::sampleClip(clip, runtime.time, *context.skeletonData, context.skeletonData->rootNode);
}