#include "Skeleton.h"

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

YAML::Node Core::Animations::Skeleton::serialize() const
{
    YAML::Node node;
    node["rootNode"] = rootNode.serialize();
    return node;
}

void Core::Animations::Skeleton::deserialize(const YAML::Node& node) { rootNode.deserialize(node["rootNode"]); }
