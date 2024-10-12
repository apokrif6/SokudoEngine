#pragma once

#include "VkRenderData.h"

class Model
{
  public:
    void init();

    VkMesh getVertexData();

  private:
    VkMesh VertexData;
};
