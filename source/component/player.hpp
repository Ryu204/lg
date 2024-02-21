#pragma once

#include <stay/physics/rigidBody.hpp>

namespace stay
{
    struct Player : public ecs::Component
    {
        // Base force to move with controls
        float moveStrength{};
        float jumpHeight{};
        // Set this to a higher-than-one float to create snappy movement
        float oppositeScale{};
        // Same but apply to airbone moments
        float airScale{};

        bool canJump{false};
        bool onGround{false};
        bool onRope{false};
        bool onDash{false};
        phys::RigidBody* movementBody;
        phys::RigidBody* hookBody;

        COMPONENT(Player, moveStrength, jumpHeight, oppositeScale, airScale);
    };
} // namespace stay