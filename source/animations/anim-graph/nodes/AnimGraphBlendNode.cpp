#include "AnimGraphBlendNode.h"

#include "animations/AnimationsData.h"
#include "animations/Animator.h"
#include "animations/anim-graph/AnimGraphLink.h"
#include "animations/anim-graph/AnimationContext.h"

Core::Animations::AnimGraphBlendNode::AnimGraphBlendNode()
{
    mInputA = createInputPin(AnimGraphValueType::Pose);

    mInputB = createInputPin(AnimGraphValueType::Pose);

    mOutputPin = createOutputPin(AnimGraphValueType::Pose);

    setProperty("alpha", 0.5f);
}

Core::Animations::Pose Core::Animations::AnimGraphBlendNode::evaluate(AnimationContext& context) const
{
    const auto* graph = context.graph;
    if (!graph)
    {
        return {};
    }

    const auto* linkA = graph->findLinkByInputPin(mInputA);
    const auto* linkB = graph->findLinkByInputPin(mInputB);
    if (!linkA || !linkB)
    {
        return {};
    }

    const auto* nodeA = graph->findNodeByPin(linkA->startPin);
    const auto* nodeB = graph->findNodeByPin(linkB->startPin);
    if (!nodeA || !nodeB)
    {
        return {};
    }

    const Pose poseA = nodeA->evaluate(context);
    const Pose poseB = nodeB->evaluate(context);

    float alpha = 0.5f;
    if (const auto* property = getProperty("alpha"))
    {
        alpha = std::get<float>(*property);
    }

    return Animator::blendPoses(poseA, poseB, alpha);
}