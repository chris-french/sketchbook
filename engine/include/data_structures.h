#pragma once

#include <vector>
#include <functional>
#include <memory>
#include <string>

#include <spdlog/spdlog.h>

namespace SketchBook
{
namespace DataStructures
{
     struct DeletionQueue
    {
        // taken from the vulkan tutorial code, with some modifications
        std::vector<std::function<void()>> deletors;

        void push(std::function<void()>&& function) 
        {
            deletors.push_back(function);
        }

        void flush() 
        {
            for (auto it = deletors.rbegin(); it != deletors.rend(); it++) 
            {
                (*it)();
            }

            deletors.clear();
        }
    };

 
    // Retiring in favor of Entt
    // --------------------------------
    template<typename Base>
    class StringMap
    {
    public:
        using Value = std::unique_ptr<Base>;

        template<typename Derived>
        Derived* add_elem(std::string key)
        {
            data.emplace(key, std::make_unique<Derived>());
            return get_elem<Derived>(key);
        }

        template<typename Derived>
        Derived* add_elem(std::string key, const Derived& in_ref)
        {
            data.emplace(key, std::make_unique<Derived>(std::move(in_ref)));
            return get_elem<Derived>(key);
        }

        template<typename Derived>
        Derived* add_elem_with_lambda(const std::string key, std::function<void(Derived*& elem)> f)
        {
            if (Derived* ptr = add_elem<Derived>(key))
            {
                f(ptr);
            }
            return get_elem<Derived>(key);
        }

        template<typename Derived>
        Derived* get_elem(std::string key)
        {
            auto it = data.find(key);
            if (it == data.end()) {
                spdlog::warn("StringMap Warning (unable to find key={})", key);
                return nullptr;
            }
            return static_cast<Derived*>(it->second.get());
        }

        inline const bool is_empty() const
        {
            return data.empty();
        }

        StringMap()=default;
        ~StringMap()=default;

    private:
        std::unordered_map<std::string, Value> data;
    };
}
}