#include "world/world.h"

namespace SketchBook
{
namespace World
{

    World::World()
        : 
            // defaults to one sec real time per on sec sim time
            m_milliseconds_per_tick(10.f), 
            m_sim_milliseconds_per_tick(10.f),
            m_tick(0), 
            b_is_running(false)
    {
        // invariants
        assert(m_milliseconds_per_tick > 0);
        assert(m_sim_milliseconds_per_tick > 0);
    }

    World::World(
        const int milliseconds_per_tick, 
        const int sim_milliseconds_per_tick, 
        const double tick) 
            : 
            m_milliseconds_per_tick(std::clamp<int>(milliseconds_per_tick, 0, 1000)),
            m_sim_milliseconds_per_tick(std::clamp<int>(sim_milliseconds_per_tick, 0, 1000)),
            m_tick(tick),
            b_is_running(false)
    {
        // invariants
        assert(m_milliseconds_per_tick > 0);
        assert(m_sim_milliseconds_per_tick > 0);
    }

    void World::step(const double tick)
    {
        // m_world.Step(m_sim_seconds_per_tick, velocity_iterations, position_iterations);

        m_event_dispatcher.trigger(
            Event_NextTick{
                tick,
                sim_seconds_per_tick()
            }
        );
    }

    void World::next_tick(const double tick)
    {
        std::scoped_lock guard(m_tick_mutex);
        m_tick.store(tick + 1.0);
    }

    void World::run()
    {
        {
            std::scoped_lock guard(m_tick_mutex);
            b_is_running = true;
        }

        while (b_is_running)
        {
            const double tick = current_tick();
            spdlog::info("beginning game tick: {}", tick);

            step(tick);
            
            std::this_thread::sleep_for(std::chrono::milliseconds(m_milliseconds_per_tick));
            spdlog::info("ending game tick: {}", tick);

            next_tick(tick);
        }
    }

    void World::pause()
    {
        std::scoped_lock lock(m_tick_mutex);
        b_is_running = false;
    }

    std::string World::current_time_string(const TimeType time_type) {
        using namespace std::chrono;

        const double tick = current_tick();
        auto per_tick_time = (time_type == TimeType::RealTime) ? m_milliseconds_per_tick : m_sim_milliseconds_per_tick;
        const float in_time = (float)tick * (float)per_tick_time;

        auto ms = duration_cast<milliseconds>(
            duration<double,std::milli>(in_time));

        auto secs = duration_cast<seconds>(ms);
        ms -= secs;

        auto mins = duration_cast<minutes>(secs);
        secs -= mins;

        auto h = duration_cast<hours>(mins);
        mins -= h;
        
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << h.count() << ':'
        << std::setfill('0') << std::setw(2) << mins.count() << ':'
        << std::setfill('0') << std::setw(2) << secs.count() << ':'
        << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    std::chrono::duration<float> World::current_time(const TimeType time_type) 
    {
        const double tick = current_tick();
        auto per_tick_time = (time_type == TimeType::RealTime) ? m_milliseconds_per_tick : m_sim_milliseconds_per_tick;
        const float in_time = (float)tick * (float)per_tick_time;
        return std::chrono::duration<float>(in_time);
    }



}
}