#include "playerSystem.hpp"

#include "../component/player.hpp"

#include <stay/physics/collider.hpp>

namespace 
{
    const stay::Vector2 skinRatio{1.2F, 1.5F};
}

namespace stay
{
   
    PlayerSystem::PlayerSystem(ecs::Manager* manager)
        : ecs::System(manager)
        , ecs::StartSystem(0)
        , ecs::InputSystem(-1)
        , ecs::UpdateSystem(0)
        , mEntered(false)
    {}

    void PlayerSystem::start()
    {
        auto view = mManager->getRegistryRef().view<Player, phys::RigidBody>();
        for (const auto [entity, player, body] : view.each())
        {
            player.hookBody = &player.getNode()->getComponent<phys::RigidBody>();
            auto* skin = player.getNode()->getChildren().at(0);
            player.movementBody = &skin->getComponent<phys::RigidBody>();
            auto& collider = skin->getComponent<phys::Collider>();
            collider.OnCollisionEnter.addEventListener(
                [&player = player](phys::Collision& contact)
                {
                    if (contact.normal.y < 0.F)
                    {
                        player.canJump = true;
                        player.onGround = true;
                    }
                }
            );
            collider.OnCollisionExit.addEventListener(
                [&player = player](phys::Collision& contact) 
                {
                    if (contact.normal.y < 0.F)
                    {
                        player.onGround = false;
                    }
                }
            );
        }
    }

    void PlayerSystem::input(const sf::Event& event)
    {
        mEntered = false;
        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.scancode)
            {
                case sf::Keyboard::Scan::K:
                    mEntered = true;
                    break;
                default:
                    break;
            }
        }

        if (mEntered)
        {
            auto view = mManager->getRegistryRef().view<Player>();
            for (auto [entity, player] : view.each())
            {
                if (!player.canJump)
                    continue;
                auto vel = player.movementBody->getVelocity();
                auto grav = player.movementBody->gravity();
                vel.y = std::sqrt(std::abs(2 * player.jumpHeight * grav.y * player.movementBody->gravityScale()));
                player.movementBody->setVelocity(vel);
                player.canJump = false;
            }
        }
    }

    void PlayerSystem::update(float /*dt*/)
    {
        Vector2 dir{0.F, 0.F};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::A))
            dir.x -= 1.F;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::D))
            dir.x += 1.F;
        auto view = mManager->getRegistryRef().view<Player>();
        for (auto [entity, player] : view.each())
        {
            if (player.onDash)
                continue;
            auto force = Vector2::from(dir * player.moveStrength);
            const auto vel = player.movementBody->getVelocity();
            const auto isLeft = vel.x < 5.F;
            const auto isRight = vel.x > -5.F;
            const bool mayIncreaseForce = (dir.x < 0.F && isRight) || (dir.x > 0.F && isLeft);
            if (mayIncreaseForce && !player.onRope)
            {
                force = force * (player.onGround ? player.oppositeScale : player.airScale);
            }
            player.movementBody->applyForce(force);
        }
    }
} // namespace stay