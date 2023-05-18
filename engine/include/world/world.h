#pragma once

#include <glm/glm.hpp>
#include <chrono>
#include <atomic>
#include <thread>
#include <mutex>

#include "entt/signal/sigh.hpp"
#include "entt/signal/dispatcher.hpp"

#include "events.h"
#include "physics.h"
#include "movement.h"

#include "box2d/box2d.h"
#include <reactphysics3d/reactphysics3d.h>

#include "spdlog/spdlog.h"


namespace SketchBook
{
namespace World
{

    struct WorldState
    {
        // box2d world
        const b2Vec2& m_box2d_gravity{0.0f, -10.0f};
        b2World m_box2d_world{m_box2d_gravity};
        int32 m_box2d_velocity_iterations{6};
        int32 m_box2d_position_iterations{2};
    };

    struct SimState
    {

    };


    struct WorldObject
    {
        void next_tick(const Event_NextTick& tick_event)
        {
            if (!b_is_active) return;
            if (f_on_next_tick_event)
            {
                f_on_next_tick_event(tick_event);
                ++m_tick_count_while_active;
            }
        }

        std::function<void(const Event_NextTick& tick_event)> f_on_next_tick_event = nullptr;
        const bool is_active() const { return b_is_active; }
        void set_active(const bool b_active) { b_is_active = b_active; }
    protected:
        double m_tick_count_while_active{0};
        bool b_is_active{false};
    };




    enum TimeType
    {
        RealTime = 0,
        SimTime
    };

    struct World
    {
        World();

        World(
            const int milliseconds_per_tick, 
            const int sim_milliseconds_per_tick, 
            const double tick = 0);

        std::string current_time_string(const TimeType time_type);
        std::chrono::duration<float> current_time(const TimeType time_type);
        entt::dispatcher& event_dispatcher() { return m_event_dispatcher; }

        void run();
        void pause();

        inline bool is_running() const { return b_is_running; }
        inline double current_tick() const { return m_tick.load(); }
        inline double total_sim_seconds() { return current_tick() * sim_seconds_per_tick(); }
        inline const float sim_seconds_per_tick() const { return static_cast<float>(m_sim_milliseconds_per_tick) / 1000.f; }

    protected:
        virtual void step(const double tick);
    
    private:

        void next_tick(const double tick);

        int m_milliseconds_per_tick;
        int m_sim_milliseconds_per_tick;
        std::atomic<double> m_tick;
        bool b_is_running;
        std::mutex m_tick_mutex;
        entt::dispatcher m_event_dispatcher{};
    };

    struct Simulation
    {
        glm::vec3 origin{0.f, 0.f, 0.f};

        Simulation() { init(); }
        ~Simulation() { join_tick_thread(); }

        std::string current_time_string() { return m_world.current_time_string(TimeType::RealTime); }
        std::string current_sim_time_string() {return m_world.current_time_string(TimeType::SimTime); }

        std::chrono::duration<float> current_time() { return m_world.current_time(TimeType::RealTime); }
        std::chrono::duration<float> current_sim_time() { return m_world.current_time(TimeType::SimTime); }

        template<typename WS> 
        WS* get_world_state() { return static_cast<WS*>(m_world_state.get()); }

        template<typename SS>
        SS* get_sim_state() { return static_cast<SS*>(m_sim_state.get()); }

        void start()
        {
            spdlog::info("starting sim");
            join_tick_thread();
            _counter_thread = std::thread(&World::run, &m_world);
        }

        void pause()
        {
            spdlog::info("pausing sim");
            m_world.pause();
            join_tick_thread();
        }

        const float seconds_per_tick() const { return m_world.sim_seconds_per_tick();}
        const double current_tick() const {return m_world.current_tick(); }
        const bool is_paused() const { return m_world.is_running(); }

        void connect_world_object(WorldObject* obj)
        {
            m_world.event_dispatcher().sink<Event_NextTick>().connect<&WorldObject::next_tick>(obj);
        }

        void disconnect_world_object(WorldObject* obj)
        {
            m_world.event_dispatcher().sink<Event_NextTick>().disconnect<&WorldObject::next_tick>(obj);
        }
        
    protected:

        virtual void init()
        {
            m_world_state = std::make_shared<WorldState>();
            m_sim_state = std::make_shared<SimState>();
        }

    private:

        void join_tick_thread()
        {
            if (_counter_thread.joinable()) _counter_thread.join();
        }

        std::shared_ptr<WorldState> m_world_state = nullptr;
        std::shared_ptr<SimState> m_sim_state = nullptr;

        World m_world;
        std::thread _counter_thread;
    };

}
}