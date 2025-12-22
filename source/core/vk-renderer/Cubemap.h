#pragma once

#include "VkRenderData.h"
#include "future"

namespace Core::Renderer
{
class Cubemap
{
public:
    static std::future<bool> loadCubemap(VkRenderData& renderData, VkCubemapData& cubemapData,
                                         const std::vector<std::string>& faces);

    static void cleanup(VkRenderData& renderData, VkCubemapData& cubemapData);
};
} // namespace Core::Renderer
