#include "Skeleton.h"
#include "vk-renderer/debug/Skeleton.h"

int Core::Animations::Skeleton::getBoneIndex(const std::string& boneName)
{
    if (!mSkeletonData)
    {
        return -1;
    }

    const auto& boneMap = mSkeletonData->boneNameToIndexMap;

    if (const auto it = boneMap.find(boneName); it != boneMap.end())
    {
        return it->second;
    }

    return -1;
}

void Core::Animations::Skeleton::initDebug(Renderer::VkRenderData& renderData)
{
    if (!debugDraw)
    {
        debugDraw = std::make_unique<Renderer::Debug::Skeleton>();
        debugDraw->init(renderData);
    }
}

void Core::Animations::Skeleton::updateDebug(Renderer::VkRenderData& renderData,
                                             const std::vector<Renderer::Debug::DebugBone>& bones) const
{
    if (debugDraw)
    {
        debugDraw->update(renderData, bones);
    }
}

void Core::Animations::Skeleton::drawDebug(Renderer::VkRenderData& renderData) const
{
    if (debugDraw)
    {
        debugDraw->draw(renderData);
    }
}

void Core::Animations::Skeleton::cleanup(Renderer::VkRenderData& renderData)
{
    if (debugDraw)
    {
        debugDraw->cleanup(renderData);
        debugDraw.reset();
    }
}
