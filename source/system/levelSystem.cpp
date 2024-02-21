#include <stay/ecs/system.hpp>
#include "../loader/loader.hpp"

namespace stay
{
    class LevelSystem : public ecs::System, public ecs::InitSystem
    {
        public:
            REGISTER_SYSTEM(LevelSystem)
            LevelSystem(ecs::Manager* manager)
                : ecs::System(manager)
                , ecs::InitSystem(0)
            {}
            void init(ecs::SystemContext& /* context */) override
            {
                constructLoader<Loader>();
            }
    };
} // namespace stay