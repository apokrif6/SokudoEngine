#pragma once

#include <GLFW/glfw3.h>

class InputHandler
{
  public:
    explicit InputHandler(GLFWwindow* inWindow);

    void handleKeyEvents(int key, int scancode, int action, int mods);

    void handleMouseButtonEvents(int button, int action, int mods);

  private:
    // TODO
    // how to remove this dependency?
    GLFWwindow* mWindow;

    bool mMouseLock = false;
};