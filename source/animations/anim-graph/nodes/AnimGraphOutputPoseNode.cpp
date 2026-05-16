#include "AnimGraphOutputPoseNode.h"

#include "animations/AnimationsData.h"

Core::Animations::Pose Core::Animations::AnimGraphOutputPoseNode::evaluate(AnimationContext& context)
{
    return mInput->evaluate(context);
}
