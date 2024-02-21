#pragma once

#include <stay/ecs/component.hpp>

namespace stay
{
    struct PlayerDebug : public ecs::Component
    {
            PlayerDebug() = default;
            COMPONENT(PlayerDebug, foo);
        private:
            int foo{};
    };
} // namespace stay