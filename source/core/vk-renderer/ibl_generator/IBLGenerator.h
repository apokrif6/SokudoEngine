#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class IBLGenerator
{
public:
    static bool init(VkRenderData& renderData);

    static bool generateIBL(VkRenderData& renderData);

    static void cleanup(VkRenderData& renderData, VkCubemapData& cubemapData);

private:
    static bool createDescriptorForHDR(VkRenderData& renderData);

    static bool createStaticCubemapLayout(VkRenderData& renderData, VkDescriptorSetLayout& layout, const std::string_view& name);

    static bool convertHDRToCubemap(VkRenderData& renderData, VkTextureData& texture, VkCubemapData& cubemapData);

    static bool convertCubemapToIrradiance(VkRenderData& renderData, VkCubemapData& cubemapData, VkCubemapData& irradianceData);

    static bool convertCubemapToPrefilteredMap(VkRenderData& renderData, VkCubemapData& cubemapData, VkCubemapData& prefilteredMapData);

    static bool generateBRDFLUT(VkRenderData& renderData, VkTextureData& brdfLutData);

    static bool createHDRToCubemapPipeline(VkRenderData& renderData);

    static bool createIrradiancePipeline(VkRenderData& renderData);

    static bool createPrefilterPipeline(VkRenderData& renderData);

    static bool createBRDFLUTPipeline(VkRenderData& renderData);
};
} // namespace Core::Renderer
