#include "../component/suicide.hpp"

#include <stay/ecs/system.hpp>
#include <stay/graphics/render.hpp>

namespace stay
{
    struct SuicideSystem : ecs::PostUpdateSystem, ecs::System
    {
        REGISTER_SYSTEM(SuicideSystem)
        SuicideSystem(ecs::Manager* manager)
            : ecs::PostUpdateSystem{0}
            , ecs::System{manager}
        {}

        void postUpdate(float dt) override
        {
            std::vector<Node*> toBeRemoved{};
            for (const auto [entity, suicide] : mManager->getRegistryRef().view<Suicide>().each())
            {
                auto* node = suicide.getNode();
                suicide.elapsed += dt;
                if (suicide.elapsed > suicide.countDown)
                {
                    toBeRemoved.push_back(node);
                    continue;
                }
                const auto ratio = suicide.elapsed / suicide.countDown;
                node->localTransform().setScale(Vector2{1.F + (suicide.scale - 1.F) * Vector2{ratio, ratio}});
                if (node->hasComponent<Render>())
                {
                    auto& render = node->getComponent<Render>();
                    render.color.a = static_cast<std::uint8_t>(static_cast<float>(UINT8_MAX) * (1.F - ratio));
                }
            }
            for (auto* node : toBeRemoved)
            {
                assert(node->stray() == false && "Cannot make stray node suicide");
                node->parent()->destroy(node);
            }
        }
    };
} // namespace stay