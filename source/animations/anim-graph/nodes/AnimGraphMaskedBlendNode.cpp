#include "AnimGraphMaskedBlendNode.h"

#include "animations/AnimationsData.h"
#include "animations/Animator.h"
#include "animations/anim-graph/AnimGraphLink.h"
#include "animations/anim-graph/AnimationContext.h"

Core::Animations::AnimGraphMaskedBlendNode::AnimGraphMaskedBlendNode()
{
    mInputA = createInputPin(AnimGraphValueType::Pose);

    mInputB = createInputPin(AnimGraphValueType::Pose);

    mOutputPin = createOutputPin(AnimGraphValueType::Pose);
}

Core::Animations::Pose Core::Animations::AnimGraphMaskedBlendNode::evaluate(AnimationContext& context) const
{
    const auto* graph = context.graph;
    if (!graph)
    {
        return context.skeletonData->referencePose;
    }

    const auto* linkA = graph->findLinkByInputPin(mInputA);
    const auto* linkB = graph->findLinkByInputPin(mInputB);
    if (!linkA || !linkB)
    {
        return context.skeletonData->referencePose;
    }

    const auto* nodeA = graph->findNodeByPin(linkA->startPin);
    const auto* nodeB = graph->findNodeByPin(linkB->startPin);
    if (!nodeA || !nodeB)
    {
        return context.skeletonData->referencePose;
    }

    const Pose poseA = nodeA->evaluate(context);
    const Pose poseB = nodeB->evaluate(context);

    float alpha = 0.5f;
    if (const auto* property = getProperty("alpha"))
    {
        alpha = std::get<float>(*property);
    }

    int maskIndex = -1;
    if (const auto* property = getProperty("maskIndex"))
    {
        maskIndex = std::get<int>(*property);
    }

    if (maskIndex < 0 || maskIndex >= context.meshComponent->getMasksCount())
    {
        return context.skeletonData->referencePose;
    }

    const auto& mask = context.meshComponent->getMask(maskIndex);

    return Animator::blendMaskedPoses(poseA, poseB, *context.skeletonData, mask, alpha);
}