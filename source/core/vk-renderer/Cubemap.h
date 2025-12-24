#pragma once

#include "VkRenderData.h"
#include "future"

namespace Core::Renderer
{
class Cubemap
{
public:
    static bool loadHDRTexture(VkRenderData& renderData, VkTextureData& texture, const std::string& path);

    static bool convertHDRToCubemap(VkRenderData& renderData, VkTextureData& texture, VkCubemapData& cubemapData);

    static bool convertCubemapToIrradiance(VkRenderData& renderData, VkCubemapData& cubemapData, VkCubemapData& irradianceData);

    static bool convertCubemapToPrefilteredMap(VkRenderData& renderData, VkCubemapData& cubemapData, VkCubemapData& prefilteredMapData);

    static bool generateBRDFLUT(VkRenderData& renderData, VkTextureData& brdfLutData);

    static void cleanup(VkRenderData& renderData, VkCubemapData& cubemapData);
};
} // namespace Core::Renderer
