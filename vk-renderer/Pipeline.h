#pragma once

#include "VkRenderData.h"

class Pipeline
{
  public:
    static bool init(VkRenderData& renderData, std::string vertexShaderFilename, std::string fragmentShaderFilename);
    static void cleanup(VkRenderData& renderData);
};
