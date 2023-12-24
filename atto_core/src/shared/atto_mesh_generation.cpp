#include "atto_mesh_generation.h"

namespace atto {
    static StaticMeshVertex CreateVertex(
        f32 px, f32 py, f32 pz,
        f32 nx, f32 ny, f32 nz,
        f32 tx, f32 ty, f32 tz,
        f32 u, f32 v ) {
        StaticMeshVertex meshVertex = {};
        meshVertex.position = glm::vec3( px, py, pz );
        meshVertex.normal = glm::vec3( nx, ny, nz );
        //tanget = Vec3f( tx, ty, tz );
        meshVertex.uv = glm::vec2( u, v );
        return meshVertex;
    }

    static StaticMeshVertex ComputeMidPoint( const StaticMeshVertex & a, const StaticMeshVertex & b ) {
        StaticMeshVertex result = {};
        result.position = 0.5f * ( a.position + b.position );
        result.normal = glm::normalize( 0.5f * ( a.normal + b.normal ) );
        //result.tanget = Normalize( 0.5f * ( a.tanget + b.tanget ) );
        result.uv = 0.5f * ( a.uv + b.uv );
        return result;
    }

    static StaticMeshData Subdivide( StaticMeshData mesh ) {
        u32 numTris = (u32)mesh.indexCount / 3;

        StaticMeshData result = {};
        result.vertexCount = numTris * 6;
        result.vertices = (StaticMeshVertex *)malloc( sizeof( StaticMeshVertex ) * result.vertexCount );
        result.indexCount = numTris * 12;
        result.indices = (u16 *)malloc( sizeof( u16 ) * result.indexCount );

        //       v1
        //       *
        //      / \
        //     /   \
        //  m0*-----*m1
        //   / \   / \
        //  /   \ /   \
        // *-----*-----*
        // v0    m2     v2

        i32 vi = 0;
        i32 ii = 0;
        for( u32 i = 0; i < numTris; ++i ) {
            StaticMeshVertex v0 = mesh.vertices[ mesh.indices[ i * 3 + 0 ] ];
            StaticMeshVertex v1 = mesh.vertices[ mesh.indices[ i * 3 + 1 ] ];
            StaticMeshVertex v2 = mesh.vertices[ mesh.indices[ i * 3 + 2 ] ];

            StaticMeshVertex m0 = ComputeMidPoint( v0, v1 );
            StaticMeshVertex m1 = ComputeMidPoint( v1, v2 );
            StaticMeshVertex m2 = ComputeMidPoint( v0, v2 );

            result.vertices[ vi++ ] = ( v0 ); // 0
            result.vertices[ vi++ ] = ( v1 ); // 1
            result.vertices[ vi++ ] = ( v2 ); // 2
            result.vertices[ vi++ ] = ( m0 ); // 3
            result.vertices[ vi++ ] = ( m1 ); // 4
            result.vertices[ vi++ ] = ( m2 ); // 5

            result.indices[ ii++ ] = ( i * 6 + 0 );
            result.indices[ ii++ ] = ( i * 6 + 3 );
            result.indices[ ii++ ] = ( i * 6 + 5 );
            
            result.indices[ ii++ ] = ( i * 6 + 3 );
            result.indices[ ii++ ] = ( i * 6 + 4 );
            result.indices[ ii++ ] = ( i * 6 + 5 );
            
            result.indices[ ii++ ] = ( i * 6 + 5 );
            result.indices[ ii++ ] = ( i * 6 + 4 );
            result.indices[ ii++ ] = ( i * 6 + 2 );
            
            result.indices[ ii++ ] = ( i * 6 + 3 );
            result.indices[ ii++ ] = ( i * 6 + 1 );
            result.indices[ ii++ ] = ( i * 6 + 4 );
        }

        mesh.Free();

        return result;
    }

    static void SwapWindingOrder( StaticMeshData & mesh ) {
        for( i32 i = 0; i < mesh.indexCount; i += 3 ) {
            u16 tmp = mesh.indices[ i + 1 ];
            mesh.indices[ i + 1 ] = mesh.indices[ i + 2 ];
            mesh.indices[ i + 2 ] = tmp;
        }
    }

    StaticMeshData StaticMeshGeneration::CreateQuad( f32 x, f32 y, f32 w, f32 h, f32 depth ) {
        StaticMeshData meshData = {};

        meshData.vertexCount = 4;
        meshData.vertices = (StaticMeshVertex *)malloc( sizeof( StaticMeshVertex ) * meshData.vertexCount );
        meshData.indexCount = 6;
        meshData.indices = (u16 *)malloc( sizeof( u16 ) * meshData.indexCount );

        meshData.vertices[ 0 ] = CreateVertex( x, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f );
        meshData.vertices[ 1 ] = CreateVertex( x, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
        meshData.vertices[ 2 ] = CreateVertex( x + w, y, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f );
        meshData.vertices[ 3 ] = CreateVertex( x + w, y - h, depth, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f );

        meshData.indices[ 0 ] = 0;
        meshData.indices[ 1 ] = 1;
        meshData.indices[ 2 ] = 2;
        meshData.indices[ 3 ] = 0;
        meshData.indices[ 4 ] = 2;
        meshData.indices[ 5 ] = 3;

        SwapWindingOrder( meshData );

        return meshData;
    }

    StaticMeshData StaticMeshGeneration::CreateBox( f32 width, f32 height, f32 depth, u32 numSubdivisions ) {
        StaticMeshData meshData = {};
        meshData.vertexCount = 24;
        meshData.vertices = (StaticMeshVertex *)malloc( sizeof( StaticMeshVertex ) * meshData.vertexCount );
        meshData.indexCount = 36;
        meshData.indices = (u16 *)malloc( sizeof( u16 ) * meshData.indexCount );

        f32 w2 = 0.5f * width;
        f32 h2 = 0.5f * height;
        f32 d2 = 0.5f * depth;

        // Fill in the front face vertex data.
        meshData.vertices[ 0 ] = CreateVertex( -w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f );
        meshData.vertices[ 1 ] = CreateVertex( -w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
        meshData.vertices[ 2 ] = CreateVertex( +w2, +h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f );
        meshData.vertices[ 3 ] = CreateVertex( +w2, -h2, -d2, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f );

        // Fill in the back face vertex data.
        meshData.vertices[ 4 ] = CreateVertex( -w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f );
        meshData.vertices[ 5 ] = CreateVertex( +w2, -h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f );
        meshData.vertices[ 6 ] = CreateVertex( +w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
        meshData.vertices[ 7 ] = CreateVertex( -w2, +h2, +d2, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f );

        // Fill in the top face vertex data.
        meshData.vertices[ 8 ] = CreateVertex( -w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f );
        meshData.vertices[ 9 ] = CreateVertex( -w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
        meshData.vertices[ 10 ] = CreateVertex( +w2, +h2, +d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f );
        meshData.vertices[ 11 ] = CreateVertex( +w2, +h2, -d2, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f );

        // Fill in the bottom face vertex data.
        meshData.vertices[ 12 ] = CreateVertex( -w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f );
        meshData.vertices[ 13 ] = CreateVertex( +w2, -h2, -d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f );
        meshData.vertices[ 14 ] = CreateVertex( +w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
        meshData.vertices[ 15 ] = CreateVertex( -w2, -h2, +d2, 0.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f );

        // Fill in the left face vertex data.
        meshData.vertices[ 16 ] = CreateVertex( -w2, -h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f );
        meshData.vertices[ 17 ] = CreateVertex( -w2, +h2, +d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f );
        meshData.vertices[ 18 ] = CreateVertex( -w2, +h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f );
        meshData.vertices[ 19 ] = CreateVertex( -w2, -h2, -d2, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f );

        // Fill in the right face vertex data.
        meshData.vertices[ 20 ] = CreateVertex( +w2, -h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f );
        meshData.vertices[ 21 ] = CreateVertex( +w2, +h2, -d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f );
        meshData.vertices[ 22 ] = CreateVertex( +w2, +h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f );
        meshData.vertices[ 23 ] = CreateVertex( +w2, -h2, +d2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f );

        // Fill in the front face index meshData
        meshData.indices[ 0 ] = 0; meshData.indices[ 1 ] = 1; meshData.indices[ 2 ] = 2;
        meshData.indices[ 3 ] = 0; meshData.indices[ 4 ] = 2; meshData.indices[ 5 ] = 3;

        // Fill in the back face index meshData
        meshData.indices[ 6 ] = 4; meshData.indices[ 7 ] = 5; meshData.indices[ 8 ] = 6;
        meshData.indices[ 9 ] = 4; meshData.indices[ 10 ] = 6; meshData.indices[ 11 ] = 7;

        // Fill in the top face index meshData
        meshData.indices[ 12 ] = 8; meshData.indices[ 13 ] = 9; meshData.indices[ 14 ] = 10;
        meshData.indices[ 15 ] = 8; meshData.indices[ 16 ] = 10; meshData.indices[ 17 ] = 11;

        // Fill in the bottom face index meshData
        meshData.indices[ 18 ] = 12; meshData.indices[ 19 ] = 13; meshData.indices[ 20 ] = 14;
        meshData.indices[ 21 ] = 12; meshData.indices[ 22 ] = 14; meshData.indices[ 23 ] = 15;

        // Fill in the left face index meshData
        meshData.indices[ 24 ] = 16; meshData.indices[ 25 ] = 17; meshData.indices[ 26 ] = 18;
        meshData.indices[ 27 ] = 16; meshData.indices[ 28 ] = 18; meshData.indices[ 29 ] = 19;

        // Fill in the right face index meshData
        meshData.indices[ 30 ] = 20; meshData.indices[ 31 ] = 21; meshData.indices[ 32 ] = 22;
        meshData.indices[ 33 ] = 20; meshData.indices[ 34 ] = 22; meshData.indices[ 35 ] = 23;

        for( u32 i = 0; i < numSubdivisions; i++ ) {
            meshData = Subdivide( meshData );
        }

        return meshData;
    }

    StaticMeshData StaticMeshGeneration::CreateSphere( f32 radius, u32 sliceCount, u32 stackCount ) {
        StaticMeshData meshData = {};
        meshData.vertexCount = stackCount * sliceCount + 2;
        meshData.vertices = (StaticMeshVertex *)malloc( sizeof( StaticMeshVertex ) * meshData.vertexCount );
        meshData.indexCount = ( stackCount * sliceCount + 2 ) * ( stackCount * sliceCount + 2 );
        meshData.indices = (u16 *)malloc( sizeof( u16 ) * meshData.indexCount );

        // Poles: note that there will be texture coordinate distortion as there is
        // not a unique point on the texture map to assign to the pole when mapping
        // a rectangular texture onto a sphere.
        StaticMeshVertex topVertex = CreateVertex( 0.0f, +radius, 0.0f, 0.0f, +1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f );
        StaticMeshVertex bottomVertex = CreateVertex( 0.0f, -radius, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f );

        i32 vertexIndexCounter = 0;
        meshData.vertices[ vertexIndexCounter++ ] = topVertex;

        f32 phiStep = glm::pi<f32>() / stackCount;
        f32 thetaStep = 2.0f * glm::pi<f32>() / sliceCount;

        // Compute vertices for each stack ring (do not count the poles as rings).
        for( u32 i = 1; i <= stackCount - 1; i++ ) {
            f32 phi = i * phiStep;

            // Vertices of ring.
            for( u32 j = 0; j <= sliceCount; j++ ) {
                f32 theta = j * thetaStep;

                StaticMeshVertex v = {};

                // spherical to Cartesian
                v.position.x = radius * sinf( phi ) * cosf( theta );
                v.position.y = radius * cosf( phi );
                v.position.z = radius * sinf( phi ) * sinf( theta );

                // Partial derivative of P with respect to theta
                //v.tanget.x = -radius * sinf( phi ) * sinf( theta );
                //v.tanget.y = 0.0f;
                //v.tanget.z = +radius * sinf( phi ) * cosf( theta );

                v.uv.x = theta / ( 2.0f * glm::pi<f32>() );
                v.uv.y = phi / glm::pi<f32>();

                meshData.vertices[ vertexIndexCounter++ ] = v;
            }
        }

        meshData.vertices[ vertexIndexCounter++ ] = bottomVertex;

        //
        // Compute indices for top stack.  The top stack was written first to the vertex buffer
        // and connects the top pole to the first ring.
        //
        i32 indicesIndexCounter = 0;
        for( u32 i = 1; i <= sliceCount; ++i ) {
            meshData.indices[ indicesIndexCounter++ ] = 0;
            meshData.indices[ indicesIndexCounter++ ] = i + 1;
            meshData.indices[ indicesIndexCounter++ ] = i;
        }

        //
        // Compute indices for inner stacks (not connected to poles).
        //

        // Offset the indices to the index of the first vertex in the first ring.
        // This is just skipping the top pole vertex.
        u32 baseIndex = 1;
        u32 ringVertexCount = sliceCount + 1;
        for( u32 i = 0; i < stackCount - 2; i++ ) {
            for( u32 j = 0; j < sliceCount; j++ ) {
                meshData.indices[ indicesIndexCounter++ ] = baseIndex + i * ringVertexCount + j;
                meshData.indices[ indicesIndexCounter++ ] = baseIndex + i * ringVertexCount + j + 1;
                meshData.indices[ indicesIndexCounter++ ] = baseIndex + ( i + 1 ) * ringVertexCount + j;
                meshData.indices[ indicesIndexCounter++ ] = baseIndex + ( i + 1 ) * ringVertexCount + j;
                meshData.indices[ indicesIndexCounter++ ] = baseIndex + i * ringVertexCount + j + 1;
                meshData.indices[ indicesIndexCounter++ ] = baseIndex + ( i + 1 ) * ringVertexCount + j + 1;
            }
        }

        //
        // Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
        // and connects the bottom pole to the bottom ring.
        //
        // South pole vertex was added last.
        u32 southPoleIndex = (u32)vertexIndexCounter - 1;

        // Offset the indices to the index of the first vertex in the last ring.
        baseIndex = southPoleIndex - ringVertexCount;

        for( u32 i = 0; i < sliceCount; ++i ) {
            meshData.indices[ indicesIndexCounter++ ] = southPoleIndex;
            meshData.indices[ indicesIndexCounter++ ] = baseIndex + i;
            meshData.indices[ indicesIndexCounter++ ] = baseIndex + i + 1;
        }

        return meshData;
    }

    StaticMeshData StaticMeshGeneration::CreateGeosphere( f32 radius, u32 numSubdivisions ) {
        return {};
    }

    StaticMeshData StaticMeshGeneration::CreateCylinder( f32 bottomRadius, f32 topRadius, f32 height, u32 sliceCount, u32 stackCount ) {
        return {};
    }

    static i32 CompareVerticesClockwise( void * context, const void * a, const void * b ) {
        const glm::vec2 * va = static_cast<const glm::vec2 *>( a );
        const glm::vec2 * vb = static_cast<const glm::vec2 *>( b );
        glm::vec2 center = *static_cast<glm::vec2 *>( context );
        glm::vec2 aDir = *va - center;
        glm::vec2 bDir = *vb - center;
        return std::atan2( aDir.y, aDir.x ) < std::atan2( bDir.y, bDir.x ) ? -1 : 1;
    }

    void GeometryFuncs::SortPointsIntoClockWiseOrder( glm::vec2 * points, i32 numPoints ) {
        glm::vec2 centroid = glm::vec2( 0 );
        for( i32 pointIndex = 0; pointIndex < numPoints; pointIndex++ ) {
            centroid += points[ pointIndex ];
        }

        centroid /= numPoints;

        qsort_s( points, numPoints, sizeof( glm::vec2 ), CompareVerticesClockwise, &centroid );
    }

}
