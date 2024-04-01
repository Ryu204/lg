#pragma once 

#include <stay/ecs/system.hpp>
#include <stay/utility/typedef.hpp>

namespace stay
{
    class Camera;
    struct DebugSystem 
        : public ecs::InputSystem
        , public ecs::System
    {   
            REGISTER_SYSTEM(DebugSystem)
            DebugSystem(ecs::Manager* manager);
            void input(const sf::Event& event) override;
    };
} // namespace stay