#pragma once

#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"
#include <functional>

#include "entt/signal/delegate.hpp"

namespace SketchBook
{
namespace Controller
{
    struct MouseButtonHandler
    {
        glm::vec2 offset {0.f, 0.f};
        glm::vec2 lst_xy {0.f, 0.f};
        bool btn_down{false};
        std::function<void(glm::vec2, glm::vec2)> on_release = [](glm::vec2, glm::vec2){};
        std::function<void(glm::vec2, glm::vec2)> on_held = [](glm::vec2, glm::vec2){};
        std::function<void(glm::vec2, glm::vec2)> on_clicked = [](glm::vec2, glm::vec2){};

        void handle_move_action(uint16_t mouse_x, uint16_t mouse_y)
        {
            if(btn_down /* MouseButtonData button down state */)
            {
                calculate_offset(mouse_x, mouse_y);
                on_held(offset, lst_xy);
            }
        }

        void handle_button_action(const bool is_pressed, uint16_t mouse_x, uint16_t mouse_y)
        {
            if (is_pressed)
            {
                if (!btn_down)
                {
                    btn_down = true;
                    lst_xy = {mouse_x, mouse_y};
                    offset = {0.f, 0.f};
                    calculate_offset(mouse_x, mouse_y);
                    on_clicked(offset, lst_xy);
                }
            }
            else
            {
                if (btn_down)
                {
                    btn_down = false;
                    calculate_offset(mouse_x, mouse_y);
                    on_release(offset, lst_xy);
                    lst_xy = {0.f, 0.f};
                    offset = {0.f, 0.f};
                }
            }
        }

    private:
        void calculate_offset(uint16_t mouse_x, uint16_t mouse_y)
        {
            offset.x = mouse_x - lst_xy.x;
            offset.y = lst_xy.y - mouse_y;
            lst_xy.x = mouse_x;
            lst_xy.y = mouse_y;
        };
    };
}
}
