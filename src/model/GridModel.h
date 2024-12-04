#pragma once

#include "vk-renderer/VkRenderData.h"

class GridModel
{
  public:
    VkMesh getVertexData();

  private:
    void init();
    VkMesh mVertexData;
};
