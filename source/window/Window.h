#pragma once

#include <vulkan/vulkan.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "core/vk-renderer/VkRenderer.h"
#include "core/tools/InputHandler.h"
#include "engine/Engine.h"

namespace Core::Application
{
class Window
{
public:
    bool init(int width, int height, const std::string& title);

    GLFWwindow* getGLFWwindow() const { return mWindow; }

    void mainLoop();

    void cleanup();

    void bindInputs();

private:
    GLFWwindow* mWindow = nullptr;

    std::unique_ptr<InputHandler> mInputHandler;
};
} // namespace Core::Application