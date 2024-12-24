#pragma once

#include "core/vk-renderer/VkRenderData.h"

class Model
{
  public:
    void init();

    Core::Renderer::VkMesh getVertexData();

  private:
    Core::Renderer::VkMesh mVertexData;
};
