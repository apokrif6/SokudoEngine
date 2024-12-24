#pragma once

#include "VkRenderData.h"
#include "tiny_gltf.h"

namespace Core::Renderer
{
class VertexBuffer
{
  public:
    static bool init(Core::Renderer::VkRenderData renderData, VkVertexBufferData& VkVertexBufferData, unsigned int bufferSize);

    static bool uploadData(Core::Renderer::VkRenderData renderData, VkVertexBufferData& vertexBufferData, Core::Renderer::VkMesh vertexData);

    static bool uploadData(Core::Renderer::VkRenderData renderData, VkVertexBufferData& vertexBufferData,
                           const tinygltf::Buffer& buffer, const tinygltf::BufferView& bufferView);

    static bool uploadData(Core::Renderer::VkRenderData renderData, VkVertexBufferData& vertexBufferData,
                           std::vector<glm::vec3> vertexData);

    static void cleanup(Core::Renderer::VkRenderData renderData, VkVertexBufferData& vertexBufferData);
};
}