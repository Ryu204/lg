#pragma once 

#include <stay/ecs/system.hpp>

namespace stay
{
    struct DashSystem 
        : public ecs::InputSystem
        , public ecs::UpdateSystem
        , public ecs::System
    {
            REGISTER_SYSTEM(DashSystem)
            DashSystem(ecs::Manager* manager);
            void update(float dt) override;
            void input(const sf::Event& event) override;
        private:
            static void disableGravity(Node* player, float duration);
            static bool isOnRope(Node* player);
    };
} // namespace stay