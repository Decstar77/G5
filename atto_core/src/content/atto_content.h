#pragma once

#include "../game/atto_core.h"

#include "atto_content_bmp.h"
#include "atto_content_sprite_atlas.h"
#include "atto_content_mesh.h"

struct aiScene;
struct aiNode;
struct aiMesh;

namespace atto {

    class Core;
    class ContentTextureProcessor {
    public:
        ContentTextureProcessor();
        ~ContentTextureProcessor();

        bool LoadFromFile( const char * file );
        void MakeAlphaEdge(); // Assumes to be RGBA8
        void FixAplhaEdges(); // Assumes to be RGBA8
        void PremultiplyAlpha(); // Assumes to be RGBA8

        LargeString     filePath = {};
        byte *          pixelData = nullptr;
        i32             width = -1;
        i32             height = -1;
        i32             channels = 4;
    };

    class PixelArtMeshCreator {
    public:
        bool LoadFromFile( const char * file );

        inline bool IsLoaded() { return points.size() != 0; }

        struct PixyPoint {
            glm::vec4 c;
            glm::vec2 p;
        };

        std::vector<PixyPoint> points;
    };

}
