#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include "core/vk-renderer/VkRenderData.h"

#define MAX_DEBUG_BONES_LINES 200

namespace Core::Renderer::Debug
{
struct DebugBone
{
    glm::vec3 start;
    glm::vec3 end;
};

class Skeleton
{
public:
    void init(Core::Renderer::VkRenderData& renderData);
    void update(Core::Renderer::VkRenderData& renderData, const std::vector<DebugBone>& bones);
    void draw(Core::Renderer::VkRenderData& renderData);
    void cleanup(Core::Renderer::VkRenderData& renderData);

private:
    void createVertexBuffer(Renderer::VkRenderData& renderData);

    Core::Renderer::VkVertexBufferData mDebugLinesBuffer;

    uint32_t mDebugLinesCount = 0;
};
} // namespace Core::Renderer::Debug
