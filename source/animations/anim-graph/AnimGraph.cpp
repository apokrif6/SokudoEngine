#include "AnimGraph.h"

#include "animations/AnimationsData.h"

Core::Animations::Pose Core::Animations::AnimGraph::evaluate(AnimationContext& context) const
{
    return mRoot->evaluate(context);
}