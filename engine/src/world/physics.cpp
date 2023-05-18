#include "world/physics.h"
#include "world/random.h"

#include <algorithm>
#include <cmath>

namespace SketchBook
{
namespace World
{
    TickDuration::TickDuration(const double& tick, const double& active_ticks, const bool is_finite)
        :   cardinality{(is_finite) ? DurationCardinality::Finite : DurationCardinality::Infinite},
            created_at{tick},
            active_until{active_ticks}
    {}

    double TickDuration::time_left(const double current_tick) const
    {
        switch(cardinality)
        {
            case DurationCardinality::Infinite:
                return Limits::D_INFINITY;
            case DurationCardinality::Finite:
                return (created_at + active_until) - current_tick;
            default:
                throw std::exception("time_left invalid duration cardinality enum");
        }
    }

    SteeringOutput::SteeringOutput(const float x, const float y, const float in_angular_delta)
        : linear{x,y}, heading_delta{in_angular_delta}
    {
    }

    SteeringOutputHandle::SteeringOutputHandle(
        const float& x, 
        const float& y, 
        const float& in_angular_delta, 
        const double& tick, 
        const double& active_ticks, 
        const bool is_finite,
        const bool wander,
        const bool is_dynamic)
        :   duration(tick, active_ticks, is_finite), 
            steering_output(x, y, in_angular_delta),
            b_wander{wander}, b_is_dynamic{is_dynamic}
    {}

    KinematicObject::KinematicObject(const float x, const float y)
        : m_position{x, y}
    {
    }

    const glm::vec2 KinematicObject::velocity_meter_per_second() const 
    { 
        return m_linear_velocity; 
    }

    const float KinematicObject::orientation_radians() const 
    { 
        return glm::radians(m_heading); 
    }

    const float KinematicObject::get_heading() const 
    { 
        return m_heading; 
    }

    void KinematicObject::set_max_speed(const float max_speed)
    {
        m_max_speed = std::max(max_speed, 0.f);
    }

    void KinematicObject::set_heading(const float heading)
    {
        float h = fmod(heading, 360);
        if (h > 180) h -= 360;
        if (h < -180) h += 360;
        m_heading = h;
    }

    void KinematicObject::update(const Event_NextTick& event)
    {
        update_steering(event);
        for (SteeringOutputHandle& h : current_steering)
            apply_steering_output(h, event);

        m_position += m_linear_velocity * event.seconds_per_tick;
        m_heading += m_angular_velocity * event.seconds_per_tick;
    }

    glm::vec3 KinematicObject::get_position_in_render_space() 
    { 
        return glm::vec3(m_position.x, -0.1f, m_position.y); 
    }

    void KinematicObject::update_heading(glm::vec2 velocity)
    {
        const float new_orientation = atan2(-velocity.x, velocity.y);
        set_heading(glm::degrees(new_orientation)-90);
    }

    void KinematicObject::update_heading(const float& heading_delta)
    {
        set_heading(get_heading() + heading_delta);
    }

    void KinematicObject::add_steering(SteeringOutputHandle& handle, const double& in_current_tick) 
    {
        if (handle.b_is_dynamic)
        {
        }
        else
        {
            m_linear_velocity += handle.steering_output.linear;
            m_angular_velocity += handle.steering_output.heading_delta;
            max_linear_velocity_guard(m_linear_velocity);
        }
        
        current_steering.push_back(handle);
        sort_steering(in_current_tick);
    }

    void KinematicObject::apply_steering_output(const SteeringOutputHandle& handle, const Event_NextTick& event)
    {
        //todo: pass in event to handle, and update duration if event.seconds_per_tick has changed
        // or using entt signal to do that

        float half_t_sqr = 0.5 * event.seconds_per_tick * event.seconds_per_tick;

        m_position += handle.steering_output.linear * half_t_sqr;
        m_heading += handle.steering_output.heading_delta * half_t_sqr;

        if (handle.b_wander)
        {
            const float _rand = SketchBook::World::Random::binom(1, 0.5);
            m_heading *= _rand * std::clamp(handle.steering_output.heading_delta / 180, -1.0f, 1.0f);
        }

        if (handle.b_is_dynamic)
        {
            m_linear_velocity += handle.steering_output.linear * event.seconds_per_tick;
            m_angular_velocity += handle.steering_output.heading_delta * event.seconds_per_tick;
        }
    }

    void KinematicObject::halt()
    {
        m_linear_velocity = glm::vec2(0.f);
        m_angular_velocity = 0.f;
    }

    void KinematicObject::max_linear_velocity_guard(glm::vec2& velo)
    {
        if (glm::length(velo) > m_max_speed)
        {
            velo = glm::normalize(velo);
            velo *= m_max_speed;
        }
    }

    void KinematicObject::sort_steering(const double& in_current_tick)
    {
        std::sort(current_steering.begin(), current_steering.end(), [&](const SteeringOutputHandle&h1, const SteeringOutputHandle& h2){
            return h1.duration.time_left(in_current_tick) < h2.duration.time_left(in_current_tick);
        });
    }

    void KinematicObject::update_steering(const Event_NextTick& event)
    {
        current_steering.erase(std::remove_if(current_steering.begin(), current_steering.end(), [&, this](const SteeringOutputHandle& h){
            const bool b_remove = h.duration.time_left(event.tick_count) <= 0;
            if (b_remove)
            {
                remove_steering(h, event);
            }
            return b_remove;
        }), current_steering.end());
    }

    void KinematicObject::remove_steering(const SteeringOutputHandle& handle, const Event_NextTick& event)
    {
        if (handle.b_is_dynamic)
        {
            const float total_ticks = static_cast<float>(handle.duration.active_until);
            const glm::vec2 velo_delta = total_ticks * handle.steering_output.linear * event.seconds_per_tick;
            m_linear_velocity -= velo_delta;
        }
        else
        {
            m_linear_velocity -= handle.steering_output.linear;
            m_angular_velocity -= handle.steering_output.heading_delta;
        }
    }
        
        


}
}