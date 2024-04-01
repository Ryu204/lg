#include "debugSystem.hpp"

#include "../component/playerDebug.hpp"

#include <stay/world/camera.hpp>
#include <stay/physics/debugShape.hpp>
#include <stay/physics/raycast.hpp>

namespace stay
{
    DebugSystem::DebugSystem(ecs::Manager* manager)
        : ecs::System{manager}
        , ecs::InputSystem{0}
    {}

    void DebugSystem::input(const sf::Event& event) 
    {
        for (const auto& [entity, debug] : mManager->getRegistryRef().view<PlayerDebug>().each())
        {
            auto* node = Node::getNode(entity);
            if (event.type != sf::Event::KeyPressed || event.key.code != sf::Keyboard::E)
                continue;
            // Test raycasting
            if (debug.points.size() >= 2)
            {
                debug.points.clear();
                node->removeComponents<DebugShape>();
                continue;
            }
            const Vector2 currentPos = node->globalTransform().getPosition();
            debug.points.emplace_back(currentPos);
            if (debug.points.size() == 2) 
            {
                auto hit = phys::Raycast::nearest(debug.points[1], debug.points[0]);
                auto& debugShape = node->addComponent<DebugShape>();
                if (hit.has_value())
                {
                    debugShape.addLine(currentPos, hit->point, Color{0xFF0000FF});
                }
                else
                {
                    debugShape.addLine(currentPos, debug.points[0], Color{0x00FF00FF});
                }
            }
        }
    }
} // namespace stay