#include "AnimGraphOutputPoseNode.h"

#include "animations/AnimationsData.h"

Core::Animations::Pose Core::Animations::AnimGraphOutputPoseNode::evaluate(AnimationContext& context) const
{
    return mInput ? mInput->evaluate(context) : Pose{};
}
