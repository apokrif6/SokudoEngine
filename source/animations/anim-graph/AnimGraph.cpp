#include "AnimGraph.h"

#include "AnimGraphLink.h"
#include "AnimationContext.h"
#include "animations/AnimationsData.h"
#include "nodes/AnimGraphOutputPoseNode.h"

Core::Animations::AnimGraphNode* Core::Animations::AnimGraph::getNode(const NodeID& id)
{
    if (const auto it = mNodes.find(id); it != mNodes.end())
    {
        return it->second.get();
    }

    return nullptr;
}

const Core::Animations::AnimGraphNode* Core::Animations::AnimGraph::getNode(const NodeID& id) const
{
    if (const auto it = mNodes.find(id); it != mNodes.end())
    {
        return it->second.get();
    }

    return nullptr;
}

const Core::Animations::AnimGraphLink* Core::Animations::AnimGraph::findLinkByInputPin(const PinID pin) const
{
    for (const auto& link : mLinks)
    {
        if (link.endPin == pin)
        {
            return &link;
        }
    }

    return nullptr;
}

const Core::Animations::AnimGraphNode* Core::Animations::AnimGraph::findNodeByPin(const PinID pin) const
{
    for (const auto& [id, node] : mNodes)
    {
        for (const auto& output : node->getOutputs())
        {
            if (output.id == pin)
            {
                return node.get();
            }
        }
    }

    return nullptr;
}

void Core::Animations::AnimGraph::removeLinksByPin(PinID pinId)
{
    std::erase_if(mLinks,
                  [pinId](const AnimGraphLink& link) { return link.startPin == pinId || link.endPin == pinId; });
}

Core::Animations::Pose Core::Animations::AnimGraph::evaluate(AnimationContext& context) const
{
    const auto* output = dynamic_cast<const AnimGraphOutputPoseNode*>(getNode(mOutputNode));

    if (!output)
    {
        return {};
    }

    context.graph = this;

    return output->evaluate(context);
}