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
        return Animator::createReferencePose(*context.skeletonData, context.skeletonData->rootNode);
    }

    const auto* node = context.graph->findNodeByPin(link->startPin);
    if (!node)
    {
        return Animator::createReferencePose(*context.skeletonData, context.skeletonData->rootNode);
    }

    return node->evaluate(context);
}
