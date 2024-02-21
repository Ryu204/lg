#pragma once

#include <stay/ecs/component.hpp>
#include <stay/type/vector.hpp>

namespace stay
{
    struct Hook : public ecs::Component
    {
            enum State
            {
                NONE, SHOT, CONNECTED,
            };

            struct Properties : Serializable
            {  
                float speed{5.F};
                float cooldown{1.F};
                float ropeLength{5.F};
                float pullSpeed{5.F};
                SERIALIZE(speed, cooldown, ropeLength, pullSpeed);
            };

            struct Status
            {
                struct Pin
                {
                    bool clockwise{};
                    Node* node{nullptr};
                    Vector2 savedPosition{};
                    Pin(Node* node, Vector2 savedPosition, bool clockwise = true) 
                        : clockwise{clockwise}, node{node}, savedPosition{std::move(savedPosition)} {}
                };
                State state{NONE};
                bool shootable{true};
                float timeToCD{0.F};
                float maxLength{0.F};
                Node* bullet;
                std::vector<Pin> createdPins{};
                Node* collisionChecker{nullptr};
            };

            Properties props{};
            Status status{};        
            
            COMPONENT(Hook, props);
    };
} // namespace stay