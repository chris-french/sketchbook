#pragma once
#include "spdlog/spdlog.h"

namespace SketchBook
{
namespace World
{
    struct Event_NextTick
    {
        double tick_count;
        float seconds_per_tick;

        const double total_seconds() const { return static_cast<double>(seconds_per_tick) * tick_count; } 
    };
}
}