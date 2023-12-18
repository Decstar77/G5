#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static i32 YSortEntities( Entity *& a, Entity *& b ) {
        return (i32)( b->pos.y - a->pos.y );
    }

    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    void GameModeGame::Init( Core * core ) {
        grid_Dark1 = core->ResourceGetAndLoadTexture( "kenney_prototype/dark/texture_01.png" );

        localPlayer = SpawnPlayer( glm::vec3( 0, 0, 3 ) );

        f32 dim = 5.0f;
        MapTriangle t1 = {};
        t1.p1 = glm::vec3( -1, 0, 1 ) * dim;
        t1.p2 = glm::vec3( -1, 0, -1 ) * dim;
        t1.p3 = glm::vec3( 1, 0, -1 ) * dim;
        t1.uv1 = glm::vec2( 0, 1 );
        t1.uv2 = glm::vec2( 0, 0 );
        t1.uv3 = glm::vec2( 1, 0 );
        t1.ComputeNormal();

        MapTriangle t2 = {};
        t2.p1 = glm::vec3( 1, 0, 1 ) * dim;
        t2.p2 = glm::vec3( -1, 0, 1 ) * dim;
        t2.p3 = glm::vec3( 1, 0, -1 ) * dim;
        t2.uv1 = glm::vec2( 1, 1 );
        t2.uv2 = glm::vec2( 0, 1 );
        t2.uv3 = glm::vec2( 1, 0 );
        t2.ComputeNormal();

        map.triangles.Add( t1 );
        map.triangles.Add( t2 );

        map.mapWidth = 10;
        map.mapHeight = 10;
        map.AddBlock( 0, 0 );

        map.Bake();
    }

    void GameModeGame::UpdateAndRender( Core * core, f32 dt ) {
        EntityUpdatePlayer( core, localPlayer );

        DrawContext * worldDraws = core->RenderGetDrawContext( 0 );
        const i32 mapTriangleCount = map.triangles.GetCount();
        for( i32 i = 0; i < mapTriangleCount; ++i ) {
            MapTriangle & t = map.triangles[ i ];
            worldDraws->DrawTriangle( t.p1, t.p2, t.p3, t.uv1, t.uv2, t.uv3, grid_Dark1 );
        }


        worldDraws->SetCamera( localPlayer->CameraGetViewMatrix(), localPlayer->camera.yfov, localPlayer->camera.zNear, localPlayer->camera.zFar );
        //worldDraws->DrawRect( glm::vec2( 0 ), glm::vec2( 100 ), 0.0f, glm::vec4( 1, 0, 0, 1 ) );
        //worldDraws->DrawPlane( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 0, 1 ), glm::vec2( 1 ), glm::vec4( 0, 1, 0, 1 ) );
        //worldDraws->DrawPlane( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ), glm::vec2( 10 ), glm::vec4( 0.4f, 0.4f, 0.2f, 1 ) );
        //worldDraws->DrawSphere( glm::vec3( 0, 3, 0 ), 1.0f );
        //worldDraws->DrawTriangle( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ), glm::vec3( 1, 0, 0 ), glm::vec4( 1, 0, 0, 1 ) );
        core->RenderSubmit( worldDraws, true );
    }

    void GameModeGame::Shutdown( Core * core ) {
    }

    Entity * GameModeGame::SpawnEntity( EntityType type ) {
        EntityHandle handle;
        Entity * ent = entityPool.Add( handle );
        if( ent != nullptr ) {
            ZeroStructPtr( ent );
            ent->handle = handle;
        }

        return ent;
    }

    Entity * GameModeGame::SpawnPlayer( glm::vec3 pos ) {
        Entity * ent = SpawnEntity( ENTITY_TYPE_PLAYER );
        if( ent != nullptr ) {
            const f32 YAW = -90.0f;
            const f32 PITCH = 0.0f;
            const f32 SPEED = 2.5f;
            const f32 SENSITIVITY = 0.1f;
            const f32 ZOOM = 45.0f;
            ent->pos = pos;
            ent->camera.yaw = YAW;
            ent->camera.pitch = PITCH;
            ent->camera.movementSpeed = SPEED;
            ent->camera.mouseSensitivity = SENSITIVITY;
            ent->camera.yfov = ZOOM;
            ent->camera.zNear = 0.1f;
            ent->camera.zFar = 250.0f;
            ent->camera.front = glm::vec3( 0.0f, 0.0f, -1.0f );
            ent->camera.up = glm::vec3( 0.0f, 1.0f, 0.0f );
            ent->hasCollision = true;
            ent->collisionCollider.type = COLLIDER_TYPE_SHPERE;
            ent->collisionCollider.sphere.r = 1.0f;
            ent->collisionCollider.sphere.c = ent->pos;
        }

        return ent;
    }

    void GameModeGame::EntityUpdatePlayer( Core * core, Entity * ent ) {
        if( ent->sleeping == true ) {
            return;
        }

        if( ent->camera.noclip == true ) {
        //if ( true ) {
            if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
                core->InputDisableMouse();
                return;
            }

            if( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
                core->InputEnableMouse();
                return;
            }

            if( core->InputMouseButtonDown( MOUSE_BUTTON_2 ) == true ) {
                f32 v = ent->camera.movementSpeed * core->GetDeltaTime();
                if( core->InputKeyDown( KEY_CODE_W ) ) {
                    ent->pos += ent->camera.front * v;
                }
                if( core->InputKeyDown( KEY_CODE_S ) ) {
                    ent->pos -= ent->camera.front * v;
                }
                if( core->InputKeyDown( KEY_CODE_A ) ) {
                    ent->pos -= ent->camera.right * v;
                }
                if( core->InputKeyDown( KEY_CODE_D ) ) {
                    ent->pos += ent->camera.right * v;
                }
                if( core->InputKeyDown( KEY_CODE_SPACE ) ) {
                    ent->pos.y += v;
                }
                if( core->InputKeyDown( KEY_CODE_LEFT_CONTROL ) ) {
                    ent->pos.y -= v;
                }

                glm::vec2 mouseDelta = core->InputMouseDeltaPixels();
                f32 xoffset = mouseDelta.x * ent->camera.mouseSensitivity;
                f32 yoffset = mouseDelta.y * ent->camera.mouseSensitivity;
                ent->camera.yaw += xoffset;
                ent->camera.pitch += yoffset;
                if( ent->camera.pitch > 89.0f ) {
                    ent->camera.pitch = 89.0f;
                }
                if( ent->camera.pitch < -89.0f ) {
                    ent->camera.pitch = -89.0f;
                }

                glm::vec3 front;
                front.x = cos( glm::radians( ent->camera.yaw ) ) * cos( glm::radians( ent->camera.pitch ) );
                front.y = sin( glm::radians( ent->camera.pitch ) );
                front.z = sin( glm::radians( ent->camera.yaw ) ) * cos( glm::radians( ent->camera.pitch ) );
                ent->camera.front = glm::normalize( front );
                ent->camera.right = glm::normalize( glm::cross( ent->camera.front, glm::vec3( 0, 1, 0 ) ) );
                ent->camera.up = glm::normalize( glm::cross( ent->camera.right, ent->camera.front ) );
            }
        }
        else {
            if( core->InputIsMouseDisabled() == false ) {
                core->InputDisableMouse();
                return;
            }

            // Camera
            glm::vec2 mouseDelta = core->InputMouseDeltaPixels();
            f32 xoffset = mouseDelta.x * ent->camera.mouseSensitivity;
            f32 yoffset = mouseDelta.y * ent->camera.mouseSensitivity;
            ent->camera.yaw += xoffset;
            ent->camera.pitch += yoffset;
            if( ent->camera.pitch > 89.0f ) {
                ent->camera.pitch = 89.0f;
            }
            if( ent->camera.pitch < -89.0f ) {
                ent->camera.pitch = -89.0f;
            }

            glm::vec3 front;
            front.x = cos( glm::radians( ent->camera.yaw ) ) * cos( glm::radians( ent->camera.pitch ) );
            front.y = sin( glm::radians( ent->camera.pitch ) );
            front.z = sin( glm::radians( ent->camera.yaw ) ) * cos( glm::radians( ent->camera.pitch ) );
            ent->camera.front = glm::normalize( front );
            ent->camera.right = glm::normalize( glm::cross( ent->camera.front, glm::vec3( 0, 1, 0 ) ) );
            ent->camera.up = glm::normalize( glm::cross( ent->camera.right, ent->camera.front ) );

            const f32 dt = core->GetDeltaTime();
            const f32 v = 1400.0f * dt;

            glm::mat3 ori = ent->camera.GetOrientation();
            ori[ 0 ].y = 0;
            ori[ 0 ] = glm::normalize( ori[ 0 ] );
            ori[ 1 ] = glm::vec3( 0, 1, 0 );
            ori[ 2 ].y = 0;
            ori[ 2 ] = glm::normalize( ori[ 2 ] );

            glm::vec3 right = ori[ 0 ];
            glm::vec3 forward = ori[ 2 ];

            glm::vec3 acc = glm::vec3( 0 );
            if( core->InputKeyDown( KEY_CODE_W ) ) {
                acc += forward * v;
            }
            if( core->InputKeyDown( KEY_CODE_S ) ) {
                acc -= forward * v;
            }
            if( core->InputKeyDown( KEY_CODE_A ) ) {
                acc -= right * v;
            }
            if( core->InputKeyDown( KEY_CODE_D ) ) {
                acc += right * v;
            }

            const f32 resistance = 14.0f;

            // @NOTE: Linear drag
            acc.x += -resistance * ent->vel.x;
            acc.z += -resistance * ent->vel.z;

            ent->vel += acc * dt;
            ent->pos += ent->vel * dt;

            // @NOTE: Collision with map
            Collider plane = {};
            plane.type = COLLIDER_TYPE_PLANE;
            plane.plane.c = glm::vec3( 0 );
            plane.plane.n = glm::vec3( 0, 0, 1 );

            ent->collisionCollider.sphere.c = ent->pos;

            Manifold m = {};
            if( CollisionCheck_SphereVsPlane( ent->collisionCollider, plane, m ) == true ) {
                ent->pos += m.normal * m.penetration;
            }

        }
    }

    glm::mat4 Entity::CameraGetViewMatrix() const {
        glm::vec3 headPos = pos + glm::vec3( 0, 1.8f, 0 );
        return glm::lookAt( headPos, headPos + camera.front, camera.up );
    }

    glm::mat3 EntCamera::GetOrientation() const {
        glm::mat3 orientation;
        orientation[ 0 ] = right;
        orientation[ 1 ] = up;
        orientation[ 2 ] = front;
        return orientation;
    }

    bool Map::AddBlock( i32 x, i32 y ) {
        if( x < 0 || x >= mapWidth ) { return false; }
        if( y < 0 || y >= mapHeight ) { return false; }

        MapBlock block = {};
        block.filled = true;
        block.xIndex = x;
        block.yIndex = y;
        block.flatIndex = PosIndexToFlatIndex( block.xIndex, block.yIndex, mapWidth );
        block.bottomLeftWS = glm::vec2( x, y ) * BlockDim;
        block.topRightWS = block.bottomLeftWS + glm::vec2( BlockDim, BlockDim );
        blocks[ block.flatIndex ] = block;

        return true;
    }

    bool Map::RemoveBlock( i32 x, i32 y ) {
        if( x < 0 || x >= mapWidth ) { return false; }
        if( y < 0 || y >= mapHeight ) { return false; }

        i32 flatIndex = PosIndexToFlatIndex( x, y, mapWidth );
        blocks[ flatIndex ] = {};
        return true;
    }

    void Map::AddWall( glm::vec2 p1, glm::vec2 p2, bool invertNormal ) {
        MapTriangle t1 = {};
        t1.p1 = glm::vec3( p1.x, BlockDim, p1.y );
        t1.p2 = glm::vec3( p1.x, 0, p1.y );
        t1.p3 = glm::vec3( p2.x, 0, p2.y );
        t1.uv1 = glm::vec2( 0, 1 );
        t1.uv2 = glm::vec2( 0, 0 );
        t1.uv3 = glm::vec2( 1, 0 );
        t1.ComputeNormal();

        MapTriangle t2 = {};
        t2.p1 = glm::vec3( p2.x, BlockDim, p2.y );
        t2.p2 = glm::vec3( p1.x, BlockDim, p1.y );
        t2.p3 = glm::vec3( p2.x, 0, p2.y );
        t2.uv1 = glm::vec2( 1, 1 );
        t2.uv2 = glm::vec2( 0, 1 );
        t2.uv3 = glm::vec2( 1, 0 );
        t2.ComputeNormal();

        if( invertNormal ) {
            //t1.InvertNormal();
            //t2.InvertNormal();
        }

        triangles.Add( t1 );
        triangles.Add( t2 );
    }

    void Map::AddFloor( glm::vec2 p1, glm::vec2 p2, i32 level ) {
        MapTriangle t1 = {};
        t1.p1 = glm::vec3( p1.x, BlockDim * level,  p2.y );
        t1.p2 = glm::vec3( p1.x, BlockDim * level,  p1.y );
        t1.p3 = glm::vec3( p2.x, BlockDim * level,  p1.y );
        t1.uv1 = glm::vec2( 0, 1 );
        t1.uv2 = glm::vec2( 0, 0 );
        t1.uv3 = glm::vec2( 1, 0 );
        t1.ComputeNormal();

        MapTriangle t2 = {};
        t2.p1 = glm::vec3( p2.x, BlockDim * level,  p2.y );
        t2.p2 = glm::vec3( p1.x, BlockDim * level,  p2.y );
        t2.p3 = glm::vec3( p2.x, BlockDim * level,  p1.y );
        t2.uv1 = glm::vec2( 1, 1 );
        t2.uv2 = glm::vec2( 0, 1 );
        t2.uv3 = glm::vec2( 1, 0 );
        t2.ComputeNormal();

        triangles.Add( t1 );
        triangles.Add( t2 );
    }

    void Map::Bake() {
        triangles.Clear( true );

        const i32 mapBlockCount = blocks.GetCapcity();
        for( i32 blockIndex = 0; blockIndex < mapBlockCount; blockIndex++ ) {
            const MapBlock & b = blocks[ blockIndex ];
            if( b.filled == true ) {
                const i32 xIndex = b.xIndex;
                const i32 yIndex = b.yIndex;
                const i32 flatIndex = b.flatIndex;

                const i32 xIndexLeft = xIndex - 1;
                const i32 xIndexRight = xIndex + 1;
                const i32 yIndexDown = yIndex - 1;
                const i32 yIndexUp = yIndex + 1;

                const i32 flatIndexLeft = PosIndexToFlatIndex( xIndexLeft, yIndex, mapWidth );
                const i32 flatIndexRight = PosIndexToFlatIndex( xIndexRight, yIndex, mapWidth );
                const i32 flatIndexDown = PosIndexToFlatIndex( xIndex, yIndexDown, mapWidth );
                const i32 flatIndexUp = PosIndexToFlatIndex( xIndex, yIndexUp, mapWidth );

                if( xIndexLeft < 0 || ( xIndexLeft >= 0 && blocks[ flatIndexLeft ].filled == false ) ) {
                    AddWall( b.bottomLeftWS, b.bottomLeftWS + glm::vec2( 0, BlockDim ), true );
                }

                if( xIndexRight >= mapWidth || ( xIndexRight < mapWidth && blocks[ flatIndexRight ].filled == false ) ) {
                    AddWall( b.bottomLeftWS + glm::vec2( BlockDim, 0 ), b.bottomLeftWS + glm::vec2( BlockDim, BlockDim ), false );
                }

                if( yIndexDown < 0 || ( yIndexDown >= 0 && blocks[ flatIndexDown ].filled == false ) ) {
                    AddWall( b.bottomLeftWS, b.bottomLeftWS + glm::vec2( BlockDim, 0 ), false );
                }

                if( yIndexUp >= mapHeight || ( yIndexUp < mapHeight && blocks[ flatIndexUp ].filled == false ) ) {
                    AddWall( b.bottomLeftWS + glm::vec2( 0, BlockDim ), b.bottomLeftWS + glm::vec2( BlockDim, BlockDim ), true );
                }

                AddFloor( b.bottomLeftWS, b.topRightWS, 0 );
                AddFloor( b.bottomLeftWS, b.topRightWS, 1 );
            }
        }
    }

}
