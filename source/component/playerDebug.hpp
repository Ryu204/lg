#pragma once

#include <stay/ecs/component.hpp>

namespace stay
{
    // For testing purposes
    struct PlayerDebug : public ecs::Component
    {
            COMPONENT(PlayerDebug, points);
            std::vector<Vector2> points{};
    };
} // namespace stay