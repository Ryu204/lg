#include "loader.hpp"

#include <stay/components.hpp>
#include "../component/dash.hpp"
#include "../component/hook.hpp"
#include "../component/player.hpp"
#include "../component/playerDebug.hpp"

namespace stay
{
    void Loader::load(std::filesystem::path&& filename, Node* root)
    {
        ldtk::Project project;
        project.loadFromFile(std::move(filename).string());
        const auto& world = project.getWorld();
        const auto& level = world.getLevel("main");

        mDetail.pixelsPerMeter = level.getField<float>("pixelsPerMeter").value();
        mDetail.worldOffset = Vector2::from(level.size) / -2.F;

        const auto& bgr = level.getLayer("background");
        loadTileset(root, bgr);

        const auto& colliders = level.getLayer("colliders");
        loadColliders(root, colliders);

        const auto& setttingsLayer = level.getLayer("settings");
        const auto settings = loadSettings(root, setttingsLayer);

        const auto& objects = level.getLayer("entities");
        loadPlayer(root, objects, settings);

        const auto& bgrEntities = level.getLayer("backgroundEntities");
        loadBackgroundEntities(root, bgrEntities);
    }

    void Loader::loadTileset(Node* parent, const ldtk::Layer& layer) const
    {
        assert(layer.hasTileset() && "not tiled layer");
        const auto tileSize = static_cast<float>(layer.getTileset().tile_size) / mDetail.pixelsPerMeter;
        for (const auto& tile : layer.allTiles())
        {
            auto* entity = parent->createChild();
            Transform tf{toWorldPosition(Vector2::from(tile.getPosition())) + Vector2{tileSize / 2.F * (vectorRight + vectorDown)}};
            entity->setGlobalTransform(tf);
            entity->localTransform().setScale(
                Vector2{ -2.F * (float)tile.flipX + 1.F, -2.F * (float)tile.flipY + 1.F }
            );
            const auto tRect = tile.getTextureRect();
            const Rect textureRect{Vector2{tRect.x, tRect.y}, Vector2{tRect.x + tRect.width, tRect.y + tRect.height}};
            const Vector2 pivot{0.5F, 0.5F};
            TextureInfo info{ "mossy", textureRect, pivot };
            entity->addComponent<Render>(Color{0xFFFFFFFF}, Vector2{tileSize, tileSize}, 0, info);
        }
    }

    void Loader::loadColliders(Node* parent, const ldtk::Layer& layer) const 
    {
        const auto tileSize = static_cast<float>(layer.getCellSize());
        for (const auto& collider : layer.allEntities())
        {
            assert(collider.getName() == "collider" && "not a collider entity");
            std::vector<Vector2> chainShape;
            const auto position = toWorldPosition(Vector2::from(collider.getPosition()));
            for (const auto& point : collider.getArrayField<ldtk::IntPoint>("chain"))
            {
                const Vector2 gridPosition = Vector2::from(point.value()) + Vector2{0.5F, 0.5F};
                const auto pointPosition = toWorldPosition(tileSize * gridPosition);
                chainShape.emplace_back(pointPosition - position);
            }
            auto* entity = parent->createChild();
            phys::Material mat{1.F, 1.F, 0.F};
            entity->addComponent<phys::RigidBody>(position);
            entity->addComponent<phys::Collider>(phys::Chain{std::move(chainShape)}, mat);
        }
    }

    void Loader::loadPlayer(Node* parent, const ldtk::Layer& layer, Settings settings) const 
    {
        const auto& playerList = layer.getEntitiesByName("player");
        assert(playerList.size() == 1 && "exactly 1 player must present");
        const auto& player = playerList.front().get();

        const auto& colliderEntity = player.getField<ldtk::EntityRef>("collider").value();
        const auto position = toWorldPosition(Vector2::from(player.getPosition()));
        const auto colliderOffset = toWorldPosition(Vector2::from(colliderEntity->getPosition())) - position;

        auto* playerNode = parent->createChild();
        
        playerNode->addComponent<phys::RigidBody>(position, 0.F, phys::BodyType::DYNAMIC);
        phys::Material light{0.1F};
        const auto colliderSize = Vector2::from(colliderEntity->getSize()) / mDetail.pixelsPerMeter;
        auto& playerBody = playerNode->addComponent<phys::Collider>(phys::Circle{colliderOffset, colliderSize.x / 2.F}, light);
        playerBody.setLayer("Player");

        auto& stats = playerNode->addComponent<Player>();
        stats.moveStrength = player.getField<float>("moveStrength").value();
        stats.jumpHeight = player.getField<float>("jumpHeight").value();
        stats.camera = settings.camera;
        stats.cameraLerpPerFrame = player.getField<float>("cameraLerpPerFrame").value();
        stats.airDampingReduction = player.getField<float>("airDampingReduction").value();

        auto& hook = playerNode->addComponent<Hook>();
        hook.props.speed = player.getField<float>("bulletSpeed").value();
        hook.props.cooldown = player.getField<float>("bulletCooldown").value();
        hook.props.ropeLength = player.getField<float>("ropeLength").value();
        hook.props.pullSpeed = player.getField<float>("pullSpeed").value();

        playerNode->addComponent<PlayerDebug>();

        auto& dash = playerNode->addComponent<Dash>();
        dash.velocity = player.getField<float>("dashSpeed").value();
        dash.length = player.getField<float>("dashLength").value();
        dash.cooldown = player.getField<float>("dashCooldown").value();
        dash.postBrake = player.getField<float>("postBrake").value();

        auto* skin = playerNode->createChild();
        auto& skinBody = skin->addComponent<phys::RigidBody>(position, 0.F, phys::BodyType::DYNAMIC);
        skinBody.setGravityScale(player.getField<float>("gravityScale").value());
        skinBody.setHorizontalDamping(player.getField<float>("HDamping").value());
        skinBody.setLinearDamping(player.getField<float>("damping").value());
        skinBody.setFixedRotation(true);

        const phys::Material playerMaterial{1.F, player.getField<float>("friction").value(), 0.F};
        const phys::Box skinShape{colliderOffset, colliderSize};
        auto& skinCollider = skin->addComponent<phys::Collider>(skinShape, playerMaterial);
        skinCollider.setLayer("Player");
        skin->addComponent<phys::Joint>().start(phys::JointInfo{playerNode->entity(), false, phys::Revolute{position + colliderOffset}});

        const auto fileTextureRect = player.getTextureRect();
        const Rect playerRect{
            Vector2{fileTextureRect.x, fileTextureRect.y}, 
            Vector2{fileTextureRect.x + fileTextureRect.width, fileTextureRect.y + fileTextureRect.height}
        };
        const auto playerRenderSize = Vector2::from(player.getSize()) / mDetail.pixelsPerMeter;
        TextureInfo skinTexture{
            "player", playerRect, Vector2{0.5F, 0.5F}
        };
        skin->addComponent<Render>(
            Color{0xFFFFFFFF}, 
            playerRenderSize, 
            1, /* Player is only rendered behind main tileset */
            skinTexture
        );
    }

    Loader::Settings Loader::loadSettings(Node* parent, const ldtk::Layer& layer) const
    {
        for (const auto& entity : layer.allEntities())
        {
            if (entity.getName() == "cameraController")
            {
                auto node = parent->createChild();
                const auto height = entity.getField<float>("height").value();
                const Rect bounds{
                    toWorldPosition(Vector2::from(entity.getPosition())),
                    toWorldPosition(Vector2::from(entity.getPosition()) + Vector2::from(entity.getSize()))
                };
                node->addComponent<CameraController>(height, bounds);
                auto tf = node->globalTransform();
                tf.setPosition(toWorldPosition(Vector2::from(entity.getWorldPosition())));
                node->setGlobalTransform(tf);
                return Settings{
                    node->entity()
                };
            }
            else 
                assert(false && "Unknown entity name");
        }   
        assert(false && "should not reach here");
        return Settings{};
    }

    void Loader::loadBackgroundEntities(Node* parent, const ldtk::Layer& layer) const 
    {
        for (const auto& entity : layer.allEntities()) 
        {
            const auto size = Vector2::from(entity.getSize()) / mDetail.pixelsPerMeter;
            auto* node = parent->createChild();
            node->localTransform().setPosition(toWorldPosition(Vector2::from(entity.getPosition())));

            const auto fileTextureRect = entity.getTextureRect();
            const Rect textureRect{
                Vector2{fileTextureRect.x, fileTextureRect.y}, 
                Vector2{fileTextureRect.x + fileTextureRect.width, fileTextureRect.y + fileTextureRect.height}
            };
            TextureInfo info{ 
                entity.getTags().front(),  
                textureRect,
                Vector2{0, 0}
            };
            node->addComponent<Render>(
                Color{0xFFFFFFFF},
                size,
                10, /* z-order really big because it's background */
                info
            );
        }
    }

    Vector2 Loader::toWorldPosition(const Vector2& filePosition) const
    {
        const auto screenSpace = (filePosition + mDetail.worldOffset) / mDetail.pixelsPerMeter;
        auto worldSpace = Vector2{screenSpace.x, -screenSpace.y};
        return std::move(worldSpace);
    }
} // namespace stay
