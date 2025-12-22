#pragma once

#include <future>
#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

namespace Core::Renderer
{
class Texture
{
public:
    static std::future<bool> loadTexture(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData,
                                         const std::string& textureFilename, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData);
};
} // namespace Core::Renderer