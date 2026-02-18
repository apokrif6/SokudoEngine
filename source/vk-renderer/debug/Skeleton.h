#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include "vk-renderer/VkRenderData.h"

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
    void init(VkRenderData& renderData);
    void update(VkRenderData& renderData, const std::vector<DebugBone>& bones);
    void draw(VkRenderData& renderData);
    void cleanup(VkRenderData& renderData);

private:
    void createVertexBuffer(VkRenderData& renderData);

    VkVertexBufferData mDebugLinesBuffer;

    uint32_t mDebugLinesCount = 0;
};
} // namespace Core::Renderer::Debug
