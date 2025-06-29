#include "Skeleton.h"


void Core::Animations::Skeleton::initDebug(Core::Renderer::VkRenderData& renderData)
{
    if (!debugDraw)
    {
        debugDraw = std::make_unique<Core::Renderer::Debug::Skeleton>();
        debugDraw->init(renderData);
    }
}

void Core::Animations::Skeleton::updateDebug(Core::Renderer::VkRenderData& renderData,
                 const std::vector<Renderer::Debug::DebugBone>& bones) const
{
    if (debugDraw)
    {
        debugDraw->update(renderData, bones);
    }
}

void Core::Animations::Skeleton::drawDebug(Core::Renderer::VkRenderData& renderData) const
{
    if (debugDraw)
    {
        debugDraw->draw(renderData);
    }
}
