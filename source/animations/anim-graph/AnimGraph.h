#pragma once

#include "nodes/AnimGraphNode.h"
#include <memory>

namespace Core::Animations
{

class AnimGraph
{
public:
    void setRoot(std::shared_ptr<AnimGraphNode> root) { mRoot = std::move(root); }

    Pose evaluate(AnimationContext& context) const;

private:
    std::shared_ptr<AnimGraphNode> mRoot;
};

} // namespace Core::Animations