#pragma once

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL.h>
#include <SDL_vulkan.h>

namespace SketchBook
{
namespace Views
{
    struct AppWindow
    {   
        VkExtent2D windowExtent;
        struct SDL_Window* sdl_window;

        AppWindow(uint32_t width, uint32_t height) :
            windowExtent{width,height},
            sdl_window{nullptr}
        {}
    };
}
}