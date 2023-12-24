#pragma once

#include "atto_core.h"

namespace atto {
    // Handy dandy mesh generation functions
    class GeometryFuncs {
    public:
        static void SortPointsIntoClockWiseOrder( glm::vec2 * points, i32 numPoints );
    };

    class StaticMeshGeneration {
    public:
        static StaticMeshData CreateQuad( f32 xLeft, f32 yTop, f32 w, f32 h, f32 depth );
        static StaticMeshData CreateBox( f32 width, f32 height, f32 depth, u32 numSubdivisions );
        static StaticMeshData CreateSphere( f32 radius, u32 sliceCount, u32 stackCount );
        static StaticMeshData CreateGeosphere( f32 radius, u32 numSubdivisions );
        static StaticMeshData CreateCylinder( f32 bottomRadius, f32 topRadius, f32 height, u32 sliceCount, u32 stackCount );
    };
}

