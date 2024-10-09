#include "Window.h"
#include "Logger.h"

bool Window::init(unsigned int width, unsigned int height, std::string title)
{
    if (!glfwInit())
    {
        Logger::log(1, "%s: glfwInit error\n", __FUNCTION__);
        return false;
    }

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    CreatedWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!CreatedWindow)
    {
        Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
        glfwTerminate();
        return false;
    }

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return true;
}

void Window::mainLoop()
{
    while (!glfwWindowShouldClose(CreatedWindow))
    {
        glfwPollEvents();
    }
}

void Window::cleanup()
{
    Logger::log(1, "%s: Terminating window\n", __FUNCTION__);
    glfwDestroyWindow(CreatedWindow);
    glfwTerminate();
}