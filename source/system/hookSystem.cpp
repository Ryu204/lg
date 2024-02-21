#include "hookSystem.hpp"

#include "../component/hook.hpp"
#include "../component/player.hpp"

#include <stay/physics/joint.hpp>
#include <stay/physics/collider.hpp>
#include <stay/type/vector.hpp>
#include <stay/utility/math.hpp>

namespace 
{
    const stay::Vector2 bulletSize{0.5F, 0.5F};
    const stay::Vector2 pinSize{0.2F, 0.2F};
    const stay::phys::Material heavyMaterial{5.F};
    constexpr float ropeMargin = 0.6F;
    constexpr float ropeWidth = 0.2F;
} // namespace

namespace stay
{
    HookSystem::HookSystem(ecs::Manager* manager)
        : ecs::System(manager)
        , ecs::UpdateSystem(0)
        , ecs::InputSystem(-1)
    {}
    
    void HookSystem::update(float dt)
    {
        updateCooldown(dt);
        updateDirection();
        attachQueuedBullets();
        splitQueuedPins();

        auto view = mManager->getRegistryRef().view<Hook, phys::RigidBody, Player>();
        for (auto [entity, hook, rigidbody, player] : view.each())
        {
            switch (hook.status.state)
            {
                case Hook::NONE:
                    player.onRope = false;
                    break;
                case Hook::SHOT:
                {
                    const auto bulletPosition = hook.status.bullet->getComponent<phys::RigidBody>().getPosition();
                    const auto playerPosition = rigidbody.getPosition();
                    const bool exceeded = utils::lengthVec2(bulletPosition - playerPosition) > hook.props.ropeLength;
                    if (exceeded)
                        resetHook(hook);
                    break;
                }
                case Hook::CONNECTED:
                    updateControl(hook, dt);
                    checkRopeForRejoin();
                    break;
            }         
        }
    }

    void HookSystem::input(const sf::Event& event)
    {
        const bool keyPressed = event.type == sf::Event::KeyPressed;
        const bool shot = keyPressed && event.key.scancode == sf::Keyboard::Scancode::J;
        const bool dispatch = keyPressed && event.key.scancode == sf::Keyboard::Scancode::K;

        auto view = mManager->getRegistryRef().view<Hook, Player>();
        for (auto [entity, hook, player] : view.each())
        {
            switch (hook.status.state)
            {
                case Hook::NONE:
                    if (shot && hook.status.shootable)
                        generateBullet(hook);
                    break;
                case Hook::SHOT:
                    break;
                case Hook::CONNECTED:
                    if (dispatch)
                        resetHook(hook);
                    player.canJump = true;
                    break;
            }         
        }
    }

    void HookSystem::updateDirection()
    {
        mDirection = Vector2{};
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            mDirection.x -= 1.F;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            mDirection.x += 1.F;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            mDirection.y += 1.F;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            mDirection.y -= 1.F;
        if (mDirection == Vector2{})
        {
            const Vector2 bestVector = vectorUp;
            mDirection = bestVector;
        }
        else
            mDirection = mDirection.norm();
    }

