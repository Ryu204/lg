#pragma once

#include <stay/ecs/system.hpp>

namespace stay 
{
    struct PlayerSystem 
        : public ecs::System
        , public ecs::StartSystem
        , public ecs::UpdateSystem
        , public ecs::InputSystem
    {
            REGISTER_SYSTEM(PlayerSystem)
            PlayerSystem(ecs::Manager* manager);
            void start() override;
            void input(const sf::Event& event) override;
            void update(float dt) override;
        private:
            bool mEntered;
    };
} // namespace stay