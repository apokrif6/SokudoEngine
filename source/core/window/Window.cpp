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

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return true;
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
