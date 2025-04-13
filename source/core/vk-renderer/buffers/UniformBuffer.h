#pragma once

#include <vulkan/vulkan.h>

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class UniformBuffer
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkUniformBufferData& UBOData, size_t bufferSize);

    static void uploadData(Core::Renderer::VkRenderData& renderData, VkUniformBufferData& UBOData,
                           std::vector<glm::mat4> matrices);

    static void uploadData(Core::Renderer::VkRenderData& renderData, VkUniformBufferData& UBOData,
                           const Core::Renderer::MaterialInfo& materialInfo);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkUniformBufferData& UBOData);
};
} // namespace Core::Renderer