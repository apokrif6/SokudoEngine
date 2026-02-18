#pragma once

#include <vulkan/vulkan.h>

#include "vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class ShaderStorageBuffer
{
public:
    static bool init(VkRenderData& renderData, VkShaderStorageBufferData& SSBOData, size_t bufferSize,
                     const std::string& name);

    static void uploadData(VkRenderData& renderData, VkShaderStorageBufferData& SSBOData,
                           std::vector<glm::mat4> matrices);

    static void cleanup(VkRenderData& renderData, VkShaderStorageBufferData& SSBOData);
};
} // namespace Core::Renderer