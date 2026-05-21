#pragma once

#include "AnimGraphNode.h"
#include <memory>

namespace Core::Animations
{
class AnimGraphOutputPoseNode final : public AnimGraphNode
{
public:
    explicit AnimGraphOutputPoseNode();

    Pose evaluate(AnimationContext& context) const override;

    [[nodiscard]] PinID getInputPin() const { return mInputPin; }

private:
    PinID mInputPin{};
};
} // namespace Core::Animations