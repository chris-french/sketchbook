#pragma once

#include <random>
#include <chrono>

namespace SketchBook
{
namespace World
{
namespace Random
{
    float binom(int t = 1, float p = 0.5) {
        static std::mt19937 
            rng(std::chrono::system_clock::now().time_since_epoch().count());
        std::binomial_distribution<> binom(t, p);
        return binom(rng);
    }
}
}
}

