
#pragma once

#include "AnimGraphNode.h"

namespace Core::Animations
{
struct AnimationClip;

class AnimGraphClipNode final : public AnimGraphNode
{
public:
    Pose evaluate(AnimationContext& context) const override;
};

} // namespace Core::Animations