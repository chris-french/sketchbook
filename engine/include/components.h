#pragma once

#include "sk_types.h"
#include <vulkan_init.h>
#include "VkBootstrap.h"

namespace SketchBook
{
namespace Components
{
    struct Pipeline
    {
        std::string name;
        VkPipeline pipeline;
    };

    struct PipelineLayout
    {
        std::string name;
        VkPipelineLayout pipeline_layout;
    };

    struct Mesh 
    {
        std::string name;
        Vertex_RBG_Normal_Mesh mesh;
    };

    struct Material
    {
        std::string name;
        std::string pipeline_name;
        std::string pipeline_layout_name;
    };

    struct Texture 
    {
        std::string name;
        std::string image_name;
        VkImageView image_view;
    };

    struct WorldPosition
    {
        float x;
        float y; 
        float height;
    };

}
}

