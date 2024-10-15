#pragma once

#include "VkRenderer.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "Model.h"

class Window
{
  public:
    bool init(int width, int height, const std::string& title);

    void mainLoop();

    void cleanup();

  private:
    GLFWwindow* mWindow = nullptr;

    std::unique_ptr<VkRenderer> mRenderer;

    std::unique_ptr<Model> mModel;
};
