#pragma once
#include <cmath>
#include "physics.h"

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
//#include <glm/gtx/vector_angle.hpp>

namespace SketchBook
{
namespace World
{
namespace Movement
{

    class KinematicMovement
    {
    public:
        static float new_orientation(float orientation, glm::vec2 velocity)
        {
            return (velocity.length() > 0) ? atan2(-velocity.x, velocity.y) : orientation;
        }

        static SteeringOutputHandle uniform_speed_move_to(
            const KinematicObject& object, 
            const glm::vec2 target, 
            const float speed /* meters per second */, 
            const double tick,
            const float seconds_per_tick,
            const bool update_heading = false,
            const bool wander = false,
            const float meter_length = 1)
        {
            // const float unit_meter_distance = glm::distance(glm::vec2(0, 0), glm::vec2(meter_length, meter_length));
            const float meters_per_second = meter_length * std::clamp(speed, 0.f, object.get_max_speed());
            const glm::vec2 velocity_eigenvector = glm::normalize(target - object.get_position());
            const glm::vec2 result_velocity = velocity_eigenvector * meters_per_second;
            const float meters_per_tick = meters_per_second * seconds_per_tick;
            glm::vec2 result_velocity_per_tick = velocity_eigenvector * meters_per_tick; 

            const float distance_to_target_meters = glm::distance(target, object.get_position()) * meter_length;
            const float distance_to_target_meters_per_tick = glm::length(result_velocity_per_tick);
            const double total_ticks = std::ceil(distance_to_target_meters / distance_to_target_meters_per_tick); //distance_to_target_meters_per_tick);

            const float orientation = object.orientation_radians();
            float desired_orientation = object.orientation_radians();
            if (update_heading)
            {
                const glm::vec2 object_heading_vec = {cos(orientation), sin(orientation)};
                const glm::vec2 delta_vec = target - object_heading_vec;
                desired_orientation = atan2(delta_vec.y, delta_vec.x);
            }

            float degree_delta = glm::degrees(desired_orientation - orientation);

            SteeringOutputHandle handle(
                result_velocity.x,
                result_velocity.y,
                degree_delta,
                tick,
                total_ticks,
                true,
                wander,
                false);

            return handle;
        }

        // idea: implement a non-uniform move-to by splitting up move-to
        // to constant bands of of monotonically decresing/increasing move-to orders

    };

    class DynamicMovement
    {
    public:

        static double calculate_t(float a, float m) {
            return (m - a) / a;
        }

        static SteeringOutputHandle move_to(
            const KinematicObject& object, 
            const glm::vec2 target, 
            const float acceleration /* m^2 per second */, 
            const double tick,
            const float seconds_per_tick,
            const float meter_length = 1)
        {
            // const float unit_meter_distance = glm::distance(glm::vec2(0, 0), glm::vec2(meter_length, meter_length));
            const float meters_per_second = meter_length * std::clamp(acceleration, 0.f, object.get_max_acceleration());
            const float meters_per_tick = meters_per_second * seconds_per_tick;
            const glm::vec2 eigen_vector = glm::normalize(target - object.get_position());
            glm::vec2 result_acceleration = eigen_vector * meters_per_second;
            glm::vec2 result_acceleration_per_tick = eigen_vector * meters_per_tick; 

            if (meters_per_second  * object.get_max_speed())
            {

            }

            //const float max_distance = ;

            const float distance_to_target_meters = glm::distance(target, object.get_position()) * meter_length;
            const float distance_squared_to_target_meters_per_tick = glm::length(result_acceleration_per_tick); // * meters_per_tick;  /*unit_meter_distance * speed * seconds_per_tick; */
            const double total_ticks = std::ceil(distance_to_target_meters / distance_squared_to_target_meters_per_tick);

            SteeringOutputHandle handle(
                result_acceleration.x,
                result_acceleration.y,
                0,
                tick,
                total_ticks,
                true,
                false,
                true);

            return handle;
        }
    };
}
}
}