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

    void setAlpha(const float alpha) { mAlpha = alpha; }

    Pose evaluate(AnimationContext& context) override;

private:
    std::shared_ptr<AnimGraphNode> mNodeA;
    std::shared_ptr<AnimGraphNode> mNodeB;

    float mAlpha = 0.f;
};

} // namespace Core::Animations