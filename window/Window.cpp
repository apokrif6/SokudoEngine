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

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    ApplicationName = title;

    CreatedWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!CreatedWindow)
    {
        Logger::log(1, "%s: Could not create window\n", __FUNCTION__);
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(CreatedWindow, this);

    glfwSetWindowPosCallback(CreatedWindow,
                             [](GLFWwindow* win, int xPosition, int yPosition)
                             {
                                 auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
                                 thisWindow->handleWindowMoveEvents(xPosition, yPosition);
                             });

    glfwSetWindowIconifyCallback(CreatedWindow,
                                 [](GLFWwindow* win, int minimized)
                                 {
                                     auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
                                     thisWindow->handleWindowMinimizedEvents(minimized);
                                 });

    glfwSetWindowMaximizeCallback(CreatedWindow,
                                  [](GLFWwindow* win, int maximized)
                                  {
                                      auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
                                      thisWindow->handleWindowMaximizedEvents(maximized);
                                  });

    glfwSetWindowCloseCallback(CreatedWindow,
                               [](GLFWwindow* win)
                               {
                                   auto thisWindow = static_cast<Window*>(glfwGetWindowUserPointer(win));
                                   thisWindow->handleWindowCloseEvents();
                               });

    if (!glfwVulkanSupported())
    {
        Logger::log(1, "%s: Vulkan is not supported\n", __FUNCTION__);
        glfwTerminate();
        return false;
    }

    if (!initVulkan())
    {
        Logger::log(1, "%s: Could not init Vulkan\n", __FUNCTION__);
        glfwTerminate();
        return false;
    }

    Logger::log(1, "%s: window successfully initialized\n", __FUNCTION__);
    return true;
}

bool Window::initVulkan()
{
    VkApplicationInfo ApplicationInfo{};
    ApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInfo.pNext = nullptr;
    ApplicationInfo.pApplicationName = ApplicationName.c_str();
    ApplicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 0, 1);
    ApplicationInfo.pEngineName = "Game Animations Programming";
    ApplicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    ApplicationInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 1, 0);

    uint32_t ExtensionCount = 0;
    const char** Extensions = glfwGetRequiredInstanceExtensions(&ExtensionCount);

    if (ExtensionCount == 0)
    {
        Logger::log(1, "%s error: No Vulkan extensions\n", __FUNCTION__);
        return false;
    }

    Logger::log(1, "%s: Found %u Vulkan extensions\n", __FUNCTION__, ExtensionCount);
    for (int i = 0; i < ExtensionCount; ++i)
    {
        Logger::log(1, "%s: %s\n", __FUNCTION__, std::string(Extensions[i]).c_str());
    }

    VkInstanceCreateInfo CreateInfo{};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pNext = nullptr;
    CreateInfo.pApplicationInfo = &ApplicationInfo;
    CreateInfo.enabledExtensionCount = ExtensionCount;
    CreateInfo.ppEnabledExtensionNames = Extensions;
    CreateInfo.enabledLayerCount = 0;

    VkResult Result = vkCreateInstance(&CreateInfo, nullptr, &Instance);
    if (Result != VK_SUCCESS)
    {
        Logger::log(1, "%s: Could not create Vulkan Instance\n", __FUNCTION__);
        return false;
    }

    uint32_t PhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, nullptr);

    if (PhysicalDeviceCount == 0)
    {
        Logger::log(1, "%s: No Vulkan capable GPU found\n", __FUNCTION__);
        return false;
    }

    std::vector<VkPhysicalDevice> PhysicalDevices;
    PhysicalDevices.resize(PhysicalDeviceCount);
    vkEnumeratePhysicalDevices(Instance, &PhysicalDeviceCount, PhysicalDevices.data());

    Result = glfwCreateWindowSurface(Instance, CreatedWindow, nullptr, &Surface);
    if (Result != VK_SUCCESS)
    {
        Logger::log(1, "%s: Could not create Vulkan surface\n", __FUNCTION__);
        return false;
    }

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
    vkDestroySurfaceKHR(Instance, Surface, nullptr);
    vkDestroyInstance(Instance, nullptr);
    glfwDestroyWindow(CreatedWindow);
    glfwTerminate();
}

void Window::handleWindowMoveEvents(int xPosition, int yPosition)
{
    Logger::log(1, "%s: Window has been moved to %i/%i\n", __FUNCTION__, xPosition, yPosition);
}

void Window::handleWindowMinimizedEvents(int minimized)
{
    if (minimized)
    {
        Logger::log(1, "%s: Window has been minimized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: Window has been restored\n", __FUNCTION__);
    }
}

void Window::handleWindowMaximizedEvents(int maximized)
{
    if (maximized)
    {
        Logger::log(1, "%s: Window has been maximized\n", __FUNCTION__);
    }
    else
    {
        Logger::log(1, "%s: Window has been restored\n", __FUNCTION__);
    }
}

void Window::handleWindowCloseEvents() { Logger::log(1, "%s: Window has been closed\n", __FUNCTION__); }
