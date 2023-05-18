#pragma once

#include <entt/entt.hpp>
#include <sk_types.h>
#include <functional>

namespace SketchBook
{
    class Registry
    {
    protected:
        entt::registry m_assets;
        entt::registry m_objects;

    public:
        entt::entity create_asset(std::function<void(entt::registry& r, const entt::entity &entity)> asset_callback)
        {
            auto entity = m_assets.create();
            asset_callback(m_assets, entity);
            return entity;
        }   

        entt::entity create_object(std::function<void(entt::registry& r, const entt::entity &entity)> object_callback)
        {
            auto entity = m_objects.create();
            object_callback(m_objects, entity);
            return entity;
        }      

        entt::registry& get_assets() { return m_assets; }
        entt::registry& get_objects() { return m_objects; }
    };
}

