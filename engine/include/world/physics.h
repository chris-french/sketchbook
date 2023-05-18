#pragma once
#include "spdlog/spdlog.h"
#include <glm/glm.hpp>
#include <cmath>
#include "./events.h"

namespace SketchBook
{
namespace World
{

    struct Limits
    {
         inline static double D_INFINITY = std::numeric_limits<double>::infinity();
    };
    

    enum DurationCardinality
    {
        Finite = 0,
        Infinite = 1
    };

    struct TickDuration
    {
        DurationCardinality cardinality;
        double created_at;
        double active_until;

        TickDuration(const double& tick, const double& active_ticks, const bool is_finite=true);

        double time_left(const double current_tick) const;
    };

    // Units
    // Time: in seconds
    // Distance: in meters

    struct SteeringOutput
    {
        glm::vec2 linear; // meters per second
        float heading_delta; // change in heading (degrees) per second in [-180, 180]

        SteeringOutput(const float x, const float y, const float in_angular_delta);
    };

    struct SteeringOutputHandle
    {
        TickDuration duration;
        SteeringOutput steering_output;
        bool b_wander;
        bool b_is_dynamic;

        SteeringOutputHandle(
            const float& x, 
            const float& y, 
            const float& in_angular_delta, 
            const double& tick, 
            const double& active_ticks, 
            const bool is_finite=true,
            const bool wander=false,
            const bool is_dynamic=false);
    };

    struct KinematicObject
    {
        KinematicObject(const float x, const float y);

        // Getters
        
        const float orientation_radians() const; // facing direction in radians
        const float get_heading() const; // facing direction in degrees [-180, 180] wrt 0=north
        const glm::vec2 velocity_meter_per_second() const;
        glm::vec3 get_position_in_render_space();
        glm::vec2 get_position() const { return m_position; }
        const std::vector<SteeringOutputHandle>& get_current_steering() const { return current_steering; }
        const float get_max_speed() const { return m_max_speed; }
        const float get_max_acceleration() const { return m_max_acceleration; }

        // Setters
        void set_max_speed(const float max_speed);
        void set_heading(const float heading);
        void update_heading(glm::vec2 velocity);
        void update_heading(const float& heading_delta);
        void add_steering(SteeringOutputHandle& handle, const double& in_current_tick);
        

       // World Update
       void update(const Event_NextTick& event);
       void halt();

    private:
        glm::vec2 m_position;
        float m_heading{0}; // degrees in [-180, 180]
        float m_angular_velocity{0}; // heading change per second
        glm::vec2 m_linear_velocity{0.f, 0.f}; // relative vel (meters per second)
        std::vector<SteeringOutputHandle> current_steering;
        float m_max_speed{1.f}; // meters per second
        float m_max_acceleration{5.f}; // m^2 per second

        void max_linear_velocity_guard(glm::vec2& velo);
        void apply_steering_output(const SteeringOutputHandle& handle, const Event_NextTick& event);
        void sort_steering(const double& in_current_tick);
        void update_steering(const Event_NextTick& event);
        void remove_steering(const SteeringOutputHandle& handle, const Event_NextTick& event);
    };

}
}