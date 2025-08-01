#include <vector>
#include "Window.h"
#include "core/tools/Logger.h"

std::unique_ptr<Core::Renderer::VkRenderer> Core::Application::Window::init(int width, int height, const std::string& title)
{
    if (!glfwInit())
    {
        Logger::log(1, "%s: glfwInit error\n", __FUNCTION__);
        return nullptr;
    }

    if (!glfwVulkanSupported())
    {
        Logger::log(1, "%s: Vulkan is not supported\n", __FUNCTION__);
        glfwTerminate();
        return nullptr;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    mWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!mWindow)
    {
        Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
        glfwTerminate();
        return nullptr;
    }

    std::unique_ptr<Core::Renderer::VkRenderer> renderer = std::make_unique<Core::Renderer::VkRenderer>(mWindow);
    if (!renderer->init(width, height))
    {
        glfwTerminate();
        Logger::log(1, "%s error: Could not init Vulkan renderer\n", __FUNCTION__);
        return nullptr;
    }

    mInputHandler = std::make_unique<InputHandler>(mWindow);
    if (!mInputHandler)
    {
        glfwTerminate();
        Logger::log(1, "%s error: Could not init Input Handler\n", __FUNCTION__);
        return nullptr;
    }

    renderer->subscribeToInputEvents(mInputHandler->getDispatcher());

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowPosCallback(mWindow,
                             [](GLFWwindow* window, int xPosition, int yPosition)
                             {
                                 Core::Engine::getInstance().getSystem<Core::Renderer::VkRenderer>()->handleWindowMoveEvents(xPosition, yPosition);
                             });

    glfwSetWindowIconifyCallback(mWindow,
                                 [](GLFWwindow* window, int minimized)
                                 {
                                     Core::Engine::getInstance().getSystem<Core::Renderer::VkRenderer>()->handleWindowMinimizedEvents(minimized);
                                 });

    glfwSetWindowMaximizeCallback(mWindow,
                                  [](GLFWwindow* window, int maximized)
                                  {
                                      Core::Engine::getInstance().getSystem<Core::Renderer::VkRenderer>()->handleWindowMaximizedEvents(maximized);
                                  });

    glfwSetWindowSizeCallback(mWindow,
                              [](GLFWwindow* window, int width, int height)
                              {
                                  Core::Engine::getInstance().getSystem<Core::Renderer::VkRenderer>()->setSize(width, height);
                              });

    glfwSetWindowCloseCallback(mWindow,
                               [](GLFWwindow* window)
                               {
                                   Core::Engine::getInstance().getSystem<Core::Renderer::VkRenderer>()->handleWindowCloseEvents();
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
                                   Core::Engine::getInstance().getSystem<Core::Renderer::VkRenderer>()->handleMouseEnterLeaveEvents(enter);
                               });

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return renderer;
}

void Core::Application::Window::mainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        Core::Engine::getInstance().update();
        Core::Engine::getInstance().draw();

        glfwPollEvents();
    }
}

void Core::Application::Window::cleanup()
{
    Core::Engine::getInstance().cleanup();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
    Logger::log(1, "%s: Terminating window\n", __FUNCTION__);
}
