#include "AnimGraphIKNode.h"

#include "animations/anim-graph/AnimGraph.h"
#include "animations/anim-graph/AnimationContext.h"

Core::Animations::AnimGraphIKNode::AnimGraphIKNode()
{
    mInputPin = createInputPin(AnimGraphValueType::Pose);
    mOutputPin = createOutputPin(AnimGraphValueType::Pose);
}

Core::Animations::Pose Core::Animations::AnimGraphIKNode::evaluate(AnimationContext& context) const
{
    const auto* graph = context.graph;
    if (!graph)
    {
        return {};
    }

    const auto* link = graph->findLinkByInputPin(mInputPin);
    if (!link)
    {
        return {};
    }

    const auto* sourceNode = graph->findNodeByPin(link->startPin);
    if (!sourceNode)
    {
        return {};
    }

    Pose pose = sourceNode->evaluate(context);

    for (const auto& solver : mSolvers)
    {
        solver->solve(*context.skeletonData, pose);
    }

    return pose;
}