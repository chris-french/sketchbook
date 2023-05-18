#pragma once

#include "controllers.h"
#include <entt/entt.hpp>
#include <SDL.h>

namespace SketchBook
{
namespace Components
{
    const glm::vec3 UP(0.0f, 1.0f, 0.0f);
    const glm::vec3 DOWN(0.0f, -1.0f, 0.0f);
    const glm::vec3 RIGHT(1.0f, 0.f, 0.f);
    const glm::vec3 FORWARD(0.f, 0.f, 1.0f);
    const glm::vec3 CAMERA_FRONT(0.f, 0.f, -1.0f);

    struct FreeCamera
    {
        std::string name {"default"};

        bool b_inverse_mode{false};

        glm::vec2 m_focus_position{0.f, 0.f};
        glm::vec3 m_position{0.f, 0.f, 5.f};
        glm::mat4 m_projection = glm::mat4(1.f);

        float m_yaw = -90.f; // the magnitude we're looking to the left or to the right
        float m_pitch = 0.f; // the angle that depicts how much we're looking up or down as seen in the first image
        float m_sensitivity = 0.5f;
        float m_rotation_speed = 0.1f;
        float m_fovy_deg = 70.f;
        float m_aspect;
        float m_near = .1f;
        float m_far = 200.f;

        FreeCamera();

        virtual void init(uint32_t width, uint32_t height);

        void change_aspect(uint32_t width, uint32_t height);
        glm::mat4 get_render_matrix(const bool use_model = true) const;
        void set_cameraup(float in_yaw = -90.f, float in_pitch = 0.f);

        inline glm::vec3 camera_front() const { return glm::normalize(inverse_direction()); }

        inline glm::vec3 camera_center() const { return m_position + camera_front(); }

        inline glm::vec3 camera_up() const { return glm::cross(inverse_direction(), camera_right()); }

        inline glm::vec3 camera_right() const { return glm::normalize(glm::cross((b_inverse_mode) ? DOWN : UP, inverse_direction())); }

        inline glm::mat4 model() const { return glm::mat4(1.0f);}

        inline glm::mat4 projection() const { return m_projection;}

        inline glm::mat4 view() const { return glm::lookAt(m_position, camera_center(), camera_up()); }

        glm::vec3 inverse_direction() const;
        
        void event_handler(SDL_Event* e, const bool handle_mouse, const bool handle_keys);

        void update_camera_up(const float yaw_delta, const float pitch_delta);

        Controller::MouseButtonHandler left_mouse_btn;
        Controller::MouseButtonHandler middle_mouse_btn;
        Controller::MouseButtonHandler right_mouse_btn;

        void on_right_mouse_clicked(glm::vec2 offset, glm::vec2 lst_xy);

    private:
        void calibrate_pitch();
        void calculate_projection();
        void on_mouse_down_event_handler(SDL_Event* e);
        void on_mouse_up_event_handler(SDL_Event* e);
        void on_mouse_wheel_event_handler(SDL_Event* e);
        void on_mouse_move_event_handler(SDL_Event* e);
    };

}
}


