#pragma once
#include <cstdio>

class Logger
{
public:
    template <typename... Args> static void log(unsigned int logLevel, Args... args)
    {
        if (logLevel <= currentLogLevel)
        {
            std::printf(args...);
            std::printf("\n");
            std::fflush(stdout);
        }
    }

    static void log(unsigned int logLevel, const char* message)
    {
        if (logLevel <= currentLogLevel)
        {
            std::puts(message);
            std::fflush(stdout);
        }
    }

    static void setLogLevel(unsigned int inLogLevel) { currentLogLevel = inLogLevel; }

private:
    static unsigned int currentLogLevel;
};
