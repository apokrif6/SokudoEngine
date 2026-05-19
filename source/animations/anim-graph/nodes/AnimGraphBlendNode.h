#pragma once

#include "AnimGraphNode.h"
#include <memory>

namespace Core::Animations
{

class AnimGraphBlendNode final : public AnimGraphNode
{
public:
    void setA(std::shared_ptr<AnimGraphNode> nodeA) { mNodeA = std::move(nodeA); }

    void setB(std::shared_ptr<AnimGraphNode> nodeB) { mNodeB = std::move(nodeB); }

    Pose evaluate(AnimationContext& context) const override;

private:
    std::shared_ptr<AnimGraphNode> mNodeA;
    std::shared_ptr<AnimGraphNode> mNodeB;
};

} // namespace Core::Animations