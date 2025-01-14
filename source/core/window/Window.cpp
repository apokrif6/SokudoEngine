#include <vector>
#include "Window.h"
#include "core/tools/Logger.h"

bool Core::Application::Window::init(int width, int height, const std::string& title)
{
    if (!glfwInit())
    {
        Logger::log(1, "%s: glfwInit error\n", __FUNCTION__);
        return false;
    }

    if (!glfwVulkanSupported())
    {
        Logger::log(1, "%s: Vulkan is not supported\n", __FUNCTION__);
        glfwTerminate();
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

    mRenderer = std::make_unique<Core::Renderer::VkRenderer>(mWindow);
    if (!mRenderer->init(width, height))
    {
        glfwTerminate();
        Logger::log(1, "%s error: Could not init Vulkan mRenderer\n", __FUNCTION__);
        return false;
    }

    mInputHandler = std::make_unique<InputHandler>(mWindow);
    if (!mInputHandler)
    {
        glfwTerminate();
        Logger::log(1, "%s error: Could not init Input Handler\n", __FUNCTION__);
        return false;
    }

    mRenderer->subscribeToInputEvents(mInputHandler->getDispatcher());

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowPosCallback(mWindow,
                             [](GLFWwindow* window, int xPosition, int yPosition)
                             {
                                 const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                 CurrentWindow->mRenderer->handleWindowMoveEvents(xPosition, yPosition);
                             });

    glfwSetWindowIconifyCallback(mWindow,
                                 [](GLFWwindow* window, int minimized)
                                 {
                                     const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                     CurrentWindow->mRenderer->handleWindowMinimizedEvents(minimized);
                                 });

    glfwSetWindowMaximizeCallback(mWindow,
                                  [](GLFWwindow* window, int maximized)
                                  {
                                      const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                      CurrentWindow->mRenderer->handleWindowMaximizedEvents(maximized);
                                  });

    glfwSetWindowSizeCallback(mWindow,
                              [](GLFWwindow* window, int width, int height)
                              {
                                  const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                  CurrentWindow->mRenderer->setSize(width, height);
                              });

    glfwSetWindowCloseCallback(mWindow,
                               [](GLFWwindow* window)
                               {
                                   const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   CurrentWindow->mRenderer->handleWindowCloseEvents();
                               });

    glfwSetKeyCallback(mWindow,
                       [](GLFWwindow* window, int key, int scancode, int action, int mods)
                       {
                           const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                           CurrentWindow->mInputHandler->handleKeyEvents(key, scancode, action, mods);
                       });

    glfwSetMouseButtonCallback(mWindow,
                               [](GLFWwindow* window, int button, int action, int mods)
                               {
                                   const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   CurrentWindow->mInputHandler->handleMouseButtonEvents(button, action, mods);
                               });

    glfwSetCursorPosCallback(mWindow,
                             [](GLFWwindow* window, double xPosition, double yPosition)
                             {
                                 const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                 CurrentWindow->mInputHandler->handleMousePositionEvents(xPosition, yPosition);
                             });

    glfwSetCursorEnterCallback(mWindow,
                               [](GLFWwindow* window, int enter)
                               {
                                   const auto CurrentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   CurrentWindow->mRenderer->handleMouseEnterLeaveEvents(enter);
                               });

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return true;
}

void Core::Application::Window::mainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        if (!mRenderer->draw())
        {
            break;
        }

        glfwPollEvents();
    }
}

void Core::Application::Window::cleanup()
{
    mRenderer->cleanup();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
    Logger::log(1, "%s: Terminating window\n", __FUNCTION__);
}
