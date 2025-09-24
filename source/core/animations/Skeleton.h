#pragma once

#include "AnimationsData.h"
#include "core/serialization/Serializable.h"

namespace Core::Animations
{
class Skeleton : public Serialization::ISerializable
{
  public:
    void setRootNode(const BoneNode& node)
    {
        rootNode = node;
    }

    [[nodiscard]] const BoneNode& getRootNode() const { return rootNode; }

    void initDebug(Core::Renderer::VkRenderData& renderData);

    void updateDebug(Core::Renderer::VkRenderData& renderData,
                     const std::vector<Renderer::Debug::DebugBone>& bones) const;

    void drawDebug(Core::Renderer::VkRenderData& renderData) const;

    void cleanup(Core::Renderer::VkRenderData& renderData);

    YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

  private:
    BoneNode rootNode;

    std::shared_ptr<Core::Renderer::Debug::Skeleton> debugDraw;
};
} // namespace Core::Animations