#pragma once

#include <glm/gtx/transform.hpp>
#include <iostream>
#include <string>
#include <unordered_map>

#include "sk_types.h"
#include "sk_initializers.h"

#include "data_structures.h"

#include "scene.h"
#include "macros.h"

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_vulkan.h"

#include "spdlog/spdlog.h"

#include "engine_base.h"
#include "registry.h"

#include "components.h"
#include "camera.h"


namespace SketchBook
{
    template<typename SubEngine>
    class Engine : public Core::EngineVulkanBase
    {
    public:
        using BaseScene = Components::Scene;

    protected:
        BaseScene* m_current_scene = nullptr;
        VkPushConstantRange m_push_constant;
        VertexInputDescription m_vertex_description;

    public:
        Engine(int width, int height) : EngineVulkanBase(width, height)
        {
            setup_push_constant();
            setup_vertex_description();
        }

        virtual void setup_push_constant() //todo: refactor?
        {
            m_push_constant.offset = 0;
            m_push_constant.size = sizeof(MeshPushConstants);
            m_push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        }

        virtual void setup_vertex_description() //todo: refactor?
        {
            m_vertex_description = Vertex_RBG_Normal::get_vertex_description();
        }

        BaseScene* current_scene() { return m_current_scene; }

        void set_scene(const entt::entity scene_entity)
        {
            m_current_scene = registry.get_assets().get<BaseScene>(scene_entity);
        }

        void set_scene(const std::string scene_name)
        {
            const auto view = registry.get_assets().view<BaseScene>();
            for (auto entity : view)
            {
                auto &scene = view.get<BaseScene>(entity);
                if (scene.name == scene_name)
                {
                    m_current_scene = &scene;
                    return;
                }
            }
            spdlog::warn("failed to set scene to \"{}\" -- scene does not exist", scene_name);
        }

        virtual void setup() { spdlog::info("Engine::setup"); }

    protected:
        virtual void init_pipelines()   { spdlog::info("Engine::init_pipelines"); }
        virtual void load_meshes()      { spdlog::info("Engine::load_meshes"); }
        virtual void init_scenes()      { spdlog::info("Engine::init_scenes"); };

        virtual void tool_imgui()
        {
            ImGuiIO& io = ImGui::GetIO();
            ImGui::ShowDemoWindow();
        }

        void init_imgui()
        {
            spdlog::info("running init_imgui");
            //1: create descriptor pool for IMGUI
            // the size of the pool is very oversize, but it's copied from imgui demo itself.
            VkDescriptorPoolSize pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
            };

            VkDescriptorPoolCreateInfo pool_info = {};
            pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
            pool_info.maxSets = 1000;
            pool_info.poolSizeCount = std::size(pool_sizes);
            pool_info.pPoolSizes = pool_sizes;

            VkDescriptorPool imguiPool;
            VK_CHECK(vkCreateDescriptorPool(m_vk_init.device, &pool_info, nullptr, &imguiPool));


            // 2: initialize imgui library

            //this initializes the core structures of imgui
            ImGui::CreateContext();

            //this initializes imgui for SDL
            ImGui_ImplSDL2_InitForVulkan(window.sdl_window);

            //this initializes imgui for Vulkan
            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = m_vk_init.instance;
            init_info.PhysicalDevice = m_vk_init.chosen_gpu;
            init_info.Device = m_vk_init.device;
            init_info.Queue = m_vk_init.graphics_queue;
            init_info.DescriptorPool = imguiPool;
            init_info.MinImageCount = 3;
            init_info.ImageCount = 3;
            init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

            ImGui_ImplVulkan_Init(&init_info, m_render_pass);

            //execute a gpu command to upload imgui font textures
            immediate_submit([&](VkCommandBuffer cmd) {
                ImGui_ImplVulkan_CreateFontsTexture(cmd);
                });

            //clear font textures from cpu data
            ImGui_ImplVulkan_DestroyFontUploadObjects();

            //add the destroy the imgui created structures
            m_mainDeletionQueue.push([=]() {
                vkDestroyDescriptorPool(m_vk_init.device, imguiPool, nullptr);
                ImGui_ImplVulkan_Shutdown();
            });
        }
    
    public:

        virtual void custom_input_handler(SDL_Event* e)
        {
        }

        virtual void camera_handler(SDL_Event* e)
        {
            if (current_scene())
            {
                if (Components::FreeCamera* camera = current_scene()->current_camera(); camera)
                {
                    if (e->type == SDL_KEYDOWN)
                    {
                        camera->event_handler(e, false, true);
                    }
                    else
                    {
                        camera->event_handler(e, true, false);
                    }
                }
            }
            else
            {
                spdlog::warn("current scene is null");
            }
        }

        void imgui_current_camera_window()
        {
            bool* p_open = NULL;
            ImGuiWindowFlags window_flags = 0;
            IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context.");
            const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);

            if (!ImGui::Begin("Camera", p_open, window_flags) || current_scene() == nullptr)
            {
                // Early out if the window is collapsed, as an optimization.
                ImGui::End();
                return;
            }

            if (auto *camera = current_scene()->current_camera())
            {
                ImGui::SeparatorText("Camera");
                ImGui::Text("pos: x=%.2f,y=%.2f | height=%.2f", camera->m_position.x, camera->m_position.z, camera->m_position.y);
                ImGui::SameLine();
                ImGui::Text("| yaw=%.2f pitch=%.2f", camera->m_yaw, camera->m_pitch);

                ImGui::Text("Aspect: %.2f", camera->m_aspect);
                ImGui::SameLine();
                ImGui::Text("Rotation speed: %.2f", camera->m_rotation_speed);
                
                ImGui::SeparatorText("Mouse");

                ImGui::Checkbox("Right mouse down", &camera->right_mouse_btn.btn_down);
                ImGui::Text("offset=[%.2f,%.2f]", camera->right_mouse_btn.offset.x, camera->right_mouse_btn.offset.y);

                ImGui::Text("Mouse Sensitivity: %.2f", camera->m_sensitivity);
                ImGui::SameLine();
                ImGui::Text("Mouse pos: %.2f,%.2f", camera->m_focus_position.x, camera->m_focus_position.y);
                
                ImGui::SeparatorText("Geometry");

                SketchBook::PrintGlmMat4("Projection", camera->m_projection);

                SketchBook::PrintGlmMat4("View", camera->view());

                SketchBook::PrintGlmMat4("Model", camera->model());
            }
            ImGui::End();
        }
    
        void run()
        {
            SDL_Event e;
            bool bQuit = false;

            spdlog::info("run loop started");
            m_time_last_frame = std::chrono::system_clock::now();

            while (!bQuit)
            {
                while(SDL_PollEvent(&e) != 0)
                {
                    //close the window when user clicks the X button or alt-f4s
                    if (e.type == SDL_QUIT) 
                    {
                        bQuit = true;
                        return;
                    }
                    ImGui_ImplSDL2_ProcessEvent(&e);
                    custom_input_handler(&e);
                    camera_handler(&e);
                }

                ImGui_ImplVulkan_NewFrame();
                ImGui_ImplSDL2_NewFrame(window.sdl_window);
                ImGui::NewFrame();
                tool_imgui();
                draw();
            }
        }

        Components::Pipeline* get_pipeline(const std::string name)
        {
            const auto view = registry.get_assets().view<Components::Pipeline>();
            for (auto &entity: view)
            {
                auto &pipeline = view.get<Components::Pipeline>(entity);
                if (pipeline.name == name) return &pipeline;
            }
            return nullptr;
        }

        Components::PipelineLayout* get_pipeline_layout(const std::string name)
        {
            const auto view = registry.get_assets().view<Components::PipelineLayout>();
            for (auto &entity: view)
            {
                auto &layout = view.get<Components::PipelineLayout>(entity);
                if (layout.name == name) return &layout;
            }
            return nullptr;
        }

        Components::Material* get_material(const std::string &name)
        {
            const auto view = registry.get_assets().view<Components::Material>();
            for (auto &entity: view)
            {
                auto &material = view.get<Components::Material>(entity);
                if (material.name == name) return &material;
            }
            return nullptr;
        }

        Components::Mesh* get_mesh(const std::string &name)
        {
            const auto view = registry.get_assets().view<Components::Mesh>();
            for (auto &entity: view)
            {
                auto &mesh = view.get<Components::Mesh>(entity);
                if (mesh.name == name) return &mesh;
            }
            return nullptr;
        }

        void create_pipeline_layout(const std::string name, std::function<void(VkPipelineLayoutCreateInfo& info)> create_layout_callback = [](VkPipelineLayoutCreateInfo& info){})
        {
            registry.create_asset([=](entt::registry&r, const entt::entity& entity) mutable {
                VkPipelineLayoutCreateInfo create_info = SketchBook::VkInit::pipeline_layout_create_info();
                create_layout_callback(create_info);
                Components::PipelineLayout& layout = r.emplace<Components::PipelineLayout>(entity);
                layout.name = name;
                VK_CHECK(vkCreatePipelineLayout(vk_init().device, &create_info, nullptr, &layout.pipeline_layout));
      
                m_mainDeletionQueue.push([=]() {
                    vkDestroyPipelineLayout(vk_init().device, layout.pipeline_layout, nullptr);
                });
            });
        }

        void create_pipeline(
            const std::string name, 
            const std::string pipeline_layout_name, 
            VkShaderModule& vert_shader, 
            VkShaderModule& fragment_shader, 
            VkPrimitiveTopology topology, 
            VkPolygonMode polygon_mode,
            std::function<void(SketchBook::Core::PipelineBuilder& builder)> builder_callback = [](SketchBook::Core::PipelineBuilder& builder){})
        {
            Components::PipelineLayout* layout = get_pipeline_layout(pipeline_layout_name);

            registry.create_asset([=](entt::registry&r, const entt::entity& entity){

                SketchBook::Core::PipelineBuilder pipelineBuilder;
                pipelineBuilder._depthStencil = SketchBook::VkInit::depth_stencil_create_info(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
                
                //vertex input controls how to read vertices from vertex buffers. We aren't using it yet
                pipelineBuilder._vertexInputInfo = SketchBook::VkInit::vertex_input_state_create_info();

                //input assembly is the configuration for drawing triangle lists, strips, or individual points.
                //we are just going to draw triangle list
                pipelineBuilder._inputAssembly = SketchBook::VkInit::input_assembly_create_info(topology);

                //build viewport and scissor from the swapchain extents
                pipelineBuilder._viewport.x = 0.0f;
                pipelineBuilder._viewport.y = 0.0f;
                pipelineBuilder._viewport.width = (float)window.windowExtent.width;
                pipelineBuilder._viewport.height = (float)window.windowExtent.height;
                pipelineBuilder._viewport.minDepth = 0.0f;
                pipelineBuilder._viewport.maxDepth = 1.0f;

                pipelineBuilder._scissor.offset = { 0, 0 };
                pipelineBuilder._scissor.extent = window.windowExtent;

                //configure the rasterizer to draw filled triangles
                pipelineBuilder._rasterizer = SketchBook::VkInit::rasterization_state_create_info(polygon_mode);

                //we don't use multisampling, so just run the default one
                pipelineBuilder._multisampling = SketchBook::VkInit::multisampling_state_create_info();

                //a single blend attachment with no blending and writing to RGBA
                pipelineBuilder._colorBlendAttachment = SketchBook::VkInit::color_blend_attachment_state();

                builder_callback(pipelineBuilder);

                pipelineBuilder._shaderStages.push_back(
                SketchBook::VkInit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, vert_shader));

                //make sure that triangleFragShader is holding the compiled colored_triangle.frag
                pipelineBuilder._shaderStages.push_back(
                    SketchBook::VkInit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader));

                pipelineBuilder._pipelineLayout = layout->pipeline_layout;

                Components::Pipeline &pipeline = r.emplace<Components::Pipeline>(entity);
                pipeline.name = name;
                spdlog::info("internal:allocated pipeline");
                pipeline.pipeline = pipelineBuilder.build_pipeline(vk_init().device, m_render_pass);
                spdlog::info("internal:created pipeline");
                m_mainDeletionQueue.push([=]() {
                    vkDestroyPipeline(vk_init().device, pipeline.pipeline, nullptr);
                });

            });
        }


        entt::entity create_material(const std::string name, const std::string pipeline_name, const std::string pipeline_layout_name)
        {
            return registry.create_asset([=](entt::registry&r, const entt::entity& entity) mutable {
                Components::Material &material = r.emplace<Components::Material>(entity);
                material.name = name;
                material.pipeline_name = pipeline_name;
                material.pipeline_layout_name = pipeline_layout_name;
            });
        }

        entt::entity create_mesh(const std::string name, std::function<void(Vertex_RBG_Normal_Mesh* mesh)> mesh_callback)
        {
            return registry.create_asset([=](entt::registry&r, const entt::entity& entity) mutable {
                Components::Mesh &mesh_comp = r.emplace<Components::Mesh>(entity);
                mesh_comp.name = name;
                mesh_callback(&mesh_comp.mesh);
                upload_mesh((void*)&mesh_comp.mesh);
            });
        }

        entt::entity create_mesh_from_file(const std::string name, const std::string filename)
        {
            return create_mesh(name, [&,filename](Vertex_RBG_Normal_Mesh* mesh){
                load_mesh_from_obj_file(filename.c_str(), mesh);
            });
        }

        entt::entity create_scene(const std::string name, std::function<void(BaseScene* scene)> scene_callback)
        {
            return registry.create_asset([=](entt::registry&r, const entt::entity& entity) mutable {
                BaseScene &scene = r.emplace<BaseScene>(entity);
                scene.name = name;
                scene_callback(&scene);
            });
        }

        void draw()
        {
            // spdlog::info("calling draw function");
            
            //wait until the GPU has finished rendering the last frame. Timeout of 1 second
            VK_CHECK(vkWaitForFences(m_vk_init.device, 1, &get_current_frame().render_fence, true, 1000000000));
            VK_CHECK(vkResetFences(m_vk_init.device, 1, &get_current_frame().render_fence));

            //request image from the swapchain, one second timeout
            uint32_t swapchainImageIndex;
            VK_CHECK(vkAcquireNextImageKHR(m_vk_init.device, m_vk_init.swapchain, 1000000000, get_current_frame().present_semaphore, nullptr, &swapchainImageIndex));

            //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
            VK_CHECK(vkResetCommandBuffer(get_current_frame().main_command_buffer, 0));

            //naming it cmd for shorter writing
            VkCommandBuffer cmd = get_current_frame().main_command_buffer;

            //begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
            VkCommandBufferBeginInfo cmdBeginInfo = {};
            cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBeginInfo.pNext = nullptr;

            cmdBeginInfo.pInheritanceInfo = nullptr;
            cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

                //make a clear-color from frame number. This will flash with a 120*pi frame period.
            VkClearValue clearValue;
            // float flash = abs(sin(_frameNumber / 120.f));
            // clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };
            const float _col = .02f;
            clearValue.color = {{_col,_col,_col,1.0f}};

            VkClearValue depthClear;
            depthClear.depthStencil.depth = 1.f;

            VkClearValue clearValues[] = { clearValue, depthClear };

            //start the main renderpass.
            //We will use the clear color from above, and the framebuffer of the index the swapchain gave us
            VkRenderPassBeginInfo rpInfo = {};
            rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            rpInfo.pNext = nullptr;

            VkExtent2D updated_extent = window.windowExtent;
            rpInfo.renderPass = m_render_pass;
            rpInfo.renderArea.offset.x = 0;
            rpInfo.renderArea.offset.y = 0;
            rpInfo.renderArea.extent = updated_extent;
            rpInfo.framebuffer = m_frame_buffers[swapchainImageIndex];

            //connect clear values
            rpInfo.clearValueCount = 2;
            rpInfo.pClearValues = &clearValues[0];


            vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
            ImGui::Render();

            // DRAWING

            if (m_current_scene != nullptr)
            {
                if (m_current_scene->b_should_render)
                {
                    if (Components::FreeCamera *camera = m_current_scene->current_camera())
                    {   
                        glm::mat4 proj = camera->projection();
                        glm::mat4 view = camera->view();
                        const glm::mat4 camera_position_matrix = camera->get_render_matrix(false);
                        buffer_data.camera_data.position = glm::vec3(camera_position_matrix[3]);
                        buffer_data.camera_data.proj = proj;
                        buffer_data.camera_data.view = view;
                        buffer_data.camera_data.view_proj = proj * view;

                        void* data;
                        vmaMapMemory(m_allocator, get_current_frame().view_buffer.allocation, &data);
                        memcpy(data, &buffer_data.camera_data, sizeof(SketchBook::Core::GPUCameraData));
                        vmaUnmapMemory(m_allocator, get_current_frame().view_buffer.allocation);
                    }

                    auto current_time = std::chrono::system_clock::now();
                    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>((current_time-m_time_last_frame));
                    m_current_scene->render(duration_ms.count()/1000.f);
                    m_time_last_frame = current_time;
                }
            }

            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

            // END DRAWING

            //finalize the render pass
             vkCmdEndRenderPass(cmd);
            //finalize the command buffer (we can no longer add commands, but it can now be executed)
            VK_CHECK(vkEndCommandBuffer(cmd));

            //prepare the submission to the queue.
            //we want to wait on the get_current_frame().present_semaphore, as that semaphore is signaled when the swapchain is ready
            //we will signal the get_current_frame().render_semaphore, to signal that rendering has finished

            VkSubmitInfo submit = {};
            submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit.pNext = nullptr;

            VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            submit.pWaitDstStageMask = &waitStage;

            submit.waitSemaphoreCount = 1;
            submit.pWaitSemaphores = &get_current_frame().present_semaphore;

            submit.signalSemaphoreCount = 1;
            submit.pSignalSemaphores = &get_current_frame().render_semaphore;

            submit.commandBufferCount = 1;
            submit.pCommandBuffers = &cmd;

            //submit command buffer to the queue and execute it.
            // get_current_frame().render_fence will now block until the graphic commands finish execution
            VK_CHECK(vkQueueSubmit(m_vk_init.graphics_queue, 1, &submit, get_current_frame().render_fence));

            // this will put the image we just rendered into the visible window.
            // we want to wait on the get_current_frame().render_semaphore for that,
            // as it's necessary that drawing commands have finished before the image is displayed to the user
            VkPresentInfoKHR presentInfo = {};
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pNext = nullptr;

            presentInfo.pSwapchains = &m_vk_init.swapchain;
            presentInfo.swapchainCount = 1;

            presentInfo.pWaitSemaphores = &get_current_frame().render_semaphore;
            presentInfo.waitSemaphoreCount = 1;

            presentInfo.pImageIndices = &swapchainImageIndex;

            VK_CHECK(vkQueuePresentKHR(m_vk_init.graphics_queue, &presentInfo));

            //increase the number of frames drawn
            m_frameNumber++;
        }
   };
}