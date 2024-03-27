#pragma once

#include <stay/ecs/component.hpp>
#include <stay/graphics/cameraController.hpp>

namespace stay
{
    struct CameraFollow : public ecs::Component
    {
        CameraFollow(float lerpPerFrame = 0.F, ecs::Entity cameraController = ecs::Entity{})
        {
            data.cameraLerpPerFrame = lerpPerFrame;
            data.cameraController = cameraController;
        }
        struct Data : Serializable
        {
            float cameraLerpPerFrame{};
            ecs::Entity cameraController{};
            SERIALIZE(cameraLerpPerFrame, cameraController)
        } data{};
        struct Status 
        {
            CameraController* cameraController{nullptr};
        } status{};
        COMPONENT(CameraFollow, data)
    };
} // namespace stay