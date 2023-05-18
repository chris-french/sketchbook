#pragma once

#include "./physics.h"

namespace SketchBook
{
namespace World
{
    struct Actor : public KinematicObject
    {
        float m_max_speed = 1.f // meter per second
    };
}
}