#pragma once

#include "../shared/atto_defines.h"
#include "../shared/atto_containers.h"
#include "../shared/atto_core.h"

namespace atto {
    class StaticMeshGeneration {
    public:
        static StaticMeshData CreateQuad( f32 xLeft, f32 yTop, f32 w, f32 h, f32 depth );
        static StaticMeshData CreateBox( f32 width, f32 height, f32 depth, u32 numSubdivisions );
        static StaticMeshData CreateSphere( f32 radius, u32 sliceCount, u32 stackCount );
        static StaticMeshData CreateGeosphere( f32 radius, u32 numSubdivisions );
        static StaticMeshData CreateCylinder( f32 bottomRadius, f32 topRadius, f32 height, u32 sliceCount, u32 stackCount );
    };
}

