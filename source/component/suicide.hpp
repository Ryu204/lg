#pragma once

#include <stay/ecs/component.hpp>
#include <cstdint>

namespace stay
{
    struct Suicide : ecs::Component
    {
        Suicide(float countDown = 1.F, float scale = 1.5F, std::uint8_t alpha = 0)
            : countDown{countDown}
            , scale{scale}
            , alpha{alpha}
        {}

        float countDown;
        float scale;
        std::uint8_t alpha;

        float elapsed{};
        COMPONENT(Suicide, countDown, scale, alpha);
    };
} // namespace stay