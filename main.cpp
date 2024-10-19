#include "Logger.h"
#include "Window.h"
#include <iostream>

int main()
{
    std::unique_ptr<Window> ApplicationWindow = std::make_unique<Window>();

    if (!ApplicationWindow->init(640, 480, "Sokudo Engine"))
    {
        Logger::log(1, "%s error: window init error\n", __FUNCTION__);
        return -1;
    }

    ApplicationWindow->mainLoop();
    ApplicationWindow->cleanup();

    return 0;
}
