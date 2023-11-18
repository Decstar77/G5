#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"

namespace atto {
    class ContentTextureProcesor {
    public:
        ContentTextureProcesor();
        ~ContentTextureProcesor();

        bool LoadFromFile( const char * file );
        void FixAplhaEdges(); // Assumes to be RGBA8

        byte *  pixelData = nullptr;
        i32     width = -1;
        i32     height = -1;
        i32     channels = 4;
    };
}
