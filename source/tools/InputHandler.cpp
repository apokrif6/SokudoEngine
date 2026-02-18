#include "InputHandler.h"

#include "events/input-events/KeyEvent.h"
#include "tools/Logger.h"
#include "events/input-events/MouseMovementEvent.h"
#include "events/input-events/MouseLockEvent.h"
#include <imgui.h>
#include <string>
#include <GLFW/glfw3.h>

#include "engine/Engine.h"

void InputHandler::subscribeToEvents(EventListener* listener) { mEventDispatcher.subscribe(listener); }

void InputHandler::handleKeyEvents(int key, int scancode, int action, int mods)
{
    std::string actionName;
    switch (action)
    {
    case GLFW_PRESS:
        actionName = "pressed";
        break;
    case GLFW_RELEASE:
        actionName = "released";
        break;
    case GLFW_REPEAT:
        actionName = "repeated";
        break;
    default:
        actionName = "invalid";
        break;
    }

    mEventDispatcher.dispatch(KeyEvent(key, scancode, action, mods));

    const char* keyName = glfwGetKeyName(key, 0);
    Logger::log(1, "%s: key %s (key %i, scancode %i) %s\n", __FUNCTION__, keyName, key, scancode, actionName.c_str());
}

void InputHandler::handleMouseButtonEvents(int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (button >= 0 && button < ImGuiMouseButton_COUNT)
    {
        io.AddMouseButtonEvent(button, action == GLFW_PRESS);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            if (Core::Engine::getInstance().getRenderData().rdViewportHovered)
            {
                mMouseLock = true;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            mMouseLock = false;
        }

        mEventDispatcher.dispatch(MouseLockEvent(mMouseLock));
    }

    std::string actionName;
    switch (action)
    {
    case GLFW_PRESS:
        actionName = "pressed";
        break;
    case GLFW_RELEASE:
        actionName = "released";
        break;
    case GLFW_REPEAT:
        actionName = "repeated";
        break;
    default:
        actionName = "invalid";
        break;
    }

    std::string mouseButtonName;
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        mouseButtonName = "left";
        break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
        mouseButtonName = "middle";
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        mouseButtonName = "right";
        break;
    default:
        mouseButtonName = "other";
        break;
    }

    Logger::log(1, "%s: %s mouse button (%i) %s\n", __FUNCTION__, mouseButtonName.c_str(), button, actionName.c_str());
}

void InputHandler::handleMousePositionEvents(double xPosition, double yPosition)
{
    const int deltaX = static_cast<int>(xPosition) - mMouseXPosition;
    const int deltaY = static_cast<int>(yPosition) - mMouseYPosition;

    if (mMouseLock)
    {
        mEventDispatcher.dispatch(MouseMovementEvent(deltaX, deltaY));
    }

    mMouseXPosition = static_cast<int>(xPosition);
    mMouseYPosition = static_cast<int>(yPosition);

    // TODO
    // should be replaced with log categories
#if SE_LOG_INPUT
    Logger::log(1, "%s: Mouse cursor has been moved to %lf/%lf\n", __FUNCTION__, xPosition, yPosition);
#endif
}