#include "engine_base.h"
#include "VkBootstrap.h"
#include "sk_views.h"
#include "spdlog/spdlog.h"
#include "sk_io.h"

#include <iostream>
#include <fstream>

#include <SDL2/SDL.h>
#include <SDL.h>
#include <SDL_vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <ctime>

namespace SketchBook
{
namespace Core
{

    EngineVulkanBase::EngineVulkanBase(int width, int height)
        : window{Views::AppWindow(width,height)}
    {
        spdlog::info("Creating SketchBook Engine | name={}", m_app_name);
        spdlog::info("float limit=[{},{}]", MIN_FLOAT_STR, MAX_FLOAT_STR); 

        SDL_Init(SDL_INIT_VIDEO);

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
            
        //create blank SDL window for our application
        window.sdl_window = (SDL_Window*)SDL_CreateWindow(
            m_app_name.c_str(),
            SDL_WINDOWPOS_UNDEFINED, //window position x (don't care)
            SDL_WINDOWPOS_UNDEFINED, //window position y (don't care)
            window.windowExtent.width,  //in pixels
            window.windowExtent.height, //in pixels
            window_flags 
        );
        
    }


    void EngineVulkanBase::immediate_submit(std::function<void(VkCommandBuffer cmd)> && function)
    {
        VkCommandBuffer cmd = upload_context._commandBuffer;
        //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
        VkCommandBufferBeginInfo cmdBeginInfo = VkInit::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

        function(cmd);

        VK_CHECK(vkEndCommandBuffer(cmd));

        VkSubmitInfo submit = VkInit::submit_info(&cmd);

        //submit command buffer to the queue and execute it.
        // _renderFence will now block until the graphic commands finish execution
        VK_CHECK(vkQueueSubmit(m_vk_init.graphics_queue, 1, &submit, upload_context._uploadFence));

        vkWaitForFences(m_vk_init.device, 1, &upload_context._uploadFence, true, 9999999999);
        vkResetFences(m_vk_init.device, 1, &upload_context._uploadFence);

        vkResetCommandPool(m_vk_init.device, upload_context._commandPool, 0);
    }


    void EngineVulkanBase::init_commands()
    {
        //create a command pool for commands submitted to the graphics queue.
        //we also want the pool to allow for resetting of individual command buffers
        VkCommandPoolCreateInfo commandPoolInfo = VkInit::command_pool_create_info(m_vk_init.graphics_queue_family, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        for(int i = 0; i< FRAME_OVERLAP; i++)
        {
            VK_CHECK(vkCreateCommandPool(m_vk_init.device, &commandPoolInfo, nullptr, &m_frames[i].command_pool));
            VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::command_buffer_allocate_info(m_frames[i].command_pool, 1);

            VK_CHECK(vkAllocateCommandBuffers(m_vk_init.device, &cmdAllocInfo, &m_frames[i].main_command_buffer));

            m_mainDeletionQueue.push([=]() {
                vkDestroyCommandPool(m_vk_init.device, m_frames[i].command_pool, nullptr);
            });
        }

        VkCommandPoolCreateInfo uploadCommandPoolInfo = VkInit::command_pool_create_info(m_vk_init.graphics_queue_family);
        //create pool for upload context
        VK_CHECK(vkCreateCommandPool(m_vk_init.device, &uploadCommandPoolInfo, nullptr, &upload_context._commandPool));

        m_mainDeletionQueue.push([=]() {
            vkDestroyCommandPool(m_vk_init.device, upload_context._commandPool, nullptr);
        });

        //allocate the default command buffer that we will use for the instant commands
        VkCommandBufferAllocateInfo cmdAllocInfo = VkInit::command_buffer_allocate_info(upload_context._commandPool, 1);
        VK_CHECK(vkAllocateCommandBuffers(m_vk_init.device, &cmdAllocInfo, &upload_context._commandBuffer));
    }

    void EngineVulkanBase::load_mesh_from_obj_file(const char *filename, Vertex_RBG_Normal_Mesh* mesh, const char* base_dir)
    {
        spdlog::info("Loading mesh[filename={},base_dir={}]", filename, base_dir);

        //attrib will contain the vertex arrays of the file
        tinyobj::attrib_t attrib;
        //shapes contains the info for each separate object in the file
        std::vector<tinyobj::shape_t> shapes;
        //materials contains the information about the material of each shape, but we won't use it.
        std::vector<tinyobj::material_t> materials;

        //error and warning output from the load function
        std::string warn;
        std::string err;

        //load the OBJ file
        IO::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, base_dir);
        if (!warn.empty()) {
             spdlog::warn(warn);
        }
        //if we have any error, print it to the console, and break the mesh loading.
        //This happens if the file can't be found or is malformed
        if (!err.empty()) {
            spdlog::error(err);
            return;
        }

        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

                //hardcode loading to triangles
                int fv = 3;

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    //vertex position
                    tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                    tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                    tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                    //vertex normal
                    tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                    tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                    tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

                    //copy it into our vertex
                    Vertex_RBG_Normal new_vert;
                    new_vert.position.x = vx;
                    new_vert.position.y = vy;
                    new_vert.position.z = vz;

                    new_vert.normal.x = nx;
                    new_vert.normal.y = ny;
                    new_vert.normal.z = nz;

                    //we are setting the vertex color as the vertex normal. This is just for display purposes
                    new_vert.color = new_vert.normal;

                    mesh->vertices.push_back(new_vert);
                }
                index_offset += fv;
            }
        }
    }

    bool EngineVulkanBase::load_image_from_file(const char *file, AllocatedImage &outImage)
    {
        int texWidth, texHeight, texChannels;

	    stbi_uc* pixels = stbi_load(file, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        if (!pixels) {
            spdlog::error("Failed to load texture file: {}", file);
            return false;
        }

        void* pixel_ptr = pixels;
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        //the format R8G8B8A8 matches exactly with the pixels loaded from stb_image lib
        VkFormat image_format = VK_FORMAT_R8G8B8A8_SRGB;

        //allocate temporary buffer for holding texture data to upload
        AllocatedBuffer stagingBuffer = create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);

        //copy data to buffer
        void* data;
        vmaMapMemory(m_allocator, stagingBuffer.allocation, &data);

        memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));

        vmaUnmapMemory(m_allocator, stagingBuffer.allocation);
        //we no longer need the loaded data, so we can free the pixels as they are now in the staging buffer
        stbi_image_free(pixels);

        VkExtent3D imageExtent;
        imageExtent.width = static_cast<uint32_t>(texWidth);
        imageExtent.height = static_cast<uint32_t>(texHeight);
        imageExtent.depth = 1;

        VkImageCreateInfo dimg_info = VkInit::image_create_info(image_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, imageExtent);

        AllocatedImage newImage;

        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        //allocate and create the image
        vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &newImage.image, &newImage.allocation, nullptr);

        immediate_submit([&](VkCommandBuffer cmd) {
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = 1;
            range.baseArrayLayer = 0;
            range.layerCount = 1;

            VkImageMemoryBarrier imageBarrier_toTransfer = {};
            imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toTransfer.image = newImage.image;
            imageBarrier_toTransfer.subresourceRange = range;

            imageBarrier_toTransfer.srcAccessMask = 0;
            imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            //barrier the image into the transfer-receive layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);

            VkBufferImageCopy copyRegion = {};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;

            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent = imageExtent;

            //copy the buffer into the image
            vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        
            VkImageMemoryBarrier imageBarrier_toReadable = imageBarrier_toTransfer;

            imageBarrier_toReadable.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toReadable.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            imageBarrier_toReadable.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageBarrier_toReadable.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            //barrier the image into the shader readable layout
            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toReadable);
        });

        m_mainDeletionQueue.push([=]() {
		    vmaDestroyImage(m_allocator, newImage.image, newImage.allocation);
        });

        vmaDestroyBuffer(m_allocator, stagingBuffer.buffer, stagingBuffer.allocation);

        spdlog::info("Texture loaded successfully from file: {}", file);
        outImage = newImage;

        return true;
    }

    void EngineVulkanBase::cleanup()
    {
        if (m_isInitialized) 
        {
            //make sure the gpu has stopped doing its things
            vkDeviceWaitIdle(m_vk_init.device);
            vkWaitForFences(m_vk_init.device, 1, &get_current_frame().render_fence, true, 1000000000);

            m_mainDeletionQueue.flush();
            vmaDestroyAllocator(m_allocator);
            vkDestroyDevice(m_vk_init.device, nullptr);
            vkDestroySurfaceKHR(m_vk_init.instance, m_vk_init.surface, nullptr);
            vkb::destroy_debug_utils_messenger(m_vk_init.instance, m_vk_init.debug_messenger);
            vkDestroyInstance(m_vk_init.instance, nullptr);
            SDL_DestroyWindow((SDL_Window*)window.sdl_window);
        }
    }

    AllocatedBuffer EngineVulkanBase::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
    {
        //allocate vertex buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;

        bufferInfo.size = allocSize;
        bufferInfo.usage = usage;

        VmaAllocationCreateInfo vmaallocInfo = {};
        vmaallocInfo.usage = memoryUsage;

        AllocatedBuffer newBuffer;

        VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &vmaallocInfo,
            &newBuffer.buffer,
            &newBuffer.allocation,
            nullptr));

        return newBuffer;
    }

    bool EngineVulkanBase::load_shader_module(const char *filePath, VkShaderModule *outShaderModule)
    {
        //open the file. With cursor at the end
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) 
        {
            return false;
        }

        //find what the size of the file is by looking up the location of the cursor
        //because the cursor is at the end, it gives the size directly in bytes
        size_t fileSize = (size_t)file.tellg();

        //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
        std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

        file.seekg(0); //put file cursor at beginning
        file.read((char*)buffer.data(), fileSize);
        file.close();

        //create a new shader module, using the buffer we loaded
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.pNext = nullptr;

        //codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
        createInfo.codeSize = buffer.size() * sizeof(uint32_t);
        createInfo.pCode = buffer.data();

        //check that the creation goes well.
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_vk_init.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            return false;
        }
        *outShaderModule = shaderModule;
        return true;

    }

    void EngineVulkanBase::init_framebuffers()
    {
        spdlog::info("init framebuffers");
        //create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
        VkFramebufferCreateInfo fb_info = {};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = nullptr;

        fb_info.renderPass = m_render_pass;
        fb_info.attachmentCount = 1;
        fb_info.width = window.windowExtent.width;
        fb_info.height = window.windowExtent.height;
        fb_info.layers = 1;

        //grab how many images we have in the swapchain
        const uint32_t swapchain_imagecount = m_vk_init.swapchain_images.size();
        m_frame_buffers = std::vector<VkFramebuffer>(swapchain_imagecount);

        //create framebuffers for each of the swapchain image views
        for (int i = 0; i < swapchain_imagecount; i++) {

            VkImageView attachments[2];
            attachments[0] = m_vk_init.swapchain_image_views[i];
            attachments[1] = m_depth_image.depth_view_image;

            fb_info.pAttachments = attachments;
            fb_info.attachmentCount = 2;

            VK_CHECK(vkCreateFramebuffer(m_vk_init.device, &fb_info, nullptr, &m_frame_buffers[i]));

            m_mainDeletionQueue.push([=]() {
                vkDestroyFramebuffer(m_vk_init.device, m_frame_buffers[i], nullptr);
                vkDestroyImageView(m_vk_init.device, m_vk_init.swapchain_image_views[i], nullptr);
            });
        }
    }
    
    void EngineVulkanBase::init_vulkan()
    {
        vkb::InstanceBuilder builder;

        //make the Vulkan instance, with basic debug features
        auto inst_ret = builder.set_app_name(m_app_name.c_str())
            .request_validation_layers(true)
            .require_api_version(1, 1, 0)
            .use_default_debug_messenger()
            .build();

        vkb::Instance vkb_inst = inst_ret.value();
        m_vk_init.instance = vkb_inst.instance;                              //store the instance
        m_vk_init.debug_messenger = vkb_inst.debug_messenger;                //store the debug messenger

        SDL_Vulkan_CreateSurface(window.sdl_window, m_vk_init.instance, &m_vk_init.surface);    // get the surface of the window we opened with SDL

        //use vkbootstrap to select a GPU.
        //We want a GPU that can write to the SDL surface and supports Vulkan 1.1
        vkb::PhysicalDeviceSelector selector{ vkb_inst };
        vkb::PhysicalDevice physicalDevice = selector
            .set_minimum_version(1, 1)
            .set_surface(m_vk_init.surface)
            .select()
            .value();

        //create the final Vulkan device
        vkb::DeviceBuilder deviceBuilder{ physicalDevice };

        vkb::Device vkbDevice = deviceBuilder.build().value();

        // Get the VkDevice handle used in the rest of a Vulkan application
        m_vk_init.device = vkbDevice.device;
        m_vk_init.chosen_gpu = physicalDevice.physical_device;

        // use vkbootstrap to get a Graphics queue
        m_vk_init.graphics_queue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        m_vk_init.graphics_queue_family = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

        //initialize the memory allocator
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.physicalDevice = m_vk_init.chosen_gpu;
        allocatorInfo.device = m_vk_init.device;
        allocatorInfo.instance = m_vk_init.instance;
        vmaCreateAllocator(&allocatorInfo, &m_allocator);
    }

    void EngineVulkanBase::init_default_renderpass()
    {
        spdlog::info("default renderpass");

        // the renderpass will use this color attachment.
        VkAttachmentDescription color_attachment = {};
        //the attachment will have the format needed by the swapchain
        color_attachment.format = m_vk_init.swapchain_image_format;
        //1 sample, we won't be doing MSAA
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        // we Clear when this attachment is loaded
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // we keep the attachment stored when the renderpass ends
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //we don't care about stencil
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        //we don't know or care about the starting layout of the attachment
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        //after the renderpass ends, the image has to be on a layout ready for display
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        //subpass
        VkAttachmentReference color_attachment_ref = {};
        //attachment number will index into the pAttachments array in the parent renderpass itself
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depth_attachment = {};
        // Depth attachment
        depth_attachment.flags = 0;
        depth_attachment.format = m_depth_image.depthFormat;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref = {};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depth_dependency = {};
        depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depth_dependency.dstSubpass = 0;
        depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depth_dependency.srcAccessMask = 0;
        depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkAttachmentDescription attachments[2] = { color_attachment,depth_attachment };
        VkSubpassDependency dependencies[2] = { dependency, depth_dependency };

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        //connect the color attachment to the info
        render_pass_info.attachmentCount = 2;
        render_pass_info.pAttachments = &attachments[0];
        //connect the subpass to the info
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 2;
        render_pass_info.pDependencies = &dependencies[0];

        VK_CHECK(vkCreateRenderPass(m_vk_init.device, &render_pass_info, nullptr, &m_render_pass));

        m_mainDeletionQueue.push([=]() {
            vkDestroyRenderPass(m_vk_init.device, m_render_pass, nullptr);
        });
    }

    void EngineVulkanBase::init_swapchain()
    {
        /*
        * Init Swap Chain using Builder
        */
        vkb::SwapchainBuilder swapchainBuilder{m_vk_init.chosen_gpu, m_vk_init.device, m_vk_init.surface };

        vkb::Swapchain vkbSwapchain = swapchainBuilder
            .use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR) //use vsync present mode
            .set_desired_extent(window.windowExtent.width, window.windowExtent.height) //todo: needs to be reset when window resized
            .build()
            .value();

        //store swapchain and its related images
        m_vk_init.swapchain = vkbSwapchain.swapchain;
        m_vk_init.swapchain_images = vkbSwapchain.get_images().value();
        m_vk_init.swapchain_image_views = vkbSwapchain.get_image_views().value();

        m_vk_init.swapchain_image_format = vkbSwapchain.image_format;

        m_mainDeletionQueue.push([=]() {
            vkDestroySwapchainKHR(m_vk_init.device, m_vk_init.swapchain, nullptr);
        });

        /*
        * Depth Buffer
        */
        VkExtent3D depthImageExtent = {
            window.windowExtent.width,
            window.windowExtent.height,
            1
        };

        //hardcoding the depth format to 32 bit float
        m_depth_image.depthFormat = VK_FORMAT_D32_SFLOAT;

        //the depth image will be an image with the format we selected and Depth Attachment usage flag
        VkImageCreateInfo dimg_info = VkInit::image_create_info(m_depth_image.depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

        //for the depth image, we want to allocate it from GPU local memory
        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        //allocate and create the image
        vmaCreateImage(m_allocator, &dimg_info, &dimg_allocinfo, &m_depth_image.allocated_image.image, &m_depth_image.allocated_image.allocation, nullptr);

        //build an image-view for the depth image to use for rendering
        VkImageViewCreateInfo dview_info = VkInit::imageview_create_info(m_depth_image.depthFormat, m_depth_image.allocated_image.image, VK_IMAGE_ASPECT_DEPTH_BIT);

        VK_CHECK(vkCreateImageView(m_vk_init.device, &dview_info, nullptr, &m_depth_image.depth_view_image));

        //add to deletion queues
        m_mainDeletionQueue.push([=]() {
            vkDestroyImageView(m_vk_init.device, m_depth_image.depth_view_image, nullptr);
            vmaDestroyImage(m_allocator, m_depth_image.allocated_image.image, m_depth_image.allocated_image.allocation);
        });
    }

    void EngineVulkanBase::init_sync_structures()
    {
        //create synchronization structures

        VkFenceCreateInfo fenceCreateInfo = VkInit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
        VkSemaphoreCreateInfo semaphoreCreateInfo = VkInit::create_semaphore_create_info();

        for (int i = 0; i < FRAME_OVERLAP; i++) 
        {    
            VK_CHECK(vkCreateFence(m_vk_init.device, &fenceCreateInfo, nullptr, &m_frames[i].render_fence));
            VK_CHECK(vkCreateSemaphore(m_vk_init.device, &semaphoreCreateInfo, nullptr, &m_frames[i].present_semaphore));
            VK_CHECK(vkCreateSemaphore(m_vk_init.device, &semaphoreCreateInfo, nullptr, &m_frames[i].render_semaphore));

            //enqueue the destruction of semaphores
            m_mainDeletionQueue.push([=]() {
                vkDestroyFence(m_vk_init.device, m_frames[i].render_fence, nullptr);
                vkDestroySemaphore(m_vk_init.device, m_frames[i].present_semaphore, nullptr);
                vkDestroySemaphore(m_vk_init.device, m_frames[i].render_semaphore, nullptr);
            });
        }

        VkFenceCreateInfo uploadFenceCreateInfo = VkInit::fence_create_info();

        VK_CHECK(vkCreateFence(m_vk_init.device, &uploadFenceCreateInfo, nullptr, &upload_context._uploadFence));
        m_mainDeletionQueue.push([=]() {
            vkDestroyFence(m_vk_init.device, upload_context._uploadFence, nullptr);
        });
        
    }

    void EngineVulkanBase::upload_mesh(void* mesh_ptr)
    {
        if (Vertex_RBG_Normal_Mesh* mesh = static_cast<Vertex_RBG_Normal_Mesh*>(mesh_ptr))
        {
            const size_t bufferSize= mesh->vertices.size() * sizeof(Vertex_RBG_Normal);
            //allocate staging buffer
            VkBufferCreateInfo stagingBufferInfo = {};
            stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferInfo.pNext = nullptr;

            stagingBufferInfo.size = bufferSize;
            stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            //let the VMA library know that this data should be on CPU RAM
            VmaAllocationCreateInfo vmaallocInfo = {};
            vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

            AllocatedBuffer stagingBuffer;

            //allocate the buffer
            VK_CHECK(vmaCreateBuffer(m_allocator, &stagingBufferInfo, &vmaallocInfo,
                &stagingBuffer.buffer,
                &stagingBuffer.allocation,
                nullptr));

            void* data;
            vmaMapMemory(m_allocator, stagingBuffer.allocation, &data);

            memcpy(data, mesh->vertices.data(), mesh->vertices.size() * sizeof(Vertex_RBG_Normal));

            vmaUnmapMemory(m_allocator, stagingBuffer.allocation);

            //allocate vertex buffer
            VkBufferCreateInfo vertexBufferInfo = {};
            vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            vertexBufferInfo.pNext = nullptr;
            //this is the total size, in bytes, of the buffer we are allocating
            vertexBufferInfo.size = bufferSize;
            //this buffer is going to be used as a Vertex Buffer
            vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            //let the VMA library know that this data should be GPU native
            vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            //allocate the buffer
            VK_CHECK(vmaCreateBuffer(m_allocator, &vertexBufferInfo, &vmaallocInfo,
                &mesh->vertex_buffer.buffer,
                &mesh->vertex_buffer.allocation,
                nullptr));

            immediate_submit([=](VkCommandBuffer cmd) {
                VkBufferCopy copy;
                copy.dstOffset = 0;
                copy.srcOffset = 0;
                copy.size = bufferSize;
                vkCmdCopyBuffer(cmd, stagingBuffer.buffer, mesh->vertex_buffer.buffer, 1, &copy);
            });

            m_mainDeletionQueue.push([=]() {
                vmaDestroyBuffer(m_allocator, mesh->vertex_buffer.buffer, mesh->vertex_buffer.allocation);
            });

            vmaDestroyBuffer(m_allocator, stagingBuffer.buffer, stagingBuffer.allocation);

        }
        else
        {
            spdlog::error("failed to load mesh");
        }
        
    }

    void EngineVulkanBase::init_descriptors()
    {
        //create a descriptor pool that will hold 10 uniform buffers
        std::vector<VkDescriptorPoolSize> sizes =
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10 }
        };

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = 0;
        pool_info.maxSets = 10;
        pool_info.poolSizeCount = (uint32_t)sizes.size();
        pool_info.pPoolSizes = sizes.data();

        vkCreateDescriptorPool(m_vk_init.device, &pool_info, nullptr, &m_descriptor_pool);
        
        //information about the binding.
        VkDescriptorSetLayoutBinding camBufferBinding = {};
        camBufferBinding.binding = 0;
        camBufferBinding.descriptorCount = 1;
        // it's a uniform buffer binding
        camBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

        // we use it from the vertex shader
        camBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutCreateInfo setinfo = {};
        setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setinfo.pNext = nullptr;

        //we are going to have 1 binding
        setinfo.bindingCount = 1;
        //no flags
        setinfo.flags = 0;
        //point to the camera buffer binding
        setinfo.pBindings = &camBufferBinding; 

        vkCreateDescriptorSetLayout(m_vk_init.device, &setinfo, nullptr, &m_global_set_layout);

        // add descriptor set layout to deletion queues
        m_mainDeletionQueue.push([=]() {
            vkDestroyDescriptorSetLayout(m_vk_init.device, m_global_set_layout, nullptr);
            vkDestroyDescriptorPool(m_vk_init.device, m_descriptor_pool, nullptr);
        });

        for (int i = 0; i < FRAME_OVERLAP; i++)
        {
            m_frames[i].view_buffer = create_buffer(sizeof(GPUCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
            
            VkDescriptorSetAllocateInfo allocInfo ={};
            allocInfo.pNext = nullptr;
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            //using the pool we just set
            allocInfo.descriptorPool = m_descriptor_pool;
            //only 1 descriptor
            allocInfo.descriptorSetCount = 1;
            //using the global data layout
            allocInfo.pSetLayouts = &m_global_set_layout;

            vkAllocateDescriptorSets(m_vk_init.device, &allocInfo, &m_frames[i].global_descriptor);
            
            VkDescriptorBufferInfo binfo;
            //it will be the camera buffer
            binfo.buffer = m_frames[i].view_buffer.buffer;
            //at 0 offset
            binfo.offset = 0;
            //of the size of a camera data struct
            binfo.range = sizeof(GPUCameraData);

            VkWriteDescriptorSet setWrite = {};
            setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            setWrite.pNext = nullptr;

            //we are going to write into binding number 0
            setWrite.dstBinding = 0;
            //of the global descriptor
            setWrite.dstSet = m_frames[i].global_descriptor;

            setWrite.descriptorCount = 1;
            //and the type is uniform buffer
            setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            setWrite.pBufferInfo = &binfo;

            vkUpdateDescriptorSets(m_vk_init.device, 1, &setWrite, 0, nullptr);

            m_mainDeletionQueue.push([=]() {
                vmaDestroyBuffer(m_allocator, m_frames[i].view_buffer.buffer, m_frames[i].view_buffer.allocation);
            });
        }
    }
}
}