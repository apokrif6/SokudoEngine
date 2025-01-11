#pragma once

#include <vulkan/vulkan.h>
#include "tiny_gltf.h"

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class IndexBuffer
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkIndexBufferData& indexBufferData, unsigned int bufferSize);

    static bool uploadData(Core::Renderer::VkRenderData& renderData, VkIndexBufferData& indexBufferData, const tinygltf::Buffer& buffer,
                           const tinygltf::BufferView& bufferView);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkIndexBufferData& IndexBufferData);
};
}