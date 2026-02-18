#pragma once

#include "tools/Logger.h"

#if defined(_WIN32)
#define SE_DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define SE_DEBUG_BREAK() raise(SIGTRAP)
#endif

#ifdef NDEBUG
#define SE_ASSERT(condition, ...) ((void)0)
#else
#define SE_ASSERT(condition, ...)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            Logger::log(0, "[ASSERT] %s:%d | Condition: %s", __FILE__, __LINE__, #condition);                          \
            Logger::log(0, __VA_ARGS__);                                                                               \
            SE_DEBUG_BREAK();                                                                                          \
        }                                                                                                              \
    } while (0)
#endif

#define SE_ENSURE(condition, ...)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(condition))                                                                                              \
        {                                                                                                              \
            Logger::log(0, "[ENSURE] %s:%d | Failed: %s", __FILE__, __LINE__, #condition);                             \
            Logger::log(0, __VA_ARGS__);                                                                               \
        }                                                                                                              \
    } while (0)

#define SE_VK_CHECK(result, ...)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        VkResult res = (result);                                                                                       \
        if (res != VK_SUCCESS)                                                                                         \
        {                                                                                                              \
            Logger::log(0, "[VULKAN ERROR] %s:%d | Code: %d", __FILE__, __LINE__, res);                                \
            Logger::log(0, __VA_ARGS__);                                                                               \
            SE_DEBUG_BREAK();                                                                                          \
        }                                                                                                              \
    } while (0)