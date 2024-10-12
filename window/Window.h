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

    void handleWindowMoveEvents(int xPosition, int yPosition);

    void handleWindowMinimizedEvents(int minimized);

    void handleWindowMaximizedEvents(int maximized);

    void handleWindowCloseEvents();

    void handleKeyEvents(int key, int scancode, int action, int mods);

    void handleMouseButtonEvents(int button, int action, int mods);

    void handleMousePositionEvents(double xPosition, double yPosition);

    void handleMouseEnterLeaveEvents(int enter);
};
