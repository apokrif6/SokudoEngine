#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class IBLGenerator
{
public:
    static bool init(VkRenderData& renderData);

    static bool generateIBL(VkRenderData& renderData);

    static void cleanup(VkRenderData& renderData, IBLData& iblData);

private:
    static bool createDescriptorForHDR(VkRenderData& renderData);

    static bool convertHDRToCubemap(VkRenderData& renderData, VkTextureData& texture, VkCubemapData& cubemapData);

    static bool convertCubemapToIrradiance(VkRenderData& renderData, VkCubemapData& cubemapData,
                                           VkCubemapData& irradianceData);

    static bool convertCubemapToPrefilteredMap(VkRenderData& renderData, VkCubemapData& cubemapData,
                                               VkCubemapData& prefilteredMapData);

    static bool generateBRDFLUT(VkRenderData& renderData, VkTextureData& brdfLutData);

    static bool createHDRToCubemapPipeline(VkRenderData& renderData);

    static bool createIrradiancePipeline(VkRenderData& renderData);

    static bool createPrefilterPipeline(VkRenderData& renderData);

    static bool createBRDFLUTPipeline(VkRenderData& renderData);

    static void cleanupCubemapResources(VkRenderData& renderData, VkCubemapData& cubemapData);
};
} // namespace Core::Renderer
