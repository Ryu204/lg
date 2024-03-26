#pragma once

#include <stay/physics/rigidBody.hpp>

namespace stay
{
    struct Player : public ecs::Component
    {
        // Base force to move with controls
        float moveStrength{};
        float jumpHeight{};
        float airDampingReduction{};
        // Camera reference
        ecs::Entity camera{};
        float cameraLerpPerFrame{};

        bool canJump{false};
        bool onGround{false};
        bool onRope{false};
        bool onDash{false};
        phys::RigidBody* movementBody;
        phys::RigidBody* hookBody;

        COMPONENT(Player, moveStrength, jumpHeight, camera);
    };
} // namespace stay