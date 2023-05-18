#pragma once

#include "sk_types.h"
#include "sk_initializers.h"
#include "vulkan_structs.h"
#include "data_structures.h"
#include "macros.h"
#include "registry.h"
#include "sk_views.h"
#include "VkBootstrap.h"
#include "components.h"
#include <chrono>

namespace SketchBook
{
namespace Core
{
    constexpr unsigned int FRAME_OVERLAP = 2;

    using DataStructures::DeletionQueue;

    struct GPUCameraData{
        glm::mat4 view;
        glm::mat4 proj;
        glm::mat4 view_proj;
        glm::vec3 position;
    };

    struct FrameData {
        VkSemaphore present_semaphore;
        VkSemaphore render_semaphore;
        VkFence render_fence;	

        VkCommandPool command_pool;
        VkCommandBuffer main_command_buffer;

        AllocatedBuffer view_buffer;
        VkDescriptorSet global_descriptor;
    };

    class EngineVulkanBase {

    public:

        EngineVulkanBase(int width, int height);

        Registry registry;

        const InitDetails& vk_init() const { return m_vk_init; }

        inline FrameData& get_current_frame(){ return m_frames[m_frameNumber % FRAME_OVERLAP]; }
        void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);
        AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        bool load_shader_module(const char *filePath, VkShaderModule *outShaderModule);
        void load_mesh_from_obj_file(const char *filename, Vertex_RBG_Normal_Mesh* mesh, const char* base_dir = "assets");

        bool load_image_from_file(const char* file, AllocatedImage& outImage);

        inline const Views::AppWindow&          get_window() const          { return window; }
        inline const Views::AppWindow&          get_window()                { return window; }

    protected:
        bool                        m_isInitialized {false};
        std::string                 m_app_name {"SketchBook"};
        int                         m_frameNumber {0};
        InitDetails                 m_vk_init = {};
        VkRenderPass                m_render_pass;
        FrameData                   m_frames[FRAME_OVERLAP];
        DeletionQueue               m_mainDeletionQueue; //todo: implement basic garbage collection?
        VmaAllocator                m_allocator; //vma lib allocator
        DepthImage                  m_depth_image;
        std::vector<VkFramebuffer>  m_frame_buffers;
        Views::AppWindow            window;
        UploadContext               upload_context;

        VkDescriptorSetLayout       m_global_set_layout;
        VkDescriptorPool            m_descriptor_pool;

        std::chrono::time_point<std::chrono::system_clock>  m_time_last_frame;

        struct BufferData {
            GPUCameraData camera_data;

        } buffer_data;
    
        void init_vulkan();
        void init_swapchain();
        void init_framebuffers();
        void init_default_renderpass();
        void init_descriptors();
        void init_sync_structures();
        void init_commands();
        void upload_mesh(void* mesh_ptr);

    public:
        void cleanup();

    };
}
}


