#pragma once

#include "sk_types.h"
#include <vector>

namespace SketchBook
{
namespace Core
{
    struct InitDetails
    {
        VkInstance instance;                                // Vulkan library handle
        VkDebugUtilsMessengerEXT debug_messenger;           // Vulkan debug output handle
        VkPhysicalDevice chosen_gpu;                        // GPU chosen as the default device
        VkDevice device;                                    // Vulkan device for commands
        VkSurfaceKHR surface;                               // Vulkan window surface
        VkSwapchainKHR swapchain;                           // swapchain
        VkFormat swapchain_image_format;                    // image format expected by the windowing system
        std::vector<VkImage> swapchain_images;              //array of images from the swapchain
        std::vector<VkImageView> swapchain_image_views;     //array of image-views from the swapchain
        VkQueue graphics_queue;                             //queue we will submit to
        uint32_t graphics_queue_family;                     //family of that queue
        VkCommandPool command_pool;                         //the command pool for our commands
        VkCommandBuffer main_command_buffer;                //the buffer we will record into
    };
}
}

