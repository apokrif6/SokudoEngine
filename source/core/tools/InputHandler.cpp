#include "InputHandler.h"
#include "core/tools/Logger.h"
#include "core/events/input-events/MouseMovementEvent.h"
#include "core/events/input-events/MouseLockEvent.h"
#include <imgui.h>
#include <string>
#include <GLFW/glfw3.h>

InputHandler::InputHandler(GLFWwindow* inWindow) { mWindow = inWindow; }

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

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        mMouseLock = !mMouseLock;

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
    ImGuiIO& io = ImGui::GetIO();
    io.AddMousePosEvent(static_cast<float>(xPosition), static_cast<float>(yPosition));

    if (io.WantCaptureMouse)
    {
        return;
    }

    const int deltaX = static_cast<int>(xPosition) - mMouseXPosition;
    const int deltaY = static_cast<int>(yPosition) - mMouseYPosition;

    if (mMouseLock)
    {
        mEventDispatcher.dispatch(MouseMovementEvent(deltaX, deltaY));
    }

    mMouseXPosition = static_cast<int>(xPosition);
    mMouseYPosition = static_cast<int>(yPosition);

    Logger::log(1, "%s: Mouse cursor has been moved to %lf/%lf\n", __FUNCTION__, xPosition, yPosition);
}