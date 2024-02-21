#pragma once

#include <stay/ecs/component.hpp>

namespace stay
{
    struct Dash : public ecs::Component
    {
            float velocity{};
            float length{};
            float cooldown{};
            float postBrake{};
            
            bool activated{false};
            bool canDash{true};
            bool left{false};
            
            COMPONENT(Dash, velocity, length, cooldown, postBrake);
    };
} // namespace stay