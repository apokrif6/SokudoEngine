#include "Window.h"
#include "stb_image.h"
#include "tools/Logger.h"
#include "ui/UserInterface.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

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

#ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(mWindow);
    HICON hIcon = static_cast<HICON>(LoadImage(GetModuleHandle(NULL), "MAINICON", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE));

    if (hIcon)
    {
        SendMessage(hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
    }
#endif

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return true;
}

void Core::Application::Window::mainLoop()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        Engine::getInstance().update();
        Engine::getInstance().draw();

        glfwPollEvents();
    }
}

void Core::Application::Window::cleanup()
{
    Engine::getInstance().cleanup();
    glfwDestroyWindow(mWindow);
    glfwTerminate();
    Logger::log(1, "%s: Terminating window\n", __FUNCTION__);
}

void Core::Application::Window::bindInputs()
{
    mInputHandler = std::make_unique<InputHandler>();

    if (!mInputHandler)
    {
        glfwTerminate();

        Logger::log(1, "%s error: Could not init Input Handler\n", __FUNCTION__);

        return;
    }

    Engine::getInstance().getSystem<Renderer::VkRenderer>()->subscribeToEventDispatcher(mInputHandler->getDispatcher());
    Engine::getInstance().getSystem<Renderer::UserInterface>()->subscribeToEventDispatcher(
        mInputHandler->getDispatcher());

    glfwSetWindowUserPointer(mWindow, this);

    glfwSetWindowPosCallback(
        mWindow, [](GLFWwindow* window, int xPosition, int yPosition)
        { Engine::getInstance().getSystem<Renderer::VkRenderer>()->handleWindowMoveEvents(xPosition, yPosition); });

    glfwSetWindowIconifyCallback(
        mWindow, [](GLFWwindow* window, int minimized)
        { Engine::getInstance().getSystem<Renderer::VkRenderer>()->handleWindowMinimizedEvents(minimized); });

    glfwSetWindowMaximizeCallback(
        mWindow, [](GLFWwindow* window, int maximized)
        { Engine::getInstance().getSystem<Renderer::VkRenderer>()->handleWindowMaximizedEvents(maximized); });

    glfwSetFramebufferSizeCallback(
        mWindow, [](GLFWwindow* window, int width, int height)
        { Engine::getInstance().getSystem<Renderer::VkRenderer>()->handleWindowResizeEvents(width, height); });

    glfwSetWindowCloseCallback(mWindow, [](GLFWwindow* window)
                               { Engine::getInstance().getSystem<Renderer::VkRenderer>()->handleWindowCloseEvents(); });

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

    glfwSetCursorEnterCallback(
        mWindow, [](GLFWwindow* window, int enter)
        { Engine::getInstance().getSystem<Renderer::VkRenderer>()->handleMouseEnterLeaveEvents(enter); });
}
