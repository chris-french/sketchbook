#pragma once
#include <entt/entt.hpp>
#include <functional>
#include "registry.h"
#include "camera.h"
#include "glm/mat4x4.hpp"
#include "imgui.h"
#include "world/world.h"
#include <memory>

namespace SketchBook
{

void PrintGlmMat4(const std::string name, const glm::mat4 m)
{
    ImGui::Text(name.c_str());
    if(ImGui::BeginTable("mat4", 4))
    {
        for (int r = 0; r < 4; r++)
        {
            for (int c = 0; c < 4; c++)
            {
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", m[r][c]);
            }
        }
        ImGui::EndTable();
    }
}


namespace Components
{
    struct Scene
    {   
        ~Scene()
        {
            if (m_sim)
            {
                m_sim->pause();
            }
        }

        std::string name;
        bool b_should_render{false};
        std::shared_ptr<SketchBook::World::Simulation> m_sim = nullptr;
        std::vector<RenderObject> render_objects;
        std::vector<FreeCamera> free_cameras;
        int free_camera_index{-1};

        std::function<void(const float delta)> render = nullptr;

        FreeCamera* current_camera()
        {
            if (free_cameras.size() > 0 && (free_camera_index > -1))
            {
                return &free_cameras[free_camera_index];
            }
            return nullptr;
        }

        void create_camera(const bool set_to_current = true)
        {
            FreeCamera camera;
            free_cameras.push_back(camera);
            if (set_to_current)
            {
                free_camera_index = free_cameras.size()-1;
            }
        }
    };
}
}


