#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class ViewportTarget
{
public:
    bool init(VkRenderData& renderData, glm::int2 size);
    void cleanup(VkRenderData& renderData);

private:
    bool createImage(VkRenderData& renderData);
    bool createImageView(VkRenderData& renderData);
    bool createFramebuffer(VkRenderData& renderData);
    bool createDescriptor(VkRenderData& renderData);

    VkSampler mSampler = VK_NULL_HANDLE;
};
} // namespace Core::Renderer