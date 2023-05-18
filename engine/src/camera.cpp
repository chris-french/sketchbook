#include "camera.h"
#include "spdlog/spdlog.h"

namespace SketchBook
{
namespace Components
{
    FreeCamera::FreeCamera()
    {
        calculate_projection();
    }

    void FreeCamera::on_right_mouse_clicked(glm::vec2 offset, glm::vec2 lst_xy)
    {
        const float x_offset = offset.x;
        const float y_offset = offset.y;
        const float yaw_delta = x_offset * m_sensitivity;
        const float pitch_delta = y_offset * m_sensitivity;
        update_camera_up(yaw_delta, pitch_delta);
    }

    void FreeCamera::init(uint32_t width, uint32_t height)
    {
        change_aspect(width, height);
        right_mouse_btn.on_held = std::bind(&FreeCamera::on_right_mouse_clicked, this, std::placeholders::_1, std::placeholders::_2);
        right_mouse_btn.on_release = std::bind(&FreeCamera::on_right_mouse_clicked, this, std::placeholders::_1, std::placeholders::_2);
    }

    void FreeCamera::change_aspect(uint32_t width, uint32_t height)
    {
        m_aspect = static_cast<float>(width)/static_cast<float>(height);
        calculate_projection();
    }

    glm::mat4 FreeCamera::get_render_matrix(const bool use_model) const
    {
        //spdlog::info("render matrix called in camera");
        return (use_model) ? (projection() * view() * model()) : (projection() * view());
    }

    void FreeCamera::set_cameraup(float in_yaw, float in_pitch)
    {
        m_yaw = in_yaw;
        m_pitch = in_pitch;
        calibrate_pitch();
    }

    void FreeCamera::update_camera_up(const float yaw_delta, const float pitch_delta)
    {
        m_yaw += (b_inverse_mode) ? -yaw_delta : yaw_delta;
        m_pitch += (b_inverse_mode) ? -pitch_delta : -pitch_delta;
        calibrate_pitch();
    }

    void FreeCamera::calibrate_pitch()
    {
        if (m_pitch < -90.f) m_pitch = -89.5f;
        if (m_pitch > 90.f) m_pitch = 89.5f;
    }

    glm::vec3 FreeCamera::inverse_direction() const
    {
        return glm::vec3 {
            cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)),
            sin(glm::radians(m_pitch)),
            sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
        };
    }

    void FreeCamera::calculate_projection()
    {
        m_projection = glm::perspective(glm::radians(m_fovy_deg), m_aspect, m_near, m_far);
        m_projection[1][1] *= -1;
    }

    void FreeCamera::event_handler(SDL_Event* e, const bool handle_mouse, const bool handle_keys)
    {
        if (handle_mouse)
        {
            switch(e->type)
            {
                case SDL_MOUSEMOTION:
                    on_mouse_move_event_handler(e);
                    break;
                case SDL_MOUSEWHEEL:
                    on_mouse_wheel_event_handler(e);
                    break;
                case SDL_MOUSEBUTTONUP:
                    on_mouse_up_event_handler(e);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    on_mouse_down_event_handler(e);
                    break;
                default:
                    break;
            }
        }

        if (handle_keys)
        {
            const Uint8* kb = SDL_GetKeyboardState(NULL);
            const glm::vec3 c_front = camera_front();
            m_position += glm::normalize(glm::cross(c_front, camera_right())) * ((kb[SDL_SCANCODE_W] - kb[SDL_SCANCODE_S]) * m_rotation_speed);
            m_position += glm::normalize(glm::cross(c_front, camera_up())) * ((-kb[SDL_SCANCODE_A] + kb[SDL_SCANCODE_D]) * m_rotation_speed);
        }

    }

    void FreeCamera::on_mouse_down_event_handler(SDL_Event* e)
    {
        //spdlog::info("mouse down signal");
        //std::cout << "mouse down" << std::endl;
        auto state = e->button.state;
        auto button = e->button.button;
        const uint16_t x = e->button.x, y = e->button.y;
        const bool is_pressed = {state == SDL_PRESSED};
        switch(button)
        {
            case SDL_BUTTON_LEFT:
                left_mouse_btn.handle_button_action(is_pressed, x, y);
                break;
            case SDL_BUTTON_MIDDLE:
                middle_mouse_btn.handle_button_action(is_pressed,x, y);
                break;
            case SDL_BUTTON_RIGHT:
                right_mouse_btn.handle_button_action(is_pressed, x,y);
                break;
        }
    }

    void FreeCamera::on_mouse_up_event_handler(SDL_Event* e)
    {
        //spdlog::info("mouse up signal");
        //std::cout << "mouse up" << std::endl;
        const uint16_t x = e->button.x, y = e->button.y;
        auto button = e->button.button;
        switch(button)
        {
            case SDL_BUTTON_LEFT:
                left_mouse_btn.handle_button_action(false, x, y);
                break;
            case SDL_BUTTON_MIDDLE:
                middle_mouse_btn.handle_button_action(false, x, y);
                break;
            case SDL_BUTTON_RIGHT:
                right_mouse_btn.handle_button_action(false, x, y);
                break;
        }
    }

    void FreeCamera::on_mouse_wheel_event_handler(SDL_Event* e)
    {
        const int _sign = (e->wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1;
        m_position += camera_front() * (_sign * e->wheel.preciseY * m_rotation_speed);
    }

    void FreeCamera::on_mouse_move_event_handler(SDL_Event* e)
    {
        //std::cout << "mouse motion" << std::endl;
        const uint16_t x = e->button.x, y = e->button.y;
        m_focus_position.x = x;
        m_focus_position.y = y;

        left_mouse_btn.handle_move_action(x,y);
        middle_mouse_btn.handle_move_action(x,y);
        right_mouse_btn.handle_move_action(x,y);
    }


}
}