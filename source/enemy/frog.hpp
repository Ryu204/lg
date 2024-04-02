#pragma once

#include <stay/utility/stateMachine.hpp>
#include <stay/ecs/component.hpp>
#include <stay/physics/rigidBody.hpp>

namespace stay
{
    struct Frog : ecs::Component
    {
        enum StateName 
        {
            Idle, Walking
        };
        struct Context 
        {
            phys::RigidBody* body{nullptr};
        };
        struct State : fsm::State<Context, StateName>
        {
            virtual bool update(float dt) { return true; };
        };

        Context context{};
        using Stack = fsm::Stack<Context, StateName>;
        std::unique_ptr<Stack> stateStack;

        Frog(float rotateSpeed = {}, Vector2 speed = {})
        {
            data.rotateSpeed = rotateSpeed;
            data.speed = speed;
        }
        struct Data : Serializable
        {
            float rotateSpeed{};
            Vector2 speed{};
            SERIALIZE(rotateSpeed, speed);
        } data{};

        COMPONENT(Frog, data);
    };
} // namespace stay