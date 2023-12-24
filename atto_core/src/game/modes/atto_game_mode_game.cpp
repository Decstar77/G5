#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static i32 YSortEntities( Entity *& a, Entity *& b ) {
        return (i32)( b->pos.y - a->pos.y );
    }

    GameModeType GameModeGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameModeGame::IsInitialized() {
        return localPlayer != nullptr;
    }

    void GameModeGame::Initialize( Core * core ) {
        grid_Dark1 = core->ResourceGetAndLoadTexture( "kenney_prototype/dark/texture_01.png", true, true );
        grid_Dark8 = core->ResourceGetAndLoadTexture( "kenney_prototype/dark/texture_08.png", true, true );
        tex_PolygonScifi_01_C = core->ResourceGetAndLoadTexture( "PolygonScifi_01_C.png", false, false );
        snd_Gun_Pistol_Shot_01 = core->ResourceGetAndLoadAudio( "gun_pistol_shot_02.wav" );

        mesh_Test = core->ResourceGetAndLoadMesh( "wep_pistol_bot.obj" );

        map.playerStartPos = glm::vec3( 0, 0, 3 );

        localPlayer = SpawnPlayer( map.playerStartPos );
        localPlayer->pos.x = -2;
        f32 dim = 5.0f;
        MapTriangle t1 = {};
        t1.p1 = glm::vec3( -1, 0, 1 ) * dim;
        t1.p2 = glm::vec3( -1, 0, -1 ) * dim;
        t1.p3 = glm::vec3( 1, 0, -1 ) * dim;
        t1.uv1 = glm::vec2( 0, 1 );
        t1.uv2 = glm::vec2( 0, 0 );
        t1.uv3 = glm::vec2( 1, 0 );
        t1.ComputeNormal();
        t1.InvertNormal();

        MapTriangle t2 = {};
        t2.p1 = glm::vec3( 1, 0, 1 ) * dim;
        t2.p2 = glm::vec3( -1, 0, 1 ) * dim;
        t2.p3 = glm::vec3( 1, 0, -1 ) * dim;
        t2.uv1 = glm::vec2( 1, 1 );
        t2.uv2 = glm::vec2( 0, 1 );
        t2.uv3 = glm::vec2( 1, 0 );
        t2.ComputeNormal();
        t2.InvertNormal();

        map.triangles.Add( t1 );
        map.triangles.Add( t2 );

        map.mapWidth = 10;
        map.mapHeight = 10;
        map.AddBlock( 0, 0 );

        map.Bake();
    }

    void GameModeGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        if( ( flags & UPDATE_AND_RENDER_FLAG_NO_UPDATE ) == false ) {
            EntityUpdatePlayer( core, localPlayer );
        }

        DrawContext * worldDraws = core->RenderGetDrawContext( 0 );
        worldDraws->SetCamera( localPlayer->CameraGetViewMatrix(), localPlayer->camera.yfov, localPlayer->camera.zNear, localPlayer->camera.zFar );

        const i32 mapTriangleCount = map.triangles.GetCount();
        for( i32 i = 0; i < mapTriangleCount; ++i ) {
            MapTriangle & t = map.triangles[ i ];
            if( glm::abs( glm::dot( t.normal, glm::vec3( 0, 1, 0 ) ) ) > 0.9f ) {
                worldDraws->DrawTriangle( t.p1, t.p2, t.p3, t.uv1, t.uv2, t.uv3, grid_Dark8 );
            }
            else {
                worldDraws->DrawTriangle( t.p1, t.p2, t.p3, t.uv1, t.uv2, t.uv3, grid_Dark1 );
            }

            glm::vec3 c = t.GetCenter();
            //worldDraws->DrawLine( c, c + t.normal, 1.0f, glm::vec4( 1, 0, 0, 1 ) );
        }

        worldDraws->DrawSphere( localPlayer->pos, localPlayer->collisionCollider.sphere.r );
        worldDraws->DrawMesh( mesh_Test, localPlayer->Player_ComputeGunTransformMatrix(), tex_PolygonScifi_01_C );
        //worldDraws->DrawRect( glm::vec2( 0 ), glm::vec2( 100 ), 0.0f, glm::vec4( 1, 0, 0, 1 ) );
        //worldDraws->DrawPlane( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 0, 1 ), glm::vec2( 1 ), glm::vec4( 0, 1, 0, 1 ) );
        //worldDraws->DrawPlane( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ), glm::vec2( 10 ), glm::vec4( 0.4f, 0.4f, 0.2f, 1 ) );
        //worldDraws->DrawSphere( glm::vec3( 0, 3, 0 ), 1.0f );
        //worldDraws->DrawTriangle( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ), glm::vec3( 1, 0, 0 ), glm::vec4( 1, 0, 0, 1 ) );
        //worldDraws->DrawLine( glm::vec3( 0, 0, 0 ), glm::vec3( 1, 1, 1 ), 1 );
        const f32 crossHairThicc = 0.0045f;
        const f32 crossHairLength = 0.02f;
        const f32 aspectRatio = worldDraws->GetMainAspectRatio();
        worldDraws->DrawLine2D_NDC( glm::vec2( 0, -crossHairLength ), glm::vec2( 0, crossHairLength ), crossHairThicc, glm::vec4( 1, 0, 0, 1 ) );
        worldDraws->DrawLine2D_NDC( glm::vec2( -crossHairLength / aspectRatio, 0 ), glm::vec2( crossHairLength /aspectRatio, 0 ), crossHairThicc * aspectRatio, glm::vec4( 1, 0, 0, 1 ) );

        if( ( flags & UPDATE_AND_RENDER_FLAG_DONT_SUBMIT_RENDER ) == false ) {
            core->RenderSubmit( worldDraws, true );
        }
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
            ent->camera = EntCamera::CreateDefault();
            ent->hasCollision = true;
            ent->collisionCollider.type = COLLIDER_TYPE_SHPERE;
            ent->collisionCollider.sphere.r = 0.5f;
            ent->collisionCollider.sphere.c = pos + glm::vec3( 0, ent->collisionCollider.sphere.r, 0 );
            ent->pos = ent->collisionCollider.sphere.c;
            ent->fireRate = 0.25f;
            ent->localStartGunPos = glm::vec3( 0.5, -0.4f, 1.2f );
            ent->localGunPos = ent->localStartGunPos;
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


            glm::mat3 ori = ent->camera.GetOrientation();
            ori[ 0 ].y = 0;
            ori[ 0 ] = glm::normalize( ori[ 0 ] );
            ori[ 1 ] = glm::vec3( 0, 1, 0 );
            ori[ 2 ].y = 0;
            ori[ 2 ] = glm::normalize( ori[ 2 ] );

            glm::vec3 right = ori[ 0 ];
            glm::vec3 forward = ori[ 2 ];

            const f32 tickTime = 0.01f; // 100Hz
            const i32 maxTickIterations = 3;
            const f32 dt = core->GetDeltaTime();
            dtAccumulator += dt;
            i32 tickIteration = 0;
            while( dtAccumulator >= tickTime && tickIteration < maxTickIterations ) {
                dtAccumulator -= tickTime;
                tickIteration++;

                const f32 v = 6000.0f * tickTime;

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

                ent->vel += acc * tickTime;
                ent->pos += ent->vel * tickTime;

                ent->collisionCollider.sphere.c = ent->pos;
                const i32 triCount = map.triangles.GetCount();
                for( i32 triIndex = 0; triIndex < triCount; triIndex++ ) {
                    MapTriangle & tri = map.triangles[ triIndex ];
                    Manifold m = {};
                    if( CollisionCheck_SphereVsTri( ent->pos, ent->collisionCollider.sphere.r, tri.p1, tri.p2, tri.p3, tri.normal, m ) == true ) {
                        ent->pos += m.normal * m.penetration;
                    }
                }

                // @HACK: Lock the player to the ground
                ent->pos.y = ent->collisionCollider.sphere.c.y;

                ent->fireRateAccumulator -= tickTime;

                if( ent->fireRateAccumulator <= 0.0f ) {
                    ent->fireRateAccumulator = 0.0f;
                    ent->localGunPos = ent->localStartGunPos;
                }

                if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) ) {
                    if( ent->fireRateAccumulator == 0.0f ) {
                        ent->fireRateAccumulator = ent->fireRate;
                        ent->localGunPos.z += 2.1f;
                        core->AudioPlay( snd_Gun_Pistol_Shot_01, 0.5f );
                    }
                }
            }

            if( ent->fireRateAccumulator > 0.0f ) {
                ent->localGunPos = glm::mix( ent->localStartGunPos, ent->localGunPos, ent->fireRateAccumulator );
            }
        }
    }

    glm::mat4 Entity::CameraGetViewMatrix() const {
        glm::vec3 headPos = pos + glm::vec3( 0, 1.6f, 0 );
        return glm::lookAt( headPos, headPos + camera.front, camera.up );
    }

    glm::mat4 Entity::Player_ComputeGunTransformMatrix() const {
        glm::mat4 localGunTransform = glm::translate( glm::mat4( 1 ), localGunPos );

        glm::vec3 headPos = pos + glm::vec3( 0, 1.6f, 0 );
        glm::mat4 headTransform = glm::translate( glm::mat4( 1 ), headPos );
        headTransform[ 0 ] = glm::vec4( camera.right, 0 );
        headTransform[ 1 ] = glm::vec4( camera.up, 0 );
        headTransform[ 2 ] = glm::vec4( camera.front, 0 );

        glm::mat4 worldGunTransform = headTransform * localGunTransform;

        return worldGunTransform;
    }

    glm::mat3 EntCamera::GetOrientation() const {
        glm::mat3 orientation;
        orientation[ 0 ] = right;
        orientation[ 1 ] = up;
        orientation[ 2 ] = front;
        return orientation;
    }

    EntCamera EntCamera::CreateDefault() {
        const f32 YAW = -90.0f;
        const f32 PITCH = 0.0f;
        const f32 SPEED = 2.5f;
        const f32 SENSITIVITY = 0.1f;
        const f32 ZOOM = 45.0f;
        EntCamera camera = {};
        camera.yaw = YAW;
        camera.pitch = PITCH;
        camera.movementSpeed = SPEED;
        camera.mouseSensitivity = SENSITIVITY;
        camera.yfov = ZOOM;
        camera.zNear = 0.1f;
        camera.zFar = 250.0f;
        camera.front = glm::vec3( 0.0f, 0.0f, -1.0f );
        camera.up = glm::vec3( 0.0f, 1.0f, 0.0f );
        return camera;
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
            t1.InvertNormal();
            t2.InvertNormal();
        }

        triangles.Add( t1 );
        triangles.Add( t2 );
    }

    bool Map::LoadFromMapFile( MapFile * mapFile ) {
        mapWidth = mapFile->mapWidth;
        mapHeight = mapFile->mapHeight;
        playerStartPos = mapFile->playerStartPos;
        const i32 blockCount = mapFile->blocks.GetCount();
        for( i32 blockIndex = 0; blockIndex < blockCount; blockIndex++ ) {
            MapFileBlock & block = mapFile->blocks[ blockIndex ];
            AddBlock( block.xIndex, block.yIndex );
        }

        Bake();

        return true;
    }

    bool Map::SaveToMapFile( MapFile * mapFile ) {
        ZeroStructPtr( mapFile ); // This may be overkill
        mapFile->version = 1;
        mapFile->mapWidth = mapWidth;
        mapFile->mapHeight = mapHeight;
        mapFile->playerStartPos = playerStartPos;
        const i32 blockCount = blocks.GetCapcity();
        for( i32 blockIndex = 0; blockIndex < blockCount; blockIndex++ ) {
            MapBlock & block = blocks[ blockIndex ];
            if( block.filled == true ) {
                MapFileBlock b = {};
                b.xIndex = block.xIndex;
                b.yIndex = block.yIndex;
                mapFile->blocks.Add( b );
            }
        }

        return true;
    }

    void Map::AddFloor( glm::vec2 p1, glm::vec2 p2, i32 level, bool invertNormal ) {
        MapTriangle t1 = {};
        t1.p1 = glm::vec3( p1.x, BlockDim * level, p2.y );
        t1.p2 = glm::vec3( p1.x, BlockDim * level, p1.y );
        t1.p3 = glm::vec3( p2.x, BlockDim * level, p1.y );
        t1.uv1 = glm::vec2( 0, 1 );
        t1.uv2 = glm::vec2( 0, 0 );
        t1.uv3 = glm::vec2( 1, 0 );
        t1.ComputeNormal();

        MapTriangle t2 = {};
        t2.p1 = glm::vec3( p2.x, BlockDim * level, p2.y );
        t2.p2 = glm::vec3( p1.x, BlockDim * level, p2.y );
        t2.p3 = glm::vec3( p2.x, BlockDim * level, p1.y );
        t2.uv1 = glm::vec2( 1, 1 );
        t2.uv2 = glm::vec2( 0, 1 );
        t2.uv3 = glm::vec2( 1, 0 );
        t2.ComputeNormal();

        if( invertNormal ) {
            t1.InvertNormal();
            t2.InvertNormal();
        }

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

                AddFloor( b.bottomLeftWS, b.topRightWS, 0, true );
                AddFloor( b.bottomLeftWS, b.topRightWS, 1, false );
            }
        }
    }

    void Map::DEBUG_SaveToFile( Core * core, const char * path ) {
        MapFile * mapFile = core->MemoryAllocateTransient<MapFile>();
        SaveToMapFile( mapFile );

        TypeDescriptor * mapType = TypeResolver<MapFile>::get();
        nlohmann::json j = mapType->JSON_Write( mapFile );
        std::string json = j.dump( 4 );

        core->ResourceWriteEntireFile( path, json.c_str() );
    }

    void Map::DEBUG_LoadFromFile( Core * core, const char * path ) {
        MapFile * mapFile = core->MemoryAllocateTransient<MapFile>();
        TypeDescriptor * mapType = TypeResolver<MapFile>::get();

        char * mapText = (char *)core->MemoryAllocateTransient( Megabytes( 10 ) );
        core->ResourceReadEntireFile( path, mapText, Megabytes( 10 ) );

        nlohmann::json j = nlohmann::json::parse( mapText );

        mapType->JSON_Read( j, mapFile );

        LoadFromMapFile( mapFile );
    }
}
