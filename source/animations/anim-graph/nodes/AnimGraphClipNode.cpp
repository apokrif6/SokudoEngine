#include "AnimGraphClipNode.h"

#include "animations/Animator.h"
#include "animations/anim-graph/AnimationContext.h"

Core::Animations::Pose Core::Animations::AnimGraphClipNode::evaluate(AnimationContext& context)
{
    return Animator::sampleClip(*mClip, context.currentTime, *context.skeletonData, context.skeletonData->rootNode);
}