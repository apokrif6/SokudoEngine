#include <vector>
#include "Window.h"
#include "Logger.h"

bool Window::init(int width, int height, const std::string& title)
{
    if (!glfwInit())
    {
        Logger::log(1, "%s: glfwInit error\n", __FUNCTION__);
        return false;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!mWindow)
    {
        Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
        glfwTerminate();
        return false;
    }

    mRenderer = std::make_unique<VkRenderer>(mWindow);
    if (!mRenderer->init(width, height))
    {
        glfwTerminate();
        Logger::log(1, "%s error: Could not init Vulkan mRenderer\n", __FUNCTION__);
        return false;
    }

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowPosCallback(mWindow,
                             [](GLFWwindow* window, int xPosition, int yPosition)
                             {
                                 auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                 CurrentRenderer->handleWindowMoveEvents(xPosition, yPosition);
                             });

    glfwSetWindowIconifyCallback(mWindow,
                                 [](GLFWwindow* window, int minimized)
                                 {
                                     auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                     CurrentRenderer->handleWindowMinimizedEvents(minimized);
                                 });

    glfwSetWindowMaximizeCallback(mWindow,
                                  [](GLFWwindow* window, int maximized)
                                  {
                                      auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                      CurrentRenderer->handleWindowMaximizedEvents(maximized);
                                  });

    glfwSetWindowSizeCallback(mWindow,
                              [](GLFWwindow* window, int width, int height)
                              {
                                  auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                                  CurrentRenderer->setSize(width, height);
                              });

    glfwSetWindowCloseCallback(mWindow,
                               [](GLFWwindow* window)
                               {
                                   auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   CurrentRenderer->handleWindowCloseEvents();
                               });

    glfwSetKeyCallback(mWindow,
                       [](GLFWwindow* window, int key, int scancode, int action, int mode)
                       {
                           auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                           CurrentRenderer->handleKeyEvents(key, scancode, action, mode);
                       });

    glfwSetMouseButtonCallback(mWindow,
                               [](GLFWwindow* window, int button, int action, int mods)
                               {
                                   auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   CurrentRenderer->handleMouseButtonEvents(button, action, mods);
                               });

    glfwSetCursorPosCallback(mWindow,
                             [](GLFWwindow* window, double xPosition, double yPosition)
                             {
                                 auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                 CurrentRenderer->handleMousePositionEvents(xPosition, yPosition);
                             });

    glfwSetCursorEnterCallback(mWindow,
                               [](GLFWwindow* window, int enter)
                               {
                                   auto CurrentRenderer = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   CurrentRenderer->handleMouseEnterLeaveEvents(enter);
                               });

    if (!glfwVulkanSupported())
    {
        Logger::log(1, "%s: Vulkan is not supported\n", __FUNCTION__);
        glfwTerminate();
        return false;
    }

    mModel = std::make_unique<Model>();
    mModel->init();

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return true;
}

void Window::mainLoop()
{
    mRenderer->uploadData(mModel->getVertexData());

    while (!glfwWindowShouldClose(mWindow))
    {
        if (!mRenderer->draw())
        {
            break;
        }

        glfwPollEvents();
    }
}

void Window::cleanup()
{
    mRenderer->cleanup();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
    Logger::log(1, "%s: Terminating window\n", __FUNCTION__);
}

void Window::handleWindowMoveEvents(int xPosition, int yPosition)
{
    Logger::log(1, "%s: mWindow has been moved to %i/%i\n", __FUNCTION__, xPosition, yPosition);
}

void Window::handleWindowMinimizedEvents(int minimized)
{
    if (minimized)
    {
        Logger::log(1, "%s: mWindow has been minimized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: mWindow has been restored\n", __FUNCTION__);
    }
}

void Window::handleWindowMaximizedEvents(int maximized)
{
    if (maximized)
    {
        Logger::log(1, "%s: mWindow has been maximized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: mWindow has been restored\n", __FUNCTION__);
    }
}

void Window::handleWindowCloseEvents() { Logger::log(1, "%s: mWindow has been closed\n", __FUNCTION__); }

void Window::handleKeyEvents(int key, int scancode, int action, int mods)
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
void Window::handleMouseButtonEvents(int button, int action, int mods)
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

void Window::handleMousePositionEvents(double xPosition, double yPosition)
{
    Logger::log(1, "%s: Mouse cursor has been moved to %lf/%lf\n", __FUNCTION__, xPosition, yPosition);
}

void Window::handleMouseEnterLeaveEvents(int enter)
{
    if (enter)
    {
        Logger::log(1, "%s: Mouse entered window\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: Mouse left window\n", __FUNCTION__);
    }
}
