#pragma once

#include "VkRenderData.h"

class ArrowModel
{
  public:
    VkMesh getVertexData();

  private:
    void init();
    VkMesh mVertexData;
};