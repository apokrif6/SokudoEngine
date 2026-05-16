#pragma once

#include <cstdint>
#include <functional>
#include <string_view>

namespace Core::Animations
{
struct AnimParamID
{
    uint32_t id;

    explicit AnimParamID(const char* string) : id(std::hash<std::string_view>{}(string)) {}
};
} // namespace Core::Animations
