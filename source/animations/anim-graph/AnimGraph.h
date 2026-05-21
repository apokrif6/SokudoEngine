#pragma once

#include "AnimGraphLink.h"
#include "nodes/AnimGraphNode.h"
#include <memory>
#include "uuid.h"

namespace Core::Animations
{
class AnimGraph
{
public:
    using NodeID = uuids::uuid;

    template <typename T, typename... Args> std::shared_ptr<T> createNode(Args&&... args)
    {
        auto node = std::make_shared<T>(std::forward<Args>(args)...);

        mNodes[node->getUUID()] = node;

        return node;
    }

    void removeNode(const NodeID& id) { mNodes.erase(id); }

    [[nodiscard]] AnimGraphNode* getNode(const NodeID& id);

    [[nodiscard]] const AnimGraphNode* getNode(const NodeID& id) const;

    [[nodiscard]] const std::unordered_map<NodeID, std::shared_ptr<AnimGraphNode>>& getNodes() const { return mNodes; }

    void addLink(const AnimGraphLink& link) { mLinks.push_back(link); }

    [[nodiscard]] const std::vector<AnimGraphLink>& getLinks() const { return mLinks; }

    [[nodiscard]] const AnimGraphLink* findLinkByInputPin(PinID pin) const;

    [[nodiscard]] const AnimGraphNode* findNodeByPin(PinID pin) const;

    void setOutputNode(const NodeID& id) { mOutputNode = id; }

    [[nodiscard]] const NodeID& getOutputNode() const { return mOutputNode; }

    Pose evaluate(AnimationContext& context) const;

private:
    NodeID mOutputNode;

    std::unordered_map<NodeID, std::shared_ptr<AnimGraphNode>> mNodes;

    std::vector<AnimGraphLink> mLinks;
};

} // namespace Core::Animations