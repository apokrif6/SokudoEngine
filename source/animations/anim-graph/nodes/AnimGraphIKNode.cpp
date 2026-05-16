#include "AnimGraphIKNode.h"

#include "animations/anim-graph/AnimationContext.h"

Core::Animations::Pose Core::Animations::AnimGraphIKNode::evaluate(AnimationContext& context)
{
    Pose pose = mInput->evaluate(context);

    for (const auto& solver : mSolvers)
    {
        solver->solve(*context.skeletonData, pose);
    }

    return pose;
}