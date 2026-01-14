#pragma once

#include "vector"

namespace Core::Profiling
{
struct PlotStats
{
    float min = 0.f;
    float max = 0.f;
    float average = 0.f;
    float last = 0.f;
};

class PlotBuffer
{
public:
    explicit PlotBuffer(const size_t maxSize = 60);

    void push(float value);

    void draw(const char* label);

private:
    void updateStats();

    std::vector<float> values;
    size_t offset;
    PlotStats stats;
};
} // namespace Core::Profiling