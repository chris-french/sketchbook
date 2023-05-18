#pragma once
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#include <sk_types.h>
#include <camera.h>
#include <sk_engine.h>
#include <vector>

static float default_yaw = -90.f;
static float default_pitch = 0.f;
static float default_x = 1.f;
static float default_y = 1.f;
static float default_h = 0.f;

namespace VulkanTutorial
{
    using SketchBook::Engine;
    using SketchBook::Components::Scene;

    class RGB_Triangle;
    class Red_Triangle;
    class MeshScene3D;
    class MonkeyMeshScene3D;

    static std::string  default_2d_pipeline_layout_key      {"pipeline_layout.default.2d"};
    static std::string  default_3d_pipeline_layout_key      {"pipeline_layout.default.3d"};
    static std::string  pipeline_triangle_key               {"pipeline.triangle"};
    static std::string  pipeline_red_triangle_key           {"pipeline.red_triangle"};
    static std::string  pipeline_mesh_key                   {"pipeline.mesh"};
    static std::string  pipeline_infinite_grid              {"pipeline.infinite_grid"};
    static std::string  mesh_monkey_key                     {"mesh.monkey"};
    static std::string  mesh_triangle_key                   {"mesh.triangle"};
    static std::string  wedge_triangle_key                  {"wedge.triangle"};
    static std::string  material_mesh_key                   {"material.mesh"};

    class TutorialEngine : public Engine<TutorialEngine>{
        
    public:
        

        TutorialEngine(int width, int height) 
            : Engine<TutorialEngine>(width, height)
        {
        }

        int m_selectedShader {0};
        
        
		void setup() override
		{
			init_vulkan();
			init_swapchain();
			init_commands();
			init_default_renderpass();
			init_framebuffers();
			init_sync_structures();
            init_descriptors();

			spdlog::info("Vulkan Init Done");

			init_imgui();
            init_pipelines();
			load_meshes();
			init_scenes();

            spdlog::info("Graphics Init Done");

			//everything went fine
			m_isInitialized = true;

			spdlog::info("Setup complete");
		}


        void tool_imgui() override
        {
            imgui_current_camera_window();
        }

    protected:
        void init_scenes() override
        {
			spdlog::info("Loading Tutorial Scenes");

            create_material(material_mesh_key, pipeline_mesh_key, default_3d_pipeline_layout_key);

            create_scene("rgb_triangle", [=](Scene* scene) mutable {
                scene->b_should_render = true;
                scene->render = [=](const float delta){
                    VkCommandBuffer& cmd = get_current_frame().main_command_buffer;
                    SketchBook::Components::Pipeline* pipeline = get_pipeline(pipeline_triangle_key);
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                    vkCmdDraw(cmd, 3, 1, 0, 0);
                };
             });

            create_scene("red_triangle", [=](Scene* scene) mutable {
                scene->b_should_render = true;
                scene->render = [=](const float delta){
                    VkCommandBuffer& cmd = get_current_frame().main_command_buffer;
                    SketchBook::Components::Pipeline* pipeline = get_pipeline(pipeline_red_triangle_key);
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                    vkCmdDraw(cmd, 3, 1, 0, 0);
                };
             });

            create_scene("mesh_scene", [=](Scene* scene) mutable {
                scene->b_should_render = true;
                scene->create_camera();
                scene->current_camera()->init(get_window().windowExtent.width, get_window().windowExtent.height);
                //scene->current_camera()->m_position = {default_x, default_h, default_y};
                //scene->current_camera()->set_cameraup(default_yaw, default_pitch);


                RenderObject obj;
                obj.material_name = material_mesh_key;
                obj.mesh_name = mesh_triangle_key;

                obj.render = [=](const float delta) mutable {
                    MeshPushConstants constants;
                    constants.normal_color = {0.f, 1.f, 0.f, 0.f};
                    constants.render_matrix = glm::mat4(1.f);

                    VkCommandBuffer& cmd = get_current_frame().main_command_buffer;

                    SketchBook::Components::Material* material = get_material(obj.material_name);
                    SketchBook::Components::Pipeline* pipeline = get_pipeline(material->pipeline_name);
                    SketchBook::Components::PipelineLayout* pipeline_layout = get_pipeline_layout(material->pipeline_layout_name);
                    SketchBook::Components::Mesh* mesh = get_mesh(obj.mesh_name);
                
                    VkDeviceSize offset2 = 0;
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                    vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->mesh.vertex_buffer.buffer, &offset2);
                    vkCmdPushConstants(cmd, pipeline_layout->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->pipeline_layout, 0, 1, &get_current_frame().global_descriptor, 0, nullptr);
                    vkCmdDraw(cmd, mesh->mesh.vertices.size(), 1, 0, 0);
                };

                scene->render_objects.push_back(obj);

                scene->render = [=](const float delta) mutable {
                    for (auto &obj : scene->render_objects)
                    {
                        obj.render(delta);
                    }
                };
             });

            create_scene("monkey_scene", [=](Scene* scene) mutable {
                scene->b_should_render = true;
                scene->create_camera();
                scene->current_camera()->init(get_window().windowExtent.width, get_window().windowExtent.height);
                
                scene->render = [&](const float delta){
                    MeshPushConstants constants;
                    constants.normal_color = {0.f, 0.f, 1.f, 0.f};
                    constants.render_matrix = glm::translate(glm::mat4(1.f), glm::vec3(0));

                    VkCommandBuffer& cmd = get_current_frame().main_command_buffer;

                    SketchBook::Components::Material* material = get_material(material_mesh_key);
                    SketchBook::Components::Pipeline* pipeline = get_pipeline(material->pipeline_name);
                    SketchBook::Components::PipelineLayout* pipeline_layout = get_pipeline_layout(material->pipeline_layout_name);
                    SketchBook::Components::Mesh* mesh = get_mesh(mesh_monkey_key);
                
                    VkDeviceSize offset2 = 0;
                    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                    vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->mesh.vertex_buffer.buffer, &offset2);
                    vkCmdPushConstants(cmd, pipeline_layout->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
                    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->pipeline_layout, 0, 1, &get_current_frame().global_descriptor, 0, nullptr);
                    vkCmdDraw(cmd, mesh->mesh.vertices.size(), 1, 0, 0);
                };
            });


            create_scene("wedge_scene", [=](Scene* scene) mutable {
                scene->b_should_render = true;
                scene->create_camera();
                scene->current_camera()->init(get_window().windowExtent.width, get_window().windowExtent.height);
                
                scene->render = [=](const float delta){

                    VkCommandBuffer& cmd = get_current_frame().main_command_buffer;

                    SketchBook::Components::Material* material = get_material(material_mesh_key);
                    SketchBook::Components::Pipeline* pipeline = get_pipeline(material->pipeline_name);
                    SketchBook::Components::PipelineLayout* pipeline_layout = get_pipeline_layout(material->pipeline_layout_name);
                    SketchBook::Components::Mesh* mesh = get_mesh(wedge_triangle_key);
                
                    float _x = 0.f, _y = 0.f, _z = 0.f;
                    float d = 1.f;
                    for (int i = 0; i < 4; i++)
                    {
                        VkDeviceSize offset = 0;
                
                        switch(i)
                        {
                            case 0:
                                _x = d;
                                _z = d;
                                break;
                            case 1:
                                _x = -d;
                                _z = d;
                                break;
                            case 2:
                                _x = d;
                                _z = -d;
                                break;
                            case 3:
                                _x = -d;
                                _z = -d;
                                break;
                        }                
                        auto pos = glm::vec3{_x,_y,_z};
                        MeshPushConstants constants;
                        constants.normal_color = {0.f, 0.f, 1.f, 1.f};
                        constants.render_matrix = glm::translate(glm::mat4(1.f), pos);

                        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
                        vkCmdBindVertexBuffers(cmd, 0, 1, &mesh->mesh.vertex_buffer.buffer, &offset);
                        vkCmdPushConstants(cmd, pipeline_layout->pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);
                        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout->pipeline_layout, 0, 1, &get_current_frame().global_descriptor, 0, nullptr);            
                        vkCmdDraw(cmd, mesh->mesh.vertices.size(), 1, 0, 0);
                    };
                };
            });

			set_scene("rgb_triangle");
            m_selectedShader = 0;

            spdlog::info("Loaded tutorial scenes");
        }

        void custom_input_handler(SDL_Event* e) override
        {
            if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_SPACE)
            {
                m_selectedShader += 1;
                if(m_selectedShader > 4)
                {
                    m_selectedShader = 0;
                }
                switch(m_selectedShader)
                {
                    case 0:
                        set_scene("rgb_triangle");
                        break;
                    case 1:
                        set_scene("red_triangle");
                        break;
                    case 2:
                        set_scene("mesh_scene");
                        break;
                    case 3:
                        set_scene("monkey_scene");
                        break;
                    case 4:
                        set_scene("wedge_scene");
                        break;
                }
            }
        }

    void load_meshes()
    {
        spdlog::info("Loading meshes");

        create_mesh_from_file(mesh_monkey_key, "assets/monkey_smooth.obj");
        create_mesh_from_file(wedge_triangle_key, "assets/wedge.obj");

        create_mesh(mesh_triangle_key, [](Vertex_RBG_Normal_Mesh* mesh){
            mesh->vertices.resize(6);

            //vertex positions
            mesh->vertices[0].position = { 1.f, 1.f, 1.0f };
            mesh->vertices[1].position = { 0.f,-1.f, 1.0f };
            mesh->vertices[2].position = { 0.f, 0.f, 0.0f };
            mesh->vertices[3].position = { 0.f, 0.f, 0.0f };
            mesh->vertices[4].position = {-.5f, 0.f, 1.0f };
            mesh->vertices[5].position = {-1.f, -1.f, 1.0f };

            //vertex colors, all green
            mesh->vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
            mesh->vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
            mesh->vertices[2].color = { 1.f, 1.f, 0.0f }; 
            mesh->vertices[3].color = { 0.f, 1.f, 0.0f }; //pure green
            mesh->vertices[4].color = { 1.f, 1.f, 0.0f }; 
            mesh->vertices[5].color = { 0.f, 1.f, 0.0f }; //pure green
        });
    }

    void init_pipelines()
    {
		spdlog::info("Loading Shaders");
        VkShaderModule triangleFragShader;
        VkShaderModule triangleVertexShader;
        VkShaderModule redTriangleFragShader;
        VkShaderModule redTriangleVertexShader;
        VkShaderModule mesh_descriptor_vertex_shader;

        LOAD_SHADER_MODULE("shaders/colored_triangle.frag.spv", triangleFragShader);
        LOAD_SHADER_MODULE("shaders/colored_triangle.vert.spv", triangleVertexShader);
        LOAD_SHADER_MODULE("shaders/triangle.frag.spv", redTriangleFragShader);
        LOAD_SHADER_MODULE("shaders/triangle.vert.spv", redTriangleVertexShader);
        LOAD_SHADER_MODULE("shaders/mesh_descriptor.vert.spv", mesh_descriptor_vertex_shader);

	   spdlog::info("Loading Pipeline Layouts");

        create_pipeline_layout(default_2d_pipeline_layout_key);

        create_pipeline_layout(default_3d_pipeline_layout_key, [=](VkPipelineLayoutCreateInfo& info) mutable {

            info.pPushConstantRanges = &m_push_constant;
            info.pushConstantRangeCount = 1;

            info.setLayoutCount = 1;
            info.pSetLayouts = &m_global_set_layout;
        });

        spdlog::info("Loading Pipelines");

        create_pipeline(pipeline_triangle_key, default_2d_pipeline_layout_key, triangleVertexShader, triangleFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL);

        create_pipeline(pipeline_red_triangle_key, default_2d_pipeline_layout_key, redTriangleVertexShader, redTriangleFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL);

        create_pipeline(pipeline_mesh_key, default_3d_pipeline_layout_key, mesh_descriptor_vertex_shader, triangleFragShader, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_POLYGON_MODE_FILL,
            [=](SketchBook::Core::PipelineBuilder& builder) mutable {
                
                //connect the pipeline builder vertex input info to the one we get from Vertex
                builder._vertexInputInfo.pVertexAttributeDescriptions = m_vertex_description.attributes.data();
                builder._vertexInputInfo.vertexAttributeDescriptionCount = m_vertex_description.attributes.size();

                builder._vertexInputInfo.pVertexBindingDescriptions = m_vertex_description.bindings.data();
                builder._vertexInputInfo.vertexBindingDescriptionCount = m_vertex_description.bindings.size();
        });

        spdlog::info("Cleanup Shaders");

        vkDestroyShaderModule(vk_init().device, redTriangleVertexShader, nullptr);
        vkDestroyShaderModule(vk_init().device, redTriangleFragShader, nullptr);
        vkDestroyShaderModule(vk_init().device, triangleFragShader, nullptr);
        vkDestroyShaderModule(vk_init().device, triangleVertexShader, nullptr);
        vkDestroyShaderModule(vk_init().device, mesh_descriptor_vertex_shader, nullptr);
    }
};
};
