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

    glfwSetWindowUserPointer(mWindow, mRenderer.get());

    glfwSetWindowPosCallback(mWindow,
                             [](GLFWwindow* window, int xPosition, int yPosition)
                             {
                                 auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                                 CurrentRenderer->handleWindowMoveEvents(xPosition, yPosition);
                             });

    glfwSetWindowIconifyCallback(mWindow,
                                 [](GLFWwindow* window, int minimized)
                                 {
                                     auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                                     CurrentRenderer->handleWindowMinimizedEvents(minimized);
                                 });

    glfwSetWindowMaximizeCallback(mWindow,
                                  [](GLFWwindow* window, int maximized)
                                  {
                                      auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
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
                                   auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                                   CurrentRenderer->handleWindowCloseEvents();
                               });

    glfwSetKeyCallback(mWindow,
                       [](GLFWwindow* window, int key, int scancode, int action, int mode)
                       {
                           auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                           CurrentRenderer->handleKeyEvents(key, scancode, action, mode);
                       });

    glfwSetMouseButtonCallback(mWindow,
                               [](GLFWwindow* window, int button, int action, int mods)
                               {
                                   auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                                   CurrentRenderer->handleMouseButtonEvents(button, action, mods);
                               });

    glfwSetCursorPosCallback(mWindow,
                             [](GLFWwindow* window, double xPosition, double yPosition)
                             {
                                 auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
                                 CurrentRenderer->handleMousePositionEvents(xPosition, yPosition);
                             });

    glfwSetCursorEnterCallback(mWindow,
                               [](GLFWwindow* window, int enter)
                               {
                                   auto CurrentRenderer = static_cast<VkRenderer*>(glfwGetWindowUserPointer(window));
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
