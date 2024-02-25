#include "atto_content.h"

#include <vector> // @NOTE(DECLAN): Going to use std vector because none of this code should run in release

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/std_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace atto {

    ContentTextureProcessor::ContentTextureProcessor() {

    }

    ContentTextureProcessor::~ContentTextureProcessor() {
        if( pixelData != nullptr ) {
            free( pixelData );
        }
    }

    bool ContentTextureProcessor::LoadFromFile( const char * file ) {
        filePath = StringFormat::Large( "res/sprites/%s", file );
        pixelData = stbi_load( filePath.GetCStr(), &width, &height, &channels, 4 );

        if( pixelData == nullptr ) {
            return false;
        }


        return true;
    }


    void ContentTextureProcessor::MakeAlphaEdge() {
        if( channels != 4 ) {
            return;
        }

        bool needsBorder = false;
        for( int y = 0; y < height; y++ ) {
            for( int x = 0; x < width; x++ ) {
                u8 r;
                u8 g;
                u8 b;
                u8 a;
                GetPixel( x, y, r, g, b, a );

                if( a != 0 ) {
                    needsBorder = true;
                }
            }
        }

        if( needsBorder == false ) {
            return;
        }

        const i32 newWidth = width + 2;
        const i32 newHeight = height + 2;
        const i32 newSize = newWidth * newHeight * 4;
        byte * newData = (byte *)malloc( newSize );

        if( newData == nullptr ) {
            INVALID_CODE_PATH;
            return;
        }

        memset( newData, 0, newSize );

        for( int y = 1; y < newHeight - 1; y++ ) {
            for( int x = 1; x < newWidth - 1; x++ ) {
                u8 r;
                u8 g;
                u8 b;
                u8 a;
                GetPixel( x - 1, y - 1, r, g, b, a );

                newData[ ( y * newWidth + x ) * 4 + 0 ] = r;
                newData[ ( y * newWidth + x ) * 4 + 1 ] = g;
                newData[ ( y * newWidth + x ) * 4 + 2 ] = b;
                newData[ ( y * newWidth + x ) * 4 + 3 ] = a;
            }
        }

        free( pixelData );

        pixelData = newData;
        width = newWidth;
        height = newHeight;
    }

    void ContentTextureProcessor::FixAplhaEdges() {
        const int sizeBytes = width * height * 4;

        // copy data into vec
        std::vector<byte> dcopy( sizeBytes );
        memcpy( dcopy.data(), pixelData, sizeBytes );

        const byte * srcptr = dcopy.data();
        byte * data_ptr = pixelData;

        const int max_radius = 4;
        const int alpha_threshold = 20;
        const int max_dist = 0x7FFFFFFF;

        for( int i = 0; i < height; i++ ) {
            for( int j = 0; j < width; j++ ) {
                const byte * rptr = &srcptr[ ( i * width + j ) * 4 ];
                byte * wptr = &data_ptr[ ( i * width + j ) * 4 ];

                if( rptr[ 3 ] >= alpha_threshold ) {
                    continue;
                }

                int closest_dist = max_dist;
                byte closest_color[ 3 ] = { 0 };

                int from_x = Max( 0, j - max_radius );
                int to_x = Min( width - 1, j + max_radius );
                int from_y = Max( 0, i - max_radius );
                int to_y = Min( height - 1, i + max_radius );

                for( int k = from_y; k <= to_y; k++ ) {
                    for( int l = from_x; l <= to_x; l++ ) {
                        int dy = i - k;
                        int dx = j - l;
                        int dist = dy * dy + dx * dx;
                        if( dist >= closest_dist ) {
                            continue;
                        }

                        const byte * rp2 = &srcptr[ ( k * width + l ) << 2 ];

                        if( rp2[ 3 ] < alpha_threshold ) {
                            continue;
                        }

                        closest_dist = dist;
                        closest_color[ 0 ] = rp2[ 0 ];
                        closest_color[ 1 ] = rp2[ 1 ];
                        closest_color[ 2 ] = rp2[ 2 ];
                    }
                }

                if( closest_dist != max_dist ) {
                    wptr[ 0 ] = closest_color[ 0 ];
                    wptr[ 1 ] = closest_color[ 1 ];
                    wptr[ 2 ] = closest_color[ 2 ];
                }
            }
        }
    }

    void ContentTextureProcessor::GetPixel( i32 x, i32 y, u8 & r, u8 & g, u8 & b, u8 & a ) {
        Assert( x < width && x >= 0 );
        Assert( y < height && y >= 0 );

        i32 offset = ( y * width + x ) * 4;
        r = pixelData[ offset + 0 ];
        g = pixelData[ offset + 1 ];
        b = pixelData[ offset + 2 ];
        a = pixelData[ offset + 3 ];
    }

    ContentModelProcessor::ContentModelProcessor() {

    }

    ContentModelProcessor::~ContentModelProcessor() {

    }

    bool ContentModelProcessor::LoadFromFile( Core * core, const char * file ) {
        filePath = StringFormat::Large( "res/models/%s", file );

        Assimp::Importer importer;
        const aiScene * scene = importer.ReadFile( filePath.GetCStr(), aiProcess_Triangulate | aiProcess_FlipUVs );

        if( !scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode ) {
            core->LogOutput( LogLevel::ERR, "%s", importer.GetErrorString() );
            return false;
        }

        ProcessNode( scene->mRootNode, scene );

        return true;
    }

    void ContentModelProcessor::ComputeBoundingBox() {
        const i32 meshCount = (i32)meshes.size();
        boundingBox = {};
        for( i32 meshIndex = 0; meshIndex < meshCount; meshIndex++ ) {
            const StaticMeshData & mesh = meshes[ meshIndex ];
            for( i32 vertexIndex = 0; vertexIndex < mesh.vertexCount; vertexIndex++ ) {
                const StaticMeshVertex & v = mesh.vertices[ vertexIndex ];
                boundingBox.Expand( v.position );
            }
        }
    }

    void ContentModelProcessor::ProcessNode( aiNode * node, const aiScene * scene ) {
        for( u32 i = 0; i < node->mNumMeshes; i++ ) {
            aiMesh * mesh = scene->mMeshes[ node->mMeshes[ i ] ];
            meshes.push_back( ProcessMesh( mesh, scene ) );
        }
        for( u32 i = 0; i < node->mNumChildren; i++ ) {
            ProcessNode( node->mChildren[ i ], scene );
        }
    }

    StaticMeshData ContentModelProcessor::ProcessMesh( aiMesh * mesh, const aiScene * scene ) {
        StaticMeshData resultingMesh = {};
        resultingMesh.name = mesh->mName.C_Str();
        resultingMesh.vertices = (StaticMeshVertex *)malloc( sizeof( StaticMeshVertex ) * mesh->mNumVertices );
        resultingMesh.indices = (u16 *)malloc( sizeof( u16 ) * mesh->mNumFaces * 3 );

        //Mat3f scaleMatrix = ScaleCardinal( Mat3f( 1.0f ), Vec3f( metaData.scale ) );

        for( u32 i = 0; i < mesh->mNumVertices; i++ ) {
            glm::vec3 pos = glm::vec3( 0 );
            glm::vec3 normal = glm::vec3( 0 );
            glm::vec2 uv = glm::vec3( 0 );

            pos.x = mesh->mVertices[ i ].x;
            pos.y = mesh->mVertices[ i ].y;
            pos.z = mesh->mVertices[ i ].z;

            if( mesh->HasNormals() ) {
                normal.x = mesh->mNormals[ i ].x;
                normal.y = mesh->mNormals[ i ].y;
                normal.z = mesh->mNormals[ i ].z;
            }

            if( mesh->mTextureCoords[ 0 ] )
            {
                uv.x = mesh->mTextureCoords[ 0 ][ i ].x;
                uv.y = mesh->mTextureCoords[ 0 ][ i ].y;

                // tangent
                //vector.x = mesh->mTangents[ i ].x;
                //vector.y = mesh->mTangents[ i ].y;
                //vector.z = mesh->mTangents[ i ].z;
                //vertex.tangent = vector;
                //
                //// bi-tangent
                //vector.x = mesh->mBitangents[ i ].x;
                //vector.y = mesh->mBitangents[ i ].y;
                //vector.z = mesh->mBitangents[ i ].z;
                //vertex.bitangent = vector;
            }

            StaticMeshVertex vert = {};
            vert.position = pos;
            vert.normal = normal;
            vert.uv = uv;

            resultingMesh.vertices[ resultingMesh.vertexCount ] = vert;
            resultingMesh.vertexCount++;
        }

        for( u32 i = 0; i < mesh->mNumFaces; i++ ) {
            aiFace face = mesh->mFaces[ i ];
            for( u32 j = 0; j < face.mNumIndices; j++ ) {
                Assert( face.mNumIndices == 3 );
                resultingMesh.indices[ resultingMesh.indexCount ] = SafeTruncateU32( face.mIndices[ j ] );
                resultingMesh.indexCount++;
            }
        }

        return resultingMesh;
    }

}