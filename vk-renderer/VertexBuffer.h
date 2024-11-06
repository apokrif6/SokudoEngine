#pragma once

#include "VkRenderData.h"

class VertexBuffer
{
  public:
    static bool init(VkRenderData& renderData);

    static bool uploadData(VkRenderData& renderData, VkMesh vertexData);

    static void cleanup(VkRenderData& renderData);
};
