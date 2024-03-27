#pragma once

#include <stay/ecs/component.hpp>
#include <stay/graphics/cameraController.hpp>

namespace stay
{
    struct CameraRegion : ecs::Component
    {
        CameraRegion(Rect bounds = Rect{})
            : bounds{std::move(bounds)}
        {}
        Rect bounds;
        COMPONENT(CameraRegion, bounds);
    };
} // namespace stay