#pragma once

#include "VkRenderData.h"

class CoordinateArrowModel
{
  public:
    VkMesh getVertexData();

  private:
    void init();
    
    VkMesh mVertexData;
};
