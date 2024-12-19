#pragma once

#include <GLFW/glfw3.h>
#include "events/EventDispatcher.h"

class InputHandler
{
  public:
    explicit InputHandler(GLFWwindow* inWindow);

    void subscribeToEvents(EventListener* listener);

    EventDispatcher& getDispatcher() { return mEventDispatcher; }

    void handleKeyEvents(int key, int scancode, int action, int mods);

    void handleMouseButtonEvents(int button, int action, int mods);

    void handleMousePositionEvents(double xPosition, double yPosition);

  private:
    GLFWwindow* mWindow;

    EventDispatcher mEventDispatcher;

    bool mMouseLock = false;

    int mMouseXPosition = 0;
    int mMouseYPosition = 0;
};