#pragma once

#include "VkRenderData.h"
#include "future"

namespace Core::Renderer
{
class Cubemap
{
public:
    static bool loadHDRTexture(VkRenderData& renderData, VkHDRTextureData& texture, const std::string& path);

    static bool convertHDRToCubemap(VkRenderData& renderData, VkHDRTextureData& hdrTexture, VkCubemapData& cubemapData);

    static void cleanup(VkRenderData& renderData, VkCubemapData& cubemapData);
};
} // namespace Core::Renderer
