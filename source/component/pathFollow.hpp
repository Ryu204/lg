#pragma once

#include <stay/ecs/component.hpp>
#include <stay/physics/rigidBody.hpp>

namespace stay
{
    struct PathFollow : public ecs::Component
    {
        PathFollow(std::vector<Vector2> path = {}, float tolerance = {}, float speed = {}, bool loop = {}, bool pingpong = {})
        {
            data.path = std::move(path);
            data.tolerance = tolerance;
            data.speed = speed;
            data.loop = loop;
            data.pingpong = pingpong;
        }
        ~PathFollow() override
        {
            if (status.body != nullptr)
            {
                status.body->OnRemoval.removeListener(status.bodyRemoveCallback);
            }
        }

        void setBody(phys::RigidBody* body)
        {
            if (status.body != nullptr)
            {
                status.body->OnRemoval.removeListener(status.bodyRemoveCallback);
            }
            if (body == nullptr)
            {
                status.body = nullptr;
                return;
            }

            status.nextGoal = 0;
            assert(body->type() == phys::BodyType::KINEMATIC);
            status.body = body;
            status.bodyRemoveCallback = body->OnRemoval.addEventListener([this] () {
                status.body = nullptr;
            });
        }

        struct Data : Serializable
        {
            std::vector<Vector2> path{};
            float tolerance{};
            float speed{};
            bool loop{};
            bool pingpong{};
            SERIALIZE(path, tolerance, speed, loop, pingpong)
        } data{};
        struct Status
        {
            bool increasing{true};
            std::size_t nextGoal{0};
            phys::RigidBody* body{nullptr};
            std::size_t bodyRemoveCallback{0};
        } status{};

        COMPONENT(PathFollow, data)
    };
} // namespace stay