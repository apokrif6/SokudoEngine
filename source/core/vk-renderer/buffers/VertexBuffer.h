#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class VertexBuffer
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& VkVertexBufferData,
                     unsigned int bufferSize);

    static bool uploadData(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData,
                           Core::Renderer::VkMesh vertexData);

    static bool uploadData(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData,
                           std::vector<glm::vec3> vertexData);

    static bool uploadData(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData,
                           std::vector<Core::Renderer::NewVertex> vertexData);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData);
};
} // namespace Core::Renderer