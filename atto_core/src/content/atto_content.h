#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"
#include "../shared/atto_core.h"

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

        LargeString     filePath = {};
        byte *          pixelData = nullptr;
        i32             width = -1;
        i32             height = -1;
        i32             channels = 4;

    private:
        void GetPixel( i32 x, i32 y, u8 & r, u8 & g, u8 & b, u8 & a );
    };

    class ContentModelProcessor {
    public:
        ContentModelProcessor();
        ~ContentModelProcessor();

        bool LoadFromFile( Core * core, const char * file );

        LargeString                     filePath = {};
        SmallString                     name = {};
        std::vector< StaticMeshData  >  meshes;

    private:
        void            ProcessNode( aiNode * node, const aiScene * scene );
        StaticMeshData  ProcessMesh( aiMesh * mesh, const aiScene * scene );
    };
}
