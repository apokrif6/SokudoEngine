#pragma once

#include "nodes/AnimGraphNode.h"
#include <memory>
#include "uuid.h"

namespace Core::Animations
{
class AnimGraph
{
public:
    using NodeID = uuids::uuid;

    NodeID addNode(std::shared_ptr<AnimGraphNode> node)
    {
        const NodeID id = generateUUID();

        mNodes[id] = std::move(node);

        return id;
    }

    template <typename T, typename... Args> std::shared_ptr<T> createNode(Args&&... args)
    {
        auto node = std::make_shared<T>(std::forward<Args>(args)...);

        mNodes[node->getUUID()] = node;

        return node;
    }

    void removeNode(const NodeID& id) { mNodes.erase(id); }

    AnimGraphNode* getNode(const NodeID& id)
    {
        if (const auto it = mNodes.find(id); it != mNodes.end())
        {
            return it->second.get();
        }

        return nullptr;
    }

    [[nodiscard]] const AnimGraphNode* getNode(const NodeID& id) const
    {
        if (const auto it = mNodes.find(id); it != mNodes.end())
        {
            return it->second.get();
        }

        return nullptr;
    }

    [[nodiscard]] const std::unordered_map<NodeID, std::shared_ptr<AnimGraphNode>>& getNodes() const { return mNodes; }

    void setRoot(const NodeID& id) { mRoot = id; }

    [[nodiscard]] const NodeID& getRoot() const { return mRoot; }

    Pose evaluate(AnimationContext& context) const;

private:
    NodeID mRoot;

    std::unordered_map<NodeID, std::shared_ptr<AnimGraphNode>> mNodes;
};

} // namespace Core::Animations