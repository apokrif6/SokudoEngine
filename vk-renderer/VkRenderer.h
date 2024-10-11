#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class VkRenderer
{
  public:
    VkRenderer(GLFWwindow* inWindow);

    bool init(unsigned int width, unsigned int height);

    bool draw();

  private:
    GLFWwindow* Window = nullptr;
};
