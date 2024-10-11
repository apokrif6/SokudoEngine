#pragma once

#include "VkRenderer.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

class Window
{
  public:
    bool init(int width, int height, const std::string& title);

    bool initVulkan();

    void mainLoop();

    void cleanup();

  private:
    GLFWwindow* CreatedWindow = nullptr;

    std::unique_ptr<VkRenderer> Renderer;

    std::string ApplicationName;

    VkInstance Instance{};

    VkSurfaceKHR Surface{};

    void handleWindowMoveEvents(int xPosition, int yPosition);

    void handleWindowMinimizedEvents(int minimized);

    void handleWindowMaximizedEvents(int maximized);

    void handleWindowResizedEvents(int width, int height);

    void handleWindowCloseEvents();

    void handleKeyEvents(int key, int scancode, int action, int mods);

    void handleMouseButtonEvents(int button, int action, int mods);

    void handleMousePositionEvents(double xPosition, double yPosition);

    void handleMouseEnterLeaveEvents(int enter);
};
