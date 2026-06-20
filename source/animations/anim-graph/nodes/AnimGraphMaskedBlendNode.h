#pragma once

#include "AnimGraphNode.h"

namespace Core::Animations
{
class AnimGraphMaskedBlendNode final : public AnimGraphNode
{
public:
    AnimGraphMaskedBlendNode();

    [[nodiscard]] PinID getInputAPin() const { return mInputA; }

    [[nodiscard]] PinID getInputBPin() const { return mInputB; }

    [[nodiscard]] PinID getOutputPin() const { return mOutputPin; }

    Pose evaluate(AnimationContext& context) const override;

private:
    PinID mInputA{};
    PinID mInputB{};

    PinID mOutputPin{};
};

} // namespace Core::Animations