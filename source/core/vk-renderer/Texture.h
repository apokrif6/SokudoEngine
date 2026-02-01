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
    static std::future<bool> loadTexture(VkRenderData& renderData, VkTextureData& textureData,
                                         const std::string& textureFilename, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

    static std::future<bool> loadHDRTexture(VkRenderData& renderData, VkTextureData& textureData,
                                            const std::string& textureFilename,
                                            VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT);

    static void cleanup(VkRenderData& renderData, VkTextureData& textureData);
};
} // namespace Core::Renderer