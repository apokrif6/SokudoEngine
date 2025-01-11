#pragma once

#include <vulkan/vulkan.h>

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class ShaderStorageBuffer
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkShaderStorageBufferData &SSBOData,
                     size_t bufferSize);

    static void uploadData(Core::Renderer::VkRenderData& renderData, VkShaderStorageBufferData &SSBOData,
                           std::vector<glm::mat4> matrices);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkShaderStorageBufferData &SSBOData);
};
}