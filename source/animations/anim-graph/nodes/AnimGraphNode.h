#pragma once

namespace Core::Animations
{
struct Pose;
struct AnimationContext;

class AnimGraphNode
{
public:
    virtual ~AnimGraphNode() = default;

    virtual Pose evaluate(AnimationContext& context) = 0;
};

} // namespace Core::Animations