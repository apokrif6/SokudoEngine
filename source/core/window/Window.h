#pragma once

#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "Model.h"
#include "core/vk-renderer/VkRenderer.h"
#include "core/tools/InputHandler.h"

namespace Core::Application
{
class Window
{
  public:
    bool init(int width, int height, const std::string& title);

    void mainLoop();

    void cleanup();

  private:
    GLFWwindow* mWindow = nullptr;

    std::unique_ptr<Core::Renderer::VkRenderer> mRenderer;

    std::unique_ptr<InputHandler> mInputHandler;
};
} // namespace Core::Application