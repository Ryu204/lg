#pragma once

#include <filesystem>

#include <LDtkLoader/Project.hpp>

#include <stay/loader/iLoader.hpp>

namespace stay
{
    class Node;
    class Loader : public ILoader
    {
        public:
            void load(std::filesystem::path&& filename, Node* root) override;
        private:
            struct Detail 
            {
                float pixelsPerMeter;
                Vector2 levelOffset;
            } mDetail;
            struct Settings {
                ecs::Entity camera;
            };

            Vector2 toWorldPosition(const Vector2& filePosition) const;

            void loadTileset(Node* parent, const ldtk::Layer& layer) const;
            void loadBackgroundEntities(Node* parent, const ldtk::Layer& layer) const;
            void loadColliders(Node* parent, const ldtk::Layer& layer) const;
            void loadPlayer(Node* parent, const ldtk::Entity& playerEntity, const Settings& settings) const;
            void loadEntities(Node* parent, const ldtk::Layer& layer) const;
            void loadCameraRegions(Node* parent, const ldtk::Layer& layer) const;
            void loadPlayerAndSettings(Node* parent, const ldtk::Layer& layer) const;
    };
} // namespace stay
