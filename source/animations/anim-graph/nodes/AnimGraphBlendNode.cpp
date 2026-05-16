#include "AnimGraphBlendNode.h"

#include "animations/AnimationsData.h"
#include "animations/Animator.h"

Core::Animations::Pose Core::Animations::AnimGraphBlendNode::evaluate(AnimationContext& context)
{
    const Pose poseA = mNodeA->evaluate(context);
    const Pose poseB = mNodeB->evaluate(context);

    return Animator::blendPoses(poseA, poseB, mAlpha);
}