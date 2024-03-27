#include "../component/cameraFollow.hpp"
#include "../component/cameraRegion.hpp"

#include <stay/ecs/system.hpp>
#include <stay/utility/math.hpp>

namespace stay 
{
    class CameraFollowSystem
        : public ecs::StartSystem
        , public ecs::PostUpdateSystem
        , public ecs::System
    {
        public:
            REGISTER_SYSTEM(CameraFollowSystem);
            CameraFollowSystem(ecs::Manager* manager) 
                : ecs::StartSystem{0}
                , ecs::PostUpdateSystem{0}
                , ecs::System{manager}
            {}

            void start() override
            {
                for (const auto& [entity, cam] : mManager->getRegistryRef().view<CameraFollow>().each())
                {
                    cam.status.cameraController = &mManager->getComponent<CameraController>(cam.data.cameraController);
                }
            }

            void postUpdate(float dt) override
            {
                const auto& camRegions = mManager->getRegistryRef().view<CameraRegion>();
                for (const auto& [entity, camFollow] : mManager->getRegistryRef().view<CameraFollow>().each()) 
                {
                    auto* camera = camFollow.status.cameraController->getNode();
                    auto tf = camFollow.getNode()->globalTransform();
                    const auto targetPosition = tf.getPosition();
                    const auto curPosition = utils::lerp<Vector2>(
                        camera->globalTransform().getPosition(),
                        targetPosition,
                        camFollow.data.cameraLerpPerFrame * dt
                    );

                    for (const auto& [e, region] : camRegions.each()) 
                    {
                        if (region.bounds.contain(targetPosition))
                        {
                            camFollow.status.cameraController->bounds = region.bounds;
                            break;
                        }
                    }

                    tf.setPosition(curPosition);
                    camera->setGlobalTransform(tf);
                }
            }
    };
} // namespace stay