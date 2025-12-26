#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Model
{
class ArrowModel
{
public:
    Renderer::VkMesh getVertexData();

private:
    void init();
    Renderer::VkMesh mVertexData;
};
} // namespace Core::Model