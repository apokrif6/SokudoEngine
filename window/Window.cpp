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
                             [](GLFWwindow* window, int xPosition, int yPosition)
                             {
                                 auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                 currentWindow->handleWindowMoveEvents(xPosition, yPosition);
                             });

    glfwSetWindowIconifyCallback(CreatedWindow,
                                 [](GLFWwindow* window, int minimized)
                                 {
                                     auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                     currentWindow->handleWindowMinimizedEvents(minimized);
                                 });

    glfwSetWindowMaximizeCallback(CreatedWindow,
                                  [](GLFWwindow* window, int maximized)
                                  {
                                      auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                      currentWindow->handleWindowMaximizedEvents(maximized);
                                  });

    glfwSetWindowSizeCallback(CreatedWindow,
                              [](GLFWwindow* window, int width, int height)
                              {
                                  auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                  currentWindow->handleWindowResizedEvents(width, height);
                              });

    glfwSetWindowCloseCallback(CreatedWindow,
                               [](GLFWwindow* window)
                               {
                                   auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   currentWindow->handleWindowCloseEvents();
                               });

    glfwSetKeyCallback(CreatedWindow,
                       [](GLFWwindow* window, int key, int scancode, int action, int mode)
                       {
                           auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                           currentWindow->handleKeyEvents(key, scancode, action, mode);
                       });

    glfwSetMouseButtonCallback(CreatedWindow,
                               [](GLFWwindow* window, int button, int action, int mods)
                               {
                                   auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   currentWindow->handleMouseButtonEvents(button, action, mods);
                               });

    glfwSetCursorPosCallback(CreatedWindow,
                             [](GLFWwindow* window, double xPosition, double yPosition)
                             {
                                 auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                 currentWindow->handleMousePositionEvents(xPosition, yPosition);
                             });

    glfwSetCursorEnterCallback(CreatedWindow,
                               [](GLFWwindow* window, int enter)
                               {
                                   auto currentWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
                                   currentWindow->handleMouseEnterLeaveEvents(enter);
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

    VkResult result = vkCreateInstance(&CreateInfo, nullptr, &Instance);
    if (result != VK_SUCCESS)
    {
        Logger::log(1, "%s: Could not create Vulkan Instance\n", __FUNCTION__);
        return false;
    }

    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(Instance, &physicalDeviceCount, nullptr);

    if (physicalDeviceCount == 0)
    {
        Logger::log(1, "%s: No Vulkan capable GPU found\n", __FUNCTION__);
        return false;
    }

    std::vector<VkPhysicalDevice> PhysicalDevices;
    PhysicalDevices.resize(physicalDeviceCount);
    vkEnumeratePhysicalDevices(Instance, &physicalDeviceCount, PhysicalDevices.data());

    result = glfwCreateWindowSurface(Instance, CreatedWindow, nullptr, &Surface);
    if (result != VK_SUCCESS)
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

void Window::handleWindowResizedEvents(int width, int height)
{
    Logger::log(1, "%s: Window size has been changed to %lf/%lf\n", __FUNCTION__, width, height);
}

void Window::handleWindowCloseEvents() { Logger::log(1, "%s: Window has been closed\n", __FUNCTION__); }

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
