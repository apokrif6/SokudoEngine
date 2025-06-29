#pragma once

#include "AnimationsData.h"

namespace Core::Animations
{
class Skeleton
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

  private:
    BoneNode rootNode;

    std::unique_ptr<Core::Renderer::Debug::Skeleton> debugDraw;
};
} // namespace Core::Animations