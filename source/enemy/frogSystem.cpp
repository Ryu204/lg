#include "frogIdle.hpp"
#include "frogWalking.hpp"

#include <stay/ecs/system.hpp>

namespace stay
{
    struct FrogSystem : ecs::System, ecs::StartSystem, ecs::UpdateSystem
    {
        FrogSystem(ecs::Manager* manager)
            : ecs::System{manager}
            , ecs::StartSystem{0}
            , ecs::UpdateSystem{0}
        {}
        REGISTER_SYSTEM(FrogSystem)

        void start() override
        {
            for (const auto [e, frog] : mManager->getRegistryRef().view<Frog>().each())
            {
                auto* node = Node::getNode(e);
                frog.context.body = &node->getComponent<phys::RigidBody>();
                frog.stateStack = std::make_unique<Frog::Stack>();
                frog.stateStack->setContext(frog.context);

                frog.stateStack->add<FrogIdle>(Frog::StateName::Idle, frog.data.rotateSpeed);
                frog.stateStack->add<FrogWalking>(Frog::StateName::Walking, frog.data.speed);

                frog.stateStack->push(Frog::StateName::Idle);
            }
        }

        void update(float dt) override
        {
            for (const auto [e, frog] : mManager->getRegistryRef().view<Frog>().each())
            {
                frog.stateStack->untilFalse<Frog::State>([dt](Frog::State& state) -> bool {
                    return state.update(dt);
                });
            }
        }
    };
} // namespace stay