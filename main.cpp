#define GLM_ENABLE_EXPERIMENTAL
#include <iostream>
#include "core/window/Window.h"
#include "core/tools/Logger.h"

int main()
{
    std::unique_ptr<Core::Application::Window> ApplicationWindow = std::make_unique<Core::Application::Window>();

    if (!ApplicationWindow->init(1280, 900, "Sokudo Engine"))
    {
        Logger::log(1, "%s error: window init error\n", __FUNCTION__);
        return -1;
    }

    ApplicationWindow->mainLoop();
    ApplicationWindow->cleanup();

    return 0;
}
