#pragma once

#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "Model.h"
#include "vk-renderer/VkRenderer.h"
#include "tools/InputHandler.h"

class Window
{
  public:
    bool init(int width, int height, const std::string& title);

    void mainLoop();

    void cleanup();

  private:
    GLFWwindow* mWindow = nullptr;

    std::unique_ptr<VkRenderer> mRenderer;

    std::unique_ptr<InputHandler> mInputHandler;
};
