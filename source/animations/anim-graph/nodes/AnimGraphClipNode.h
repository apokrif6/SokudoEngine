
#pragma once

#include "AnimGraphNode.h"

namespace Core::Animations
{
struct AnimationClip;

class AnimGraphClipNode final : public AnimGraphNode
{
public:
    AnimGraphClipNode();

    Pose evaluate(AnimationContext& context) const override;

    [[nodiscard]] PinID getOutputPin() const { return mOutputPin; }

private:
    PinID mOutputPin{};
};

} // namespace Core::Animations