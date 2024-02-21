#pragma once

#include <unordered_map>

#include <stay/ecs/system.hpp>
#include <stay/world/node.hpp>

namespace stay
{
    namespace phys
    {
        class RigidBody;
    } // namespace phys

    struct Hook;

    struct HookSystem : public ecs::System, public ecs::UpdateSystem, public ecs::InputSystem
    {
            REGISTER_SYSTEM(HookSystem)
            HookSystem(ecs::Manager* manager);
            void update(float dt) override;
            void input(const sf::Event& event) override;
        private:
            void generateBullet(Hook& hook);
            static void resetHook(Hook& hook);
            
            void queueForAttachment(Hook* hook, phys::RigidBody* obstacle);
            void attachQueuedBullets();
            void splitQueuedPins();
            
            void createPinAt(const Vector2& position, Hook* hook, Node* platform);
            static void connectToPin(Hook* hook, Node* pin, const Vector2& pinPosition, const Vector2& playerPosition);
            void createBoxConnect(Node* player, Node* pin, float length, float margin);

            void updateCooldown(float dt);
            void updateDirection();
            static void updateControl(Hook& hook, float dt);
            void checkRopeForRejoin();
            
            Vector2 mDirection;
            std::unordered_map<Hook*, phys::RigidBody*> mQueuedAttaches;
            std::unordered_map<Hook*, std::tuple<Vector2, Node*>> mQueuedSplitting;
    };
} // namespace stay