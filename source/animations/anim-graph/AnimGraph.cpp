#include "AnimGraph.h"

#include "animations/AnimationsData.h"

Core::Animations::Pose Core::Animations::AnimGraph::evaluate(AnimationContext& context) const
{
    const AnimGraphNode* root = getNode(mRoot);

    if (root == nullptr)
    {
        return {};
    }

    return root->evaluate(context);
}