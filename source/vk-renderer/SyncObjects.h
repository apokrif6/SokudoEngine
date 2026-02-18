#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class SyncObjects
{
public:
    static bool init(VkRenderData& renderData);

    static void cleanup(VkRenderData& renderData);
};
} // namespace Core::Renderer