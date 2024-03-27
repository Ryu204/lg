#include "loader.hpp"

#include <stay/components.hpp>
#include "../component/dash.hpp"
#include "../component/hook.hpp"
#include "../component/player.hpp"
#include "../component/cameraFollow.hpp"
#include "../component/cameraRegion.hpp"
#include "../component/playerDebug.hpp"

#include <utility>

namespace stay
{
    void Loader::load(std::filesystem::path&& filename, Node* root)
    {
        ldtk::Project project;
        project.loadFromFile(std::move(filename).string());
        const auto& world = project.getWorld();
        const auto& mainLevel = world.getLevel("main0");

        mDetail.pixelsPerMeter = mainLevel.getField<float>("pixelsPerMeter").value();
        mDetail.levelOffset = Vector2::from(mainLevel.position);

        const auto& playerAndSetttingsLayer = mainLevel.getLayer("settings");
        loadPlayerAndSettings(root, playerAndSetttingsLayer);

        for (auto levelCount = 0; true; levelCount++)
        {
            std::reference_wrapper<const ldtk::Level> level = mainLevel; 
            try 
            {
                const auto currentLevel = std::string("main") + std::to_string(levelCount);
                level = world.getLevel(currentLevel);
                mDetail.levelOffset = Vector2::from(level.get().position);
            } 
            catch (...) 
            {
                break;
            }

            const auto& bgr = level.get().getLayer("background");
            loadTileset(root, bgr);

            const auto& colliders = level.get().getLayer("colliders");
            loadColliders(root, colliders);

            const auto& entities = level.get().getLayer("entities");
            loadEntities(root, entities);
 
            const auto& bgrEntities = level.get().getLayer("backgroundEntities");
            loadBackgroundEntities(root, bgrEntities);

            const auto& camRegions = level.get().getLayer("cameraRegions");
            loadCameraRegions(root, camRegions);
        }
    }

    void Loader::loadTileset(Node* parent, const ldtk::Layer& layer) const
    {
        assert(layer.hasTileset() && "not tiled layer");
        const auto tileSize = static_cast<float>(layer.getTileset().tile_size) / mDetail.pixelsPerMeter;
        for (const auto& tile : layer.allTiles())
        {
            auto* entity = parent->createChild();
            Transform tf{toWorldPosition(Vector2::from(tile.getWorldPosition())) + Vector2{tileSize / 2.F * (vectorRight + vectorDown)}};
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
            const auto position = toWorldPosition(Vector2::from(collider.getWorldPosition()));
            for (const auto& point : collider.getArrayField<ldtk::IntPoint>("chain"))
            {
                const Vector2 gridPosition = Vector2::from(point.value()) + Vector2{0.5F, 0.5F};
                const auto pointPosition = toWorldPosition(mDetail.levelOffset + tileSize * gridPosition);
                chainShape.emplace_back(pointPosition - position);
            }
            auto* entity = parent->createChild();
            phys::Material mat{1.F, 1.F, 0.F};
            entity->addComponent<phys::RigidBody>(position);
            entity->addComponent<phys::Collider>(phys::Chain{std::move(chainShape)}, mat);
        }
    }

    void Loader::loadEntities(Node* parent, const ldtk::Layer& layer) const 
    {
        // TODO: add entity loading
    }

    void Loader::loadPlayer(Node* parent, const ldtk::Entity& playerEntity, const Settings& settings) const
    {
        const auto& colliderEntity = playerEntity.getField<ldtk::EntityRef>("collider").value();
        const auto position = toWorldPosition(Vector2::from(playerEntity.getWorldPosition()));
        const auto colliderOffset = toWorldPosition(Vector2::from(colliderEntity->getWorldPosition())) - position;

        auto* playerNode = parent->createChild();
        
        playerNode->addComponent<phys::RigidBody>(position, 0.F, phys::BodyType::DYNAMIC);
        phys::Material light{0.1F};
        const auto colliderSize = Vector2::from(colliderEntity->getSize()) / mDetail.pixelsPerMeter;
        auto& playerBody = playerNode->addComponent<phys::Collider>(phys::Circle{colliderOffset, colliderSize.x / 2.F}, light);
        playerBody.setLayer("Player");

        auto& stats = playerNode->addComponent<Player>();
        stats.moveStrength = playerEntity.getField<float>("moveStrength").value();
        stats.jumpHeight = playerEntity.getField<float>("jumpHeight").value();
        stats.airDampingReduction = playerEntity.getField<float>("airDampingReduction").value();

        auto& hook = playerNode->addComponent<Hook>();
        hook.props.speed = playerEntity.getField<float>("bulletSpeed").value();
        hook.props.cooldown = playerEntity.getField<float>("bulletCooldown").value();
        hook.props.ropeLength = playerEntity.getField<float>("ropeLength").value();
        hook.props.pullSpeed = playerEntity.getField<float>("pullSpeed").value();

        playerNode->addComponent<CameraFollow>(
            playerEntity.getField<float>("cameraLerpPerFrame").value(),
            settings.camera
        );
        
        playerNode->addComponent<PlayerDebug>();

        auto& dash = playerNode->addComponent<Dash>();
        dash.velocity = playerEntity.getField<float>("dashSpeed").value();
        dash.length = playerEntity.getField<float>("dashLength").value();
        dash.cooldown = playerEntity.getField<float>("dashCooldown").value();
        dash.postBrake = playerEntity.getField<float>("postBrake").value();

        auto* skin = playerNode->createChild();
        auto& skinBody = skin->addComponent<phys::RigidBody>(position, 0.F, phys::BodyType::DYNAMIC);
        skinBody.setGravityScale(playerEntity.getField<float>("gravityScale").value());
        skinBody.setHorizontalDamping(playerEntity.getField<float>("HDamping").value());
        skinBody.setLinearDamping(playerEntity.getField<float>("damping").value());
        skinBody.setFixedRotation(true);

        const phys::Material playerMaterial{1.F, playerEntity.getField<float>("friction").value(), 0.F};
        const phys::Box skinShape{colliderOffset, colliderSize};
        auto& skinCollider = skin->addComponent<phys::Collider>(skinShape, playerMaterial);
        skinCollider.setLayer("Player");
        skin->addComponent<phys::Joint>().start(phys::JointInfo{playerNode->entity(), false, phys::Revolute{position + colliderOffset}});

        const auto fileTextureRect = playerEntity.getTextureRect();
        const Rect playerRect{
            Vector2{fileTextureRect.x, fileTextureRect.y}, 
            Vector2{fileTextureRect.x + fileTextureRect.width, fileTextureRect.y + fileTextureRect.height}
        };
        const auto playerRenderSize = Vector2::from(playerEntity.getSize()) / mDetail.pixelsPerMeter;
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

    void Loader::loadPlayerAndSettings(Node* parent, const ldtk::Layer& layer) const
    {
        assert(layer.getEntitiesByName("cameraController").size() > 0);
        const ldtk::Entity& cameraController = layer.getEntitiesByName("cameraController").front();
        auto camera = parent->createChild();
        const auto height = cameraController.getField<float>("height").value();
        camera->addComponent<CameraController>(height);
        auto cameraTf = camera->globalTransform();
        cameraTf.setPosition(toWorldPosition(Vector2::from(cameraController.getWorldPosition())));
        camera->setGlobalTransform(cameraTf);
        
        const Settings settings { camera->entity() };

        assert(layer.getEntitiesByName("player").size() > 0);
        const ldtk::Entity& playerEntity = layer.getEntitiesByName("player").front();
        loadPlayer(parent, playerEntity, settings);
    }

    void Loader::loadCameraRegions(Node* parent, const ldtk::Layer& layer) const
    {
        for (const auto& entity : layer.allEntities()) 
        {
            assert(entity.getName() == "cameraRegion");
            const Rect bounds {
                toWorldPosition(Vector2::from(entity.getWorldPosition())),
                toWorldPosition(Vector2::from(entity.getWorldPosition()) + Vector2::from(entity.getSize()))
            };
            auto* node = parent->createChild();
            node->addComponent<CameraRegion>(bounds);
        }
    }

    void Loader::loadBackgroundEntities(Node* parent, const ldtk::Layer& layer) const 
    {
        for (const auto& entity : layer.allEntities()) 
        {
            const auto size = Vector2::from(entity.getSize()) / mDetail.pixelsPerMeter;
            auto* node = parent->createChild();
            node->localTransform().setPosition(toWorldPosition(Vector2::from(entity.getWorldPosition())));

            const auto fileTextureRect = entity.getTextureRect();
            const Rect textureRect{
                Vector2{fileTextureRect.x, fileTextureRect.y}, 
                Vector2{fileTextureRect.x + fileTextureRect.width, fileTextureRect.y + fileTextureRect.height}
            };
            assert(entity.getTags().size() > 0);
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
        const auto screenSpace = filePosition / mDetail.pixelsPerMeter;
        auto worldSpace = Vector2{screenSpace.x, -screenSpace.y};
        return std::move(worldSpace);
    }
} // namespace stay
