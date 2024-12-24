#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Model
{
class CoordinateArrowModel
{
public:
  Core::Renderer::VkMesh getVertexData();

private:
  void init();

  Core::Renderer::VkMesh mVertexData;
};
}