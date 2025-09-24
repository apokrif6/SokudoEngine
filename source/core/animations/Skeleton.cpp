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

void Core::Animations::Skeleton::cleanup(Core::Renderer::VkRenderData &renderData)
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

void Core::Animations::Skeleton::deserialize(const YAML::Node& node)
{
    rootNode.deserialize(node["rootNode"]);
}
