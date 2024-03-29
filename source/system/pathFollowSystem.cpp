#include "../component/pathFollow.hpp"

#include <stay/ecs/system.hpp>
#include <stay/utility/math.hpp>

namespace stay 
{
    class PathFollowSystem
        : public ecs::UpdateSystem
        , public ecs::System
    {
        public:
            REGISTER_SYSTEM(PathFollowSystem);
            PathFollowSystem(ecs::Manager* manager) 
                : ecs::UpdateSystem{0}
                , ecs::System{manager}
            {}

            void update(float dt) override
            {
                std::vector<ecs::Entity> toBeRemoved{};
                for (const auto& [entity, path] : mManager->getRegistryRef().view<PathFollow>().each())
                {
                    const auto unassigned = path.status.body == nullptr;
                    if (unassigned)
                        continue;
                    const auto pointCount = path.data.path.size();
                    assert(pointCount >= 2 && pointCount > path.status.nextGoal);
                    const Vector2 direction = path.data.path[path.status.nextGoal] - path.status.body->getPosition();
                    const auto distance = utils::lengthVec2(direction);
                    const auto approached = distance <= path.data.tolerance;
                    if (!approached)
                    {
                        path.status.body->setVelocity(direction.norm() * path.data.speed);
                        continue;
                    }
                    const auto finishedARow = 
                        (path.status.increasing && path.status.nextGoal == pointCount - 1)
                        || (!path.status.increasing && path.status.nextGoal == 0);
                    if (!finishedARow)
                    {
                        path.status.nextGoal += 2 * path.status.increasing - 1;
                        continue;
                    }
                    if (!path.data.loop)
                    {
                        toBeRemoved.push_back(entity);
                        continue;
                    }
                    if (path.data.pingpong)
                    {
                        path.status.nextGoal = path.status.increasing 
                            ? pointCount - 2
                            : 1;
                        path.status.increasing = !path.status.increasing;
                        continue;
                    }
                    path.status.nextGoal = (path.status.nextGoal + 1) % pointCount;
                }
                for (const auto& e : toBeRemoved)
                {
                    Node::getNode(e)->getComponent<phys::RigidBody>().setVelocity(Vector2{});
                    mManager->removeComponents<PathFollow>(e);
                }
            }
    };
} // namespace stay