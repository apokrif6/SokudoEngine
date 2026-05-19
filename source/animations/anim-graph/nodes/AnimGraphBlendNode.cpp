#include "AnimGraphBlendNode.h"

#include "animations/AnimationsData.h"
#include "animations/Animator.h"

Core::Animations::Pose Core::Animations::AnimGraphBlendNode::evaluate(AnimationContext& context) const
{
    if (!mNodeA || !mNodeB)
    {
        return {};
    }

    float alpha = 0.5f;
    if (auto* property = getProperty("alpha"))
    {
        alpha = std::get<float>(*property);
    }

    const Pose poseA = mNodeA->evaluate(context);
    const Pose poseB = mNodeB->evaluate(context);

    return Animator::blendPoses(poseA, poseB, alpha);
}