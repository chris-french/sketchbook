#pragma once
#include <vulkan_init.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstddef>
#include <vector>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp> 
#include <functional>
#include <glm/common.hpp>
#include <boost/lexical_cast.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES // for C++
#include <cmath>
#include <math.h>

#define MIN_FLOAT_STR boost::lexical_cast<std::string>(std::numeric_limits<float>::lowest())
#define MAX_FLOAT_STR boost::lexical_cast<std::string>(std::numeric_limits<float>::max())

struct SDL_Window;

struct WorldConfig {
    // pixels / meters conversion
};

struct EntityBase
{
    std::string entity_name;
};

struct ModelMatrix
{
    glm::vec3 scale {1.f};
    float rotation_angle{0.f}; // radians
    glm::vec3 rotation_axis{1.f};
    glm::vec3 translation{1.f};

    inline glm::mat4 render_matrix()
    {
        glm::mat4 S = glm::scale(glm::mat4(1.0), scale);
        glm::mat4 R = glm::rotate(glm::mat4(1.0), glm::clamp(rotation_angle, -2.f * (float)M_PI, 2.f * (float)M_PI), rotation_axis);
        glm::mat4 T = glm::translate(glm::mat4(1.0), translation);
        return (T * R) * S;
    }
};


struct VertexInputDescription {

	std::vector<VkVertexInputBindingDescription> bindings;
	std::vector<VkVertexInputAttributeDescription> attributes;

	VkPipelineVertexInputStateCreateFlags flags = 0;
};

class Vertex_RBG_Normal {
public:
    glm::vec3 position = {};
    glm::vec3 normal = {};
    glm::vec3 color = {};

    Vertex_RBG_Normal()=default;

    Vertex_RBG_Normal(float x, float y, float z)
        : position{x,y,z}, normal{}, color{}
    {
    }

    static VertexInputDescription get_vertex_description()
    {
        VertexInputDescription description;

        //we will have just 1 vertex buffer binding, with a per-vertex rate
        VkVertexInputBindingDescription mainBinding = {};
        mainBinding.binding = 0;
        mainBinding.stride = sizeof(Vertex_RBG_Normal);
        mainBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        description.bindings.push_back(mainBinding);

        //Position will be stored at Location 0
        VkVertexInputAttributeDescription positionAttribute = {};
        positionAttribute.binding = 0;
        positionAttribute.location = 0;
        positionAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        positionAttribute.offset = offsetof(Vertex_RBG_Normal, position);

        //Normal will be stored at Location 1
        VkVertexInputAttributeDescription normalAttribute = {};
        normalAttribute.binding = 0;
        normalAttribute.location = 1;
        normalAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        normalAttribute.offset = offsetof(Vertex_RBG_Normal, normal);

        //Color will be stored at Location 2
        VkVertexInputAttributeDescription colorAttribute = {};
        colorAttribute.binding = 0;
        colorAttribute.location = 2;
        colorAttribute.format = VK_FORMAT_R32G32B32_SFLOAT;
        colorAttribute.offset = offsetof(Vertex_RBG_Normal, color);

        description.attributes.push_back(positionAttribute);
        description.attributes.push_back(normalAttribute);
        description.attributes.push_back(colorAttribute);
        return description;
    }
};



struct RenderObject
{
    std::string name {"default_render_object"};
    std::string mesh_name;
    std::string material_name;
    ModelMatrix model_matrix;

    bool b_should_render{true};

    std::function<void(const float delta)> render = nullptr;
};

struct AllocatedBuffer 
{
    VkBuffer buffer;
    VmaAllocation allocation;
};

struct AllocatedImage 
{
    VkImage image;
    VmaAllocation allocation;
};

struct DepthImage
{
    VkImageView depth_view_image;
    AllocatedImage allocated_image;
    VkFormat depthFormat;
};

struct Vertex_RBG_Normal_Mesh 
{
    std::vector<Vertex_RBG_Normal> vertices;
    AllocatedBuffer vertex_buffer;
};


struct MeshPushConstants {
    glm::mat4 render_matrix{};
	glm::vec4 normal_color{.8f, 0.f, 0.f, 0.f};
};





   