    void HookSystem::updateControl(Hook& hook, float dt)
    {
        float up = 0.F;
        up += static_cast<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::W));
        up -= static_cast<float>(sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::S));

        auto* prismatic = hook.getNode()->getComponent<phys::Joint>().getNativeHandle<b2PrismaticJoint>();
        float currentLength = prismatic->GetJointTranslation();
        currentLength -= up * hook.props.pullSpeed * dt;
        currentLength = utils::clamp(currentLength, 0.F, hook.status.maxLength);
        prismatic->SetLimits(0, currentLength);
    }

    void HookSystem::generateBullet(Hook& hook)
    {
        auto* playerNode = hook.getNode();
        auto& playerBody = playerNode->getComponent<phys::RigidBody>();
        auto* bullet = playerNode->createChild();
        auto& bulletBody = bullet->addComponent<phys::RigidBody>(playerBody.getPosition(), 0.F, phys::BodyType::DYNAMIC);
        auto& bulletCollider = bullet->addComponent<phys::Collider>(phys::Box{Vector2(), bulletSize}, heavyMaterial);

        bulletBody.setBullet(true);
        bulletBody.setVelocity(mDirection * hook.props.speed);
        bulletCollider.setLayer("Bullet");
        bulletCollider.OnCollisionEnter.addEventListener( [this, &hook](phys::Collision& contact) -> void {
            queueForAttachment(&hook, &contact.other->getNode()->getComponent<phys::RigidBody>());
        });
        bulletCollider.connect();

        hook.status.state = Hook::SHOT;
        hook.status.maxLength = hook.props.ropeLength;
        hook.status.bullet = bullet;
        hook.status.timeToCD = hook.props.cooldown;
        hook.status.shootable = false;
    }

    void HookSystem::resetHook(Hook& hook)
    {
        if (hook.status.bullet != nullptr)
        {
            hook.getNode()->destroy(hook.status.bullet);
            hook.status.bullet = nullptr;
        }

        for (auto [unused, pin, unused2] : hook.status.createdPins)
            hook.getNode()->destroy(pin);
        hook.status.createdPins.clear();

        if (hook.status.collisionChecker != nullptr)
        {
            hook.getNode()->destroy(hook.status.collisionChecker);
            hook.status.collisionChecker = nullptr;
        }

        hook.status.state = Hook::NONE;
    }

    void HookSystem::updateCooldown(float dt)
    {
        auto view = mManager->getRegistryRef().view<Hook>();
        for (auto [entity, hook] : view.each())
        {
            hook.status.timeToCD -= dt;
            if (hook.status.timeToCD < 0.F)
            {
                hook.status.timeToCD = 0.F;
                hook.status.shootable = true;
            }
        }
    }

    void HookSystem::attachQueuedBullets()
    {
        for (const auto& [hook, obstacle] : mQueuedAttaches)
        {
            auto position = hook->status.bullet->getComponent<phys::RigidBody>().getPosition();
            // Delete the bullet
            hook->getNode()->destroy(hook->status.bullet);
            hook->status.bullet = nullptr;
            createPinAt(position, hook, obstacle->getNode());
            hook->getNode()->getComponent<Player>().onRope = true;
        }
        mQueuedAttaches.clear();
    }

    void HookSystem::splitQueuedPins() 
    {
        for (const auto& [hook, tup] : mQueuedSplitting)
        {
            const auto& [pos, plat] = tup;
            createPinAt(pos, hook, plat);
        }

        mQueuedSplitting.clear();
    }

    void HookSystem::checkRopeForRejoin() 
    {
        static constexpr float tolerance = 0.2F;
        auto view = mManager->getRegistryRef().view<Hook, phys::RigidBody>();
        std::vector<std::tuple<Hook*, float /*abLength*/, phys::RigidBody* /*playerBody*/>> rejoinList;
        for (const auto& [entity, hook, playerBody] : view.each())
        {
            const bool unsplitted = hook.status.createdPins.size() <= 1;
            if (unsplitted)
                continue;
            const auto& pin = hook.status.createdPins.back();
            // A = lastPin, B = pin next to it, c = player position
            const auto b = (hook.status.createdPins.end() - 2)->node->getComponent<phys::RigidBody>().getPosition();
            const auto a = pin.node->getComponent<phys::RigidBody>().getPosition();            
            const auto ab = Vector2(b - a).norm();
            const auto ac = Vector2(hook.getNode()->getComponent<phys::RigidBody>().getPosition() - a).norm();
            const auto crossZ = ab.x * ac.y - ab.y * ac.x;
            const auto sameSide = pin.clockwise ? crossZ < tolerance : crossZ > -tolerance;
            if (sameSide)
                continue;
            rejoinList.emplace_back(&hook, Vector2(b - a).getLength(), &playerBody);
        }
        for (const auto& [hook, length, playerBody] : rejoinList)
        {
            auto& pinList = hook->status.createdPins;
            hook->status.maxLength += length;
            hook->getNode()->destroy(pinList.back().node);
            pinList.pop_back();
            
            // Reconnect to last pin
            auto* pinToConnect = pinList.back().node;
            auto& playerPrisJoint = hook->getNode()->addComponent<phys::Joint>();
            const auto playerPosition = playerBody->getPosition();
            const auto position = pinToConnect->getComponent<phys::RigidBody>().getPosition();
            connectToPin(hook, pinToConnect, position, playerPosition);
            createBoxConnect(hook->getNode(), pinToConnect, hook->status.maxLength, ropeMargin);
        }
    }

    void HookSystem::queueForAttachment(Hook* hook, phys::RigidBody* obstacle)
    {
        mQueuedAttaches.emplace(hook, obstacle);
    }

    void HookSystem::createBoxConnect(Node* player, Node* pin, float length, float margin) 
    {
        auto& playerBody = player->getComponent<phys::RigidBody>();
        const auto playerPosition = playerBody.getPosition();
        auto& pinBody = pin->getComponent<phys::RigidBody>();
        auto pinPosition = pinBody.getPosition();

        const Vector2 direction = playerPosition - pinPosition;
        const Vector2 absoluteCenter = pinPosition + length / 2.F * direction.norm();
        auto center = Vector2::from(
            pinBody.body()->GetLocalPoint(absoluteCenter.toVec2<b2Vec2>())
        );
        auto angleToOx = std::abs(direction.x) < 1e-4 ? 90.F : std::atan2(direction.y, direction.x) * RAD2DEG;
        auto absoluteAngle = angleToOx - 90.F;
        auto angle = absoluteAngle - pinBody.getAngle();

        phys::Box box{center, Vector2{ropeWidth, length - 2 * margin}, angle};
        static phys::Material mat{0.005F};
        auto& collider = pin->addComponent<phys::Collider01>(box, mat);
        collider.setLayer("Player");
        
        auto& hook = player->getComponent<Hook>();
        collider.OnCollisionDetection.addEventListener([this, &playerBody, &hook, pinPosition](phys::Collision& contact, b2Contact& nativeInfo) {
            const bool touchAtExtension = utils::lengthVec2(playerBody.getPosition() - pinPosition) < utils::lengthVec2(contact.position - pinPosition);
            if (!touchAtExtension)
                mQueuedSplitting.emplace(&hook, std::tuple<Vector2, Node*>(contact.position, contact.other->getNode()));
            nativeInfo.SetEnabled(false);
        });
        collider.connect();
    }

    void HookSystem::createPinAt(const Vector2& position, Hook* hook, Node* platform)
    {
        bool clockwise = false; // <- to be calculated
        auto* pin = hook->getNode()->createChild();
        auto& pinBody = pin->addComponent<phys::RigidBody>(position, 0.F, phys::BodyType::DYNAMIC);
        auto& playerBody = hook->getNode()->getComponent<phys::RigidBody>();
        const auto playerPosition = playerBody.getPosition();

        auto& basePinCollider = pin->addComponent<phys::Collider>(phys::Box{Vector2{}, pinSize}, heavyMaterial);
        basePinCollider.setLayer("Isolate");
        basePinCollider.connect();

        if (!hook->status.createdPins.empty())
        {
            auto& lastPinInfo = hook->status.createdPins.back();
            const auto lastPinPosition = lastPinInfo.node->getComponent<phys::RigidBody>().getPosition();   
            const Vector2 pinsRay = lastPinPosition - position;
            // Create joint to the pin
            hook->getNode()->removeComponents<phys::Joint>();
            lastPinInfo.node->removeComponents<phys::Collider01>();
            hook->status.maxLength -= std::min(hook->status.maxLength, utils::lengthVec2(pinsRay));
             // Determine the orientation
            const auto abNormed = pinsRay.norm();
            const auto acNormed = Vector2(lastPinInfo.savedPosition - position).norm();
            clockwise = abNormed.x * acNormed.y - abNormed.y * acNormed.x > 0.F;
        }
        auto& bulletRevJoint = pin->addComponent<phys::Joint>();
        bulletRevJoint.start(phys::JointInfo{platform->entity(), true, phys::Revolute{position}});
        connectToPin(hook, pin, position, playerPosition);
        // Create collision checker
        createBoxConnect(hook->getNode(), pin, hook->status.maxLength, ropeMargin);
        // Update information
        hook->status.state = Hook::CONNECTED;
        hook->status.createdPins.emplace_back(pin, playerPosition, clockwise);
    }

    void HookSystem::connectToPin(Hook* hook, Node* pin, const Vector2& pinPosition, const Vector2& playerPosition)
    {
        if (hook->getNode()->hasComponent<phys::Joint>())
            hook->getNode()->removeComponents<phys::Joint>();
        auto& playerPrisJoint = hook->getNode()->addComponent<phys::Joint>();
        playerPrisJoint.start(phys::JointInfo{pin->entity(), false, phys::Prismatic{playerPosition, pinPosition, pinPosition - playerPosition}});
        auto* nativePrisJoint = playerPrisJoint.getNativeHandle<b2PrismaticJoint>();
        nativePrisJoint->EnableLimit(true);
        nativePrisJoint->SetLimits(0.F, Vector2(pinPosition - playerPosition).getLength());
    }
} // namespace stay