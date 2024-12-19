#pragma once

#include "vk-renderer/VkRenderData.h"

class ArrowModel
{
  public:
    VkMesh getVertexData();

  private:
    void init();
    VkMesh mVertexData;
};