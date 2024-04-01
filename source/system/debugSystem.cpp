#include "debugSystem.hpp"

#include "../component/playerDebug.hpp"
#include "../component/player.hpp"

#include <stay/world/camera.hpp>
#include <stay/physics/debugShape.hpp>

namespace stay
{
    DebugSystem::DebugSystem(ecs::Manager* manager)
        : ecs::System{manager}
        , ecs::InitSystem{0}
        , ecs::InputSystem{0}
        , ecs::UpdateSystem{0}
        , mCamera{nullptr}
        , mWindow{nullptr}
    {}

    void DebugSystem::init(ecs::SystemContext& context)
    {
        mCamera = &context.camera;
        mWindow = &context.window;
    }

    void DebugSystem::input(const sf::Event& event) 
    {
        for (const auto& [entity, debug] : mManager->getRegistryRef().view<PlayerDebug>().each())
        {
            auto* node = Node::getNode(entity);
            DebugShape* shape{nullptr};
            if (event.type != sf::Event::KeyPressed || event.key.code != sf::Keyboard::E)
                continue;
            if (node->hasComponent<DebugShape>())
                shape = &node->getComponent<DebugShape>();
            else
                shape = &node->addComponent<DebugShape>();
            shape->addLine(node->globalTransform().getPosition(), Vector2{});
        }
    }

    void DebugSystem::update(float /*dt*/)
    {
        if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Q))
            return;
        for (const auto& [entity, player, unused] : mManager->getRegistryRef().view<Player, PlayerDebug>().each())
        {
            const auto mouse = mousePosition();
            player.hookBody->setPosition(mouse);
            player.movementBody->setPosition(mouse);
        }
    }

    Vector2 DebugSystem::mousePosition() const
    {
        assert(mCamera != nullptr && mWindow != nullptr && "uninitialized debug system");
        auto res = Vector2::from(mWindow->mapPixelToCoords(sf::Mouse::getPosition(*mWindow), mCamera->getView()));
        res.y *= -1.F;
        return res;
    }
} // namespace stay