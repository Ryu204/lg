#include "dashSystem.hpp"
#include "../component/dash.hpp"
#include "../component/player.hpp"
#include "../component/hook.hpp"

#include <stay/utility/invoke.hpp>

namespace stay 
{
    DashSystem::DashSystem(ecs::Manager* manager)
        : ecs::System(manager)
        , ecs::UpdateSystem(0)
        , ecs::InputSystem(0)
    {}

    void DashSystem::input(const sf::Event& event)
    {
        if (event.type != sf::Event::KeyPressed 
        || event.key.scancode != sf::Keyboard::Scan::K 
        || !sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::S))
            return;
        auto view = registry().view<Dash, Player>();
        for (const auto [entity, dash, player] : view.each())
        {
            auto* playerNode = dash.getNode();
            auto& body = *player.movementBody;
            if (isOnRope(playerNode) || dash.activated || !dash.canDash)
                continue;

            dash.left = body.getVelocity().x < 0.F;
            dash.canDash = false;
            dash.activated = true;
            const auto dashTime = dash.length / dash.velocity;
            Invoke::after(dash.cooldown, [&dash = dash]{
                dash.canDash = true;
            });
            Invoke::after(dashTime, [&dash = dash, &body = body, &player = player]{
                dash.activated = false;
                body.setVelocity(body.getVelocity() * dash.postBrake);
                player.onDash = false;
            });
            disableGravity(playerNode, dashTime);
            player.onDash = true;
        }
    }

    void DashSystem::update(float /*dt*/)
    {
        auto view = registry().view<Dash, Player>();
        for (const auto [entity, dash, player] : view.each())
        {
            if (dash.activated)
            {
                const Vector2 velocity{dash.left ? -dash.velocity : dash.velocity, 0.F};
                player.movementBody->setVelocity(velocity);
            }
        }
    }

    void DashSystem::disableGravity(Node* player, float duration)
    {
        auto& mainBody = player->getComponent<phys::RigidBody>();
        auto& hookBody = *player->getComponent<Player>().movementBody;
        const auto bodyScale = mainBody.gravityScale();
        const auto hookBodyScale = hookBody.gravityScale();
        mainBody.setGravityScale(0.F);
        hookBody.setGravityScale(0.F);
        Invoke::after(duration, [=, &mainBody, &hookBody]{
            mainBody.setGravityScale(bodyScale);
            hookBody.setGravityScale(hookBodyScale);
        });
    }

    bool DashSystem::isOnRope(Node* player) 
    {
        if (!player->hasComponent<Hook>())
            return false;
        return player->getComponent<Hook>().status.state != Hook::NONE;
    }
} // namespace stay