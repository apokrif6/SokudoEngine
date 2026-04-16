#pragma once

#include "AnimationsData.h"
#include "resources/Mesh.h"

namespace Core::Animations
{
class Skeleton
{
public:
    void setData(const Resources::SkeletonData* skeletonData) { mSkeletonData = skeletonData; }

    [[nodiscard]] const BoneNode& getRootNode() const { return mSkeletonData->rootNode; }

    void initDebug(Renderer::VkRenderData& renderData);

    void updateDebug(Renderer::VkRenderData& renderData, const std::vector<Renderer::Debug::DebugBone>& bones) const;

    void drawDebug(Renderer::VkRenderData& renderData) const;

    void cleanup(Renderer::VkRenderData& renderData);

private:
    const Resources::SkeletonData* mSkeletonData = nullptr;

    std::shared_ptr<Renderer::Debug::Skeleton> debugDraw;
};
} // namespace Core::Animations