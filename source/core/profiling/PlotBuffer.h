#pragma once

#include "deque"

namespace Core::Profiling
{
class PlotBuffer
{
public:
    explicit PlotBuffer(const size_t inMaxSize = 60) : maxSize(inMaxSize) {}

    void push(float value);

    void draw(const char* label);

private:
    std::deque<float> values;
    size_t maxSize;
};
} // namespace Core::Profiling