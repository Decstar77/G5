#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"

namespace atto {
    class ContentTextureProcesor {
    public:
        ContentTextureProcesor();
        ~ContentTextureProcesor();

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
}
