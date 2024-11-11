#include <iostream>
#include "window/Window.h"
#include "tools/Logger.h"

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
