#pragma once

#include <stay/ecs/component.hpp>
#include <stay/physics/rigidBody.hpp>
#include <stay/physics/collider.hpp>

#include "../component/pathFollow.hpp"

namespace stay
{
    struct PlatformTrigger : public ecs::Component
    {
        PlatformTrigger(ecs::Entity targetPlatform = {})
        {
            data.targetPlatform = targetPlatform;
        }
        ~PlatformTrigger() override
        {
            if (status.triggered && getNode()->hasComponent<phys::Collider>())
            {
                getNode()->getComponent<phys::Collider>().OnTriggerEnter.removeListener(status.selfColliderCallback);
            }
        }
        struct Data : Serializable
        {
            ecs::Entity targetPlatform{0};
            SERIALIZE(targetPlatform);
        } data{};
        struct Status
        {
            PathFollow* targetPlatform{nullptr};
            std::size_t selfColliderCallback{};
            bool triggered{false};
        } status{};
        COMPONENT(PlatformTrigger, data);

        void start() 
        {
            status.targetPlatform = &Node::getNode(data.targetPlatform)->getComponent<PathFollow>();
            
            auto& selfCollider = getNode()->getComponent<phys::Collider>();
            status.selfColliderCallback = selfCollider.OnTriggerEnter.addEventListener(
                [this, &selfCollider] (phys::Collision& col) {
                    if (status.triggered)
                        return;
                    const auto isPlayer = col.other->layer() == "Player";
                    if (!isPlayer)
                        return;
                    auto& platformBody = status.targetPlatform->getNode()->getComponent<phys::RigidBody>();
                    status.targetPlatform->setBody(&platformBody);
                    status.triggered = true;
                }
            );
        }
    };
} // namespace stay