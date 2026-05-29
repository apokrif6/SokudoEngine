#include "AnimGraphOutputPoseNode.h"

#include "animations/AnimationsData.h"
#include "animations/Animator.h"
#include "animations/anim-graph/AnimGraph.h"
#include "animations/anim-graph/AnimGraphLink.h"
#include "animations/anim-graph/AnimationContext.h"

Core::Animations::AnimGraphOutputPoseNode::AnimGraphOutputPoseNode()
{
    mInputPin = createInputPin(AnimGraphValueType::Pose);
}

Core::Animations::Pose Core::Animations::AnimGraphOutputPoseNode::evaluate(AnimationContext& context) const
{
    const auto* link = context.graph->findLinkByInputPin(mInputPin);
    if (!link)
    {
        return context.skeletonData->referencePose;
    }

    const auto* node = context.graph->findNodeByPin(link->startPin);
    if (!node)
    {
        return context.skeletonData->referencePose;
    }

    return node->evaluate(context);
}
