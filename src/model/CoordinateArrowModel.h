#pragma once

#include "vk-renderer/VkRenderData.h"

class CoordinateArrowModel
{
  public:
    VkMesh getVertexData();

  private:
    void init();

    VkMesh mVertexData;
};
