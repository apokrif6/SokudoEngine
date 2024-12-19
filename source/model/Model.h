#pragma once

#include "vk-renderer/VkRenderData.h"

class Model
{
  public:
    void init();

    VkMesh getVertexData();

  private:
    VkMesh mVertexData;
};
