#pragma once 

#include <stay/ecs/system.hpp>
#include <stay/utility/typedef.hpp>

namespace stay
{
    class Camera;
    struct DebugSystem 
        : public ecs::UpdateSystem
        , public ecs::InitSystem
        , public ecs::InputSystem
        , public ecs::System
    {   
            REGISTER_SYSTEM(DebugSystem)
            DebugSystem(ecs::Manager* manager);
            void init(ecs::SystemContext& context) override;
            void update(float dt) override;
            void input(const sf::Event& event) override;
        private:
            Vector2 mousePosition() const;
            Camera* mCamera;
            RWin* mWindow;
    };
} // namespace stay