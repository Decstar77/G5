#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {

    GameModeType GameMode_SinglePlayerGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameMode_SinglePlayerGame::IsInitialized() {
        return true;
    }

    void GameMode_SinglePlayerGame::Initialize( Core * core ) {
        simMap.localPlayerNumber.value = 1;
        simMap.localPlayerTeamNumber.value = 1;
        simMap.Initialize( core );
    }

    void GameMode_SinglePlayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags updateAndRenderFlags ) {
        simMap.Update( core, dt );
    }

    void GameMode_SinglePlayerGame::Shutdown( Core * core ) {
    }

    GameModeType GameMode_MultiplayerGame::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameMode_MultiplayerGame::IsInitialized() {
        return true;
    }

    void GameMode_MultiplayerGame::Initialize( Core * core ) {
        simMap.Initialize( core );

        if( startParms.localPlayerNumber == 1 ) {
            core->LogOutput( LogLevel::INFO, "I am the host" );
            isHost = true;
            simMap.localPlayerTeamNumber.value = 1;
        }
        else {
            core->LogOutput( LogLevel::INFO, "I am a peer" );
            isHost = false;
            simMap.localPlayerTeamNumber.value = 2;
        }

        simMap.localPlayerNumber.value = startParms.localPlayerNumber;
    }

    void GameMode_MultiplayerGame::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        simMap.Update( core, dt );
    }

    void GameMode_MultiplayerGame::Shutdown( Core * core ) {

    }

    //void Map::Start( Core * core, const GameStartParams & parms ) {
    //    isMp = parms.isMutliplayer;
    //    localPlayerNumber = parms.localPlayerNumber;
    //    otherPlayerNumber = parms.otherPlayerNumber;
    //    isStarting = true;

    //    MapFile * mapFile = core->MemoryAllocateTransient< MapFile >();
    //    bool tileMapLoaded = core->ResourceReadTextRefl( mapFile, "res/maps/space_ship/space_ship_map.json" );
    //    tileMap.spriteResource = core->ResourceGetAndCreateSprite( "res/maps/space_ship/space_ship_tiles.png", 0, 0, 0, 0 );

    //    if( tileMap.spriteResource == nullptr || tileMapLoaded == false ) {
    //        core->LogOutput( LogLevel::ERR, "Could not loaded tile set" );
    //        return;
    //    }

    //    tileMap.Initialize();
    //    tileMap.spriteResource->isTileMap = true;
    //    tileMap.spriteResource->tileWidth = (i32)TILE_SIZE;
    //    tileMap.spriteResource->tileHeight = (i32)TILE_SIZE;

    //    tileMap.tileXCount = mapFile->width;
    //    tileMap.tileYCount = mapFile->height;

    //    const i32 layerCount = mapFile->layers.GetCount();
    //    for( i32 layerIndex = 0; layerIndex < layerCount; layerIndex++ ) {
    //        MapFileLayer & layer = mapFile->layers[ layerIndex ];

    //        SmallString tileSetSource = mapFile->tilesets[ layerIndex ].source;
    //        tileSetSource.StripFileExtension();
    //        tileSetSource.Add( ".json" );

    //        MapFileTileSet * tileSetFile = core->MemoryAllocateTransient< MapFileTileSet >();
    //        if( core->ResourceReadTextRefl( tileSetFile, StringFormat::Large( "res/maps/space_ship/%s", tileSetSource.GetCStr() ).GetCStr() ) == false ) {
    //            continue;
    //        }

    //        if( layer.type == "tilelayer" ) {
    //            const i32 dataCount = layer.data.GetCount();
    //            for( i32 i = 0; i < dataCount; i++ ) {
    //                SmallString source = {};
    //                const i32 tileId = mapFile->ResolveId( layer.data[ i ], source );
    //                const i32 tileIndex = layer.data[ i ] - 1;
    //                if( tileIndex >= 0 ) {
    //                    i32 spriteTileX = tileIndex % tileSetFile->columns;
    //                    i32 spriteTileY = tileIndex / tileSetFile->columns;

    //                    i32 xIndex = i % tileMap.tileXCount;
    //                    i32 yIndex = i / tileMap.tileXCount;

    //                    i32 flags = 0;
    //                    const i32 prefabCount = tileSetFile->tiles.GetCount();
    //                    for( i32 prefabIndex = 0; prefabIndex < prefabCount; prefabIndex++ ) {
    //                        MapFileTile & prefab = tileSetFile->tiles[ prefabIndex ];
    //                        if( prefab.id == tileId ) {
    //                            const i32 propertyCount = prefab.properties.GetCount();
    //                            for( i32 propertyIndex = 0; propertyIndex < propertyCount; propertyIndex++ ) {
    //                                const SmallString & prop = prefab.properties[ propertyIndex ].name;
    //                                if( prop == "No Walk" ) {
    //                                    flags |= TILE_FLAG_NO_WALK;
    //                                }
    //                            }

    //                            break;
    //                        }
    //                    }

    //                    // Invert yIndex because tiles uses 0,0 at top left but we are 0,0 at bottom left
    //                    yIndex = tileMap.tileYCount - yIndex - 1;

    //                    tileMap.PlaceTile( xIndex, yIndex, spriteTileX, spriteTileY, flags );
    //                }
    //            }
    //        }

    //        if( layer.type == "objectgroup" ) {
    //            const i32 objectCount = layer.objects.GetCount();
    //            for( i32 objectIndex = 0; objectIndex < objectCount; objectIndex++ ) {
    //                MapFileTile & obj = layer.objects[ objectIndex ];
    //                SmallString source = {};
    //                u32 localId = mapFile->ResolveId( obj.gid, source );

    //                const i32 tileCount = tileSetFile->tiles.GetCount();
    //                for( i32 tileIndex = 0; tileIndex < tileCount; tileIndex++ ) {
    //                    MapFileTile & prefab = tileSetFile->tiles[ tileIndex ];
    //                    if( prefab.id == localId ) {
    //                        glm::vec2 pos = glm::vec2( obj.x, obj.y );
    //                        pos += glm::vec2( obj.width, -obj.height ) / 2.0f;
    //                        pos.y = mapFile->height * TILE_SIZE - pos.y;
    //                        glm::vec2 vel = glm::vec2( 0 );
    //                        EntityType type = ReflEnumFromString< EntityType >( prefab.type.GetCStr() );
    //                        SpawnEntity( core, type, pos, vel );
    //                    }
    //                }
    //            }
    //        }
    //    }

    //    if( isMp ) {
    //        if( localPlayerNumber == 1 ) {
    //            isAuthority = true;
    //            localPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), player1SpawnLocation );
    //            localPlayer->playerNumber = localPlayerNumber;

    //            otherPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), player2SpawnLocation );
    //            otherPlayer->playerNumber = parms.otherPlayerNumber;
    //            otherPlayer->netStreamed = true;
    //        }
    //        else {
    //            otherPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), player2SpawnLocation );
    //            otherPlayer->playerNumber = parms.otherPlayerNumber;
    //            otherPlayer->netStreamed = true;

    //            localPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), player1SpawnLocation );
    //            localPlayer->playerNumber = localPlayerNumber;
    //        }

    //        players.Add( localPlayer );
    //        players.Add( otherPlayer );
    //    }
    //    else {
    //        isAuthority = true;
    //        localPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), player1SpawnLocation );
    //        localPlayer->playerNumber = localPlayerNumber;
    //        players.Add( localPlayer );
    //    }



    //    isStarting = false;
    //}

    //void Map::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags updateAndRenderFlags ) {
    //    static TextureResource * sprUiPanel             = core->ResourceGetAndCreateTexture( "res/sprites/ui_ability_panel.png", false, false );
    //    static TextureResource * sprCharDroneSelection  = core->ResourceGetAndCreateTexture( "res/sprites/char_drone_selection.png", false, false );
    //    static TextureResource * sprParticleSingleWhite = core->ResourceGetAndCreateTexture( "res/sprites/particle_single_white_1x1.png", false, false );

    //    static SpriteResource * sprPlayerGun        = core->ResourceGetAndCreateSprite( "res/ents/player/gun.json", 1, 32, 32, 0 );
    //    static SpriteResource * sprPlayerBullet     = core->ResourceGetAndCreateSprite( "res/ents/player/bullet.json", 1, 8, 8, 0 );

    //    static SpriteResource * sprBotTurretTurnOn  = core->ResourceGetAndLoadSprite( "res/ents/char_bot_turret/char_bot_turret_turn.json" );
    //    static SpriteResource * sprBotTurretOn      = core->ResourceGetAndLoadSprite( "res/ents/char_bot_turret/char_bot_turret_on.json" );
    //    static SpriteResource * sprBotTurretBullet  = core->ResourceGetAndLoadSprite( "res/ents/char_bot_turret/char_bot_turret_bullet.json" );

    //    static TextureResource * sprUIHealth = core->ResourceGetAndCreateTexture( "res/sprites/ui/ui_health.png", false ,false );

    //    static SpriteResource * sprVFX_SmallExplody = core->ResourceGetAndLoadSprite( "res/sprites/vfx_small_explody/vfx_small_explody.json" );

    //    const f32 soundMinDist = 400;
    //    const f32 soundMaxDist = 10000;
    //    static AudioResource * sndCloseExplody1     = core->ResourceGetAndCreateAudio( "res/sounds/tomwinandysfx_explosions_volume_i_closeexplosion_01.wav", true, true, soundMinDist, soundMaxDist );
    //    static AudioResource * sndCloseExplody2     = core->ResourceGetAndCreateAudio( "res/sounds/TomWinandySFX_Explosions Volume I_CloseExplosion_06.wav", true, true, soundMinDist, soundMaxDist );
    //    static FontHandle fontHandle                = core->ResourceGetFont( "default" );


    //    if( isMp == true ) {
    //        if( core->NetworkIsConnected() == true ) {
    //            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //            while( core->NetworkRecieve( msg ) ) {
    //                switch( msg.type ) {
    //                    case NetworkMessageType::ENTITY_POS_UPDATE:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent != nullptr ) {
    //                            ent->pos = NetworkMessagePop<glm::vec2>( msg, offset );
    //                            ent->ori = NetworkMessagePop<f32>( msg, offset );
    //                        }
    //                        else {
    //                            core->LogOutput( LogLevel::WARN, "ENTITY_POS_UPDATE :: Could not find entity" );
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_ANIM_UPDATE:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent != nullptr ) {
    //                            i64 spriteId = NetworkMessagePop<i64>( msg, offset );
    //                            f32 framDuration = NetworkMessagePop<f32>( msg, offset );
    //                            bool isLooping = NetworkMessagePop<bool>( msg, offset );
    //                            SpriteResource * sprite = core->ResourceGetLoadedSprite( spriteId );
    //                            ent->spriteAnimator.frameDuration = framDuration;
    //                            ent->spriteAnimator.SetSpriteIfDifferent( core, sprite, isLooping );
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_AUDIO_PLAY:
    //                    {
    //                        i32 offset = 0;
    //                        SmallString id = NetworkMessagePop<SmallString>( msg, offset );
    //                        AudioResource * r = core->ResourceGetAndCreateAudio( id.GetCStr(), true, false, 30, 800 );
    //                        if( r != nullptr ) {
    //                            core->AudioPlay( r );
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_SPAWN:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent == nullptr ) {
    //                            EntityType type = EntityType::Make( (EntityType::_enumerated)NetworkMessagePop<i32>( msg, offset ) );
    //                            glm::vec2 pos = NetworkMessagePop<glm::vec2>( msg, offset );
    //                            glm::vec2 vel = NetworkMessagePop<glm::vec2>( msg, offset );
    //                            i32 netId = NetworkMessagePop<i32>( msg, offset );
    //                            if( isAuthority == true ) { // Peer requesting spawn
    //                                ent = SpawnEntitySim( core, type, pos, vel );
    //                                NetworkMessage & reply = *core->MemoryAllocateTransient< NetworkMessage >();
    //                                reply.type = NetworkMessageType::ENTITY_SPAWN;
    //                                NetworkMessagePush( reply, ent->handle );
    //                                NetworkMessagePush( reply, (i32)type );
    //                                NetworkMessagePush( reply, pos );
    //                                NetworkMessagePush( reply, vel );
    //                                NetworkMessagePush( reply, netId );
    //                                core->NetworkSend( reply );
    //                            }
    //                            else {
    //                                if( netId == 0 ) { // Host directed spawned
    //                                    SpawnEntitySim( core, type, pos, vel );
    //                                }
    //                                else { // Reply to peer spawn request.
    //                                    SpawnEntityResolve( core, netId );
    //                                }
    //                            }
    //                        }
    //                        else {
    //                            core->LogOutput( LogLevel::ERR, "ENTITY_SPAWN :: Already spawned" );
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_DESTROY:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent != nullptr ) {
    //                            i32 netId = NetworkMessagePop<i32>( msg, offset );
    //                            if( isAuthority == true ) { // Peer requesting destruction
    //                                NetworkMessage & reply = *core->MemoryAllocateTransient< NetworkMessage >();
    //                                reply.type = NetworkMessageType::ENTITY_DESTROY;
    //                                NetworkMessagePush( msg, ent->handle );
    //                                NetworkMessagePush( msg, netId );
    //                                core->NetworkSend( msg );

    //                                DestroyEntitySim( core, ent );
    //                            }
    //                            else {
    //                                if( netId == 0 ) { // Host directed destructions
    //                                    DestroyEntitySim( core, ent );
    //                                }
    //                                else { // Reply to peer destruction
    //                                    DestroyEntityResolve( core,  netId );
    //                                }
    //                            }
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_PLAYER_UPDATE:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent != nullptr ) {
    //                            glm::vec2 pos = NetworkMessagePop<glm::vec2>( msg, offset );
    //                            f32 gunOri = NetworkMessagePop<f32>( msg, offset );
    //                            f32 gunDir = NetworkMessagePop<f32>( msg, offset );
    //                            ent->pos = pos;
    //                            ent->playerStuff.weaponOri = gunOri;
    //                            ent->playerStuff.weaponDir = gunDir;
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_RPC_UNIT_TAKE_DAMAGE:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent != nullptr ) {
    //                            i32 damage = NetworkMessagePop<i32>( msg, offset );
    //                            Unit_TakeDamageSim( core, ent, damage );
    //                        }
    //                        else {
    //                            core->LogOutput( LogLevel::WARN, "ENTITY_RPC_UNIT_TAKE_DAMAGE :: Could not find entity %d %d", handle.idx, handle.gen );
    //                        }
    //                    } break;
    //                    case NetworkMessageType::ENTITY_RPC_UNIT_FIRE:
    //                    {
    //                        i32 offset = 0;
    //                        EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
    //                        Entity * ent = entityPool.Get( handle );
    //                        if( ent != nullptr ) {
    //                            f32 fireRate = NetworkMessagePop<f32>( msg, offset );
    //                            Unit_Fire( core, ent, fireRate );
    //                        }
    //                        else {
    //                            core->LogOutput( LogLevel::WARN, "ENTITY_RPC_UNIT_FIRE :: Could not find entity %d %d", handle.idx, handle.gen );
    //                        }
    //                    } break;
    //                }
    //            }
    //        }
    //    }
    //    

    //    EntList & entities = *core->MemoryAllocateTransient<EntList>();
    //    entityPool.GatherActiveObjs( entities );

    //    if( isMp == true ) {
    //        const f32 tickTime = 0.016f; // 60z
    //        static f32 dtAccumulator = 0.0f;
    //        dtAccumulator += dt;
    //        if( dtAccumulator >= tickTime ) {
    //            dtAccumulator = 0.0f;
    //            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //            msg.isUDP = true;
    //            msg.type = NetworkMessageType::ENTITY_PLAYER_UPDATE;
    //            NetworkMessagePush( msg, localPlayer->handle );
    //            NetworkMessagePush( msg, localPlayer->pos );
    //            NetworkMessagePush( msg, localPlayer->playerStuff.weaponOri );
    //            NetworkMessagePush( msg, localPlayer->playerStuff.weaponDir );
    //            core->NetworkSend( msg );

    //            if( isAuthority == true ) {
    //                const i32 entityCount = entities.GetCount();
    //                for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
    //                    Entity * ent = entities[ entityIndexA ];
    //                    if( ent->netStreamer == true ) {
    //                        ZeroStruct( msg );
    //                        msg.type = NetworkMessageType::ENTITY_POS_UPDATE;
    //                        NetworkMessagePush( msg, ent->handle );
    //                        NetworkMessagePush( msg, ent->pos );
    //                        NetworkMessagePush( msg, ent->ori );
    //                        core->NetworkSend( msg );
    //                    }
    //                }
    //            }
    //        }
    //    }

    //    DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0 );
    //    DrawContext * uiDrawContext = core->RenderGetDrawContext( 1 );
    //    DrawContext * debugDrawContext = core->RenderGetDrawContext( 2 );

    //    const i32 temporyEntitiesCount = temporySpawningEntities.GetCount();
    //    for( i32 i = 0; i < temporyEntitiesCount; i++ ) {
    //        entities.Add( temporySpawningEntities.Get( i ) );
    //    }

    //    //localCameraPos = ent->pos;
    //    const glm::vec2 camMin = localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f;
    //    const glm::vec2 camMax = localCameraPos + spriteDrawContext->GetCameraDims() / 2.0f;
    //    const glm::vec2 worldMin = glm::vec2( 0, 0 );
    //    const glm::vec2 worldMax = glm::vec2( tileMap.tileXCount * TILE_SIZE, tileMap.tileYCount * TILE_SIZE );

    //    //if( camMin.x < worldMin.x ) { localCameraPos.x = spriteDrawContext->GetCameraDims().x / 2.0f; }
    //    //if( camMin.y < worldMin.y ) { localCameraPos.y = spriteDrawContext->GetCameraDims().y / 2.0f; }
    //    //if( camMax.x > worldMax.x ) { localCameraPos.x = worldMax.x - spriteDrawContext->GetCameraDims().x / 2.0f; }
    //    //if( camMax.y > worldMax.y ) { localCameraPos.y = worldMax.y - spriteDrawContext->GetCameraDims().y / 2.0f; }

    //    spriteDrawContext->SetCameraPos( localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f );
    //    debugDrawContext->SetCameraPos( localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f );
    //    core->AudioSetListener( localCameraPos );

    //    const glm::vec2 mousePosPix = core->InputMousePosPixels();
    //    const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

    //    const BoxBounds2D cameraWsBounds = { camMin, camMax };

    //    const i32 tileCount = tileMap.tiles.GetCapcity();
    //    for( i32 i = 0; i < tileCount; i++ ) {
    //        TileMapTile & tile = tileMap.tiles[ i ];
    //        if( tile.active == true ){
    //            //if( tile.wsBounds.Intersects( cameraWsBounds ) == true ) {
    //                spriteDrawContext->DrawSprite( tileMap.spriteResource, tile.spriteTileIndexX, tile.spriteTileIndexY, tile.center );
    //            //}
    //        }
    //    }

    //    // @HACK:
    //    if( dt > 0.5f ) {
    //        dt = 0.016f;
    //    }

    //    const i32 entityCount = entities.GetCount();
    //    for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
    //        Entity * ent = entities[ entityIndexA ];

    //        if( ent->active == false ) {
    //            continue;
    //        }

    //        if( ent->netStreamed ) {
    //            ent->netVisualPos = glm::mix( ent->netVisualPos, ent->pos, 0.25f );
    //        }

    //        ent->spriteAnimator.Update( core, dt );

    //        const f32 entVel = glm::length( ent->vel );
    //        if( entVel > 50.0f ) {
    //            ent->facingDir = glm::sign( ent->vel.x );
    //        }

    //        if( ent->facingDir == 0.0 ) {
    //            ent->facingDir = 1.0f;
    //        }

    //        glm::vec2 drawPos = ent->netStreamed ? ent->netVisualPos : ent->pos;
    //        if( ent->spriteAnimator.sprite != nullptr ) {
    //            spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, ent->ori, glm::vec2( ent->facingDir, 1.0f ), ent->colorMultiplier );
    //        }

    //        Collider2D wsCollider = ent->GetWorldCollisionCollider();
    //        //debugDrawContext->DrawRect( wsCollider.box.min, wsCollider.box.max, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );

    //        bool collidedWithTiles = false;

    //        FixedList<TileMapTile *, 9> apron = {};
    //        i32 entTileXPos = (i32)( ent->pos.x / 32.0f );
    //        i32 entTileYPos = (i32)( ent->pos.y / 32.0f );
    //        tileMap.GetApron( entTileXPos, entTileYPos, apron );
    //        const i32 apronCount = apron.GetCount();
    //        for( i32 tileIndex = 0; tileIndex < apronCount; tileIndex++ ) {
    //            TileMapTile * tile = apron[ tileIndex ];
    //            if( EnumHasFlag( tile->flags, TILE_FLAG_NO_WALK ) == true ) {
    //                Manifold2D mani = {};
    //                if( wsCollider.Collision( tile->wsBounds, mani ) == true ) {
    //                    ent->pos -= mani.normal * mani.penetration;
    //                    wsCollider = ent->GetWorldCollisionCollider();
    //                    collidedWithTiles = true;
    //                    //ent->vel = glm::dot( ent->vel, -mani.normal ) * ent->vel; TODO: Adjust velo accordingly
    //                }
    //            }

    //            //debugDrawContext->DrawRect( tile->center, glm::vec2( 30, 30 ), 0.0f, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );
    //            //debugDrawContext->DrawRect( tile->wsBounds.min, tile->wsBounds.max, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );
    //        }

    //        ent->acc = glm::vec2( 0.0 );
    //        if( EnumHasFlag( updateAndRenderFlags, UPDATE_AND_RENDER_FLAG_NO_UPDATE ) == false ) {
    //            switch( ent->type ) {
    //                case EntityType::PLAYER:
    //                {
    //                    PlayerStuff & player = ent->playerStuff;

    //                    if( ent->playerNumber == localPlayerNumber ) {
    //                        const f32 playerSpeed = 2500.0f / 2.0f;

    //                        bool getInput = true;

    //                        if( getInput == true ) {
    //                            if( core->InputKeyDown( KeyCode::KEY_CODE_W ) ) {
    //                                ent->acc.y += 1;
    //                            }
    //                            if( core->InputKeyDown( KeyCode::KEY_CODE_S ) ) {
    //                                ent->acc.y -= 1;
    //                            }
    //                            if( core->InputKeyDown( KeyCode::KEY_CODE_A ) ) {
    //                                ent->acc.x -= 1;
    //                            }
    //                            if( core->InputKeyDown( KeyCode::KEY_CODE_D ) ) {
    //                                ent->acc.x += 1;
    //                            }

    //                            if( ent->acc != glm::vec2( 0, 0 ) ) {
    //                                ent->acc = glm::normalize( ent->acc ) * playerSpeed;
    //                            }
    //                        }

    //                        ent->acc.x -= ent->vel.x * ent->resistance;
    //                        ent->acc.y -= ent->vel.y * ent->resistance;
    //                        ent->vel += ent->acc * dt;
    //                        ent->pos += ent->vel * dt;

    //                        //debugDrawContext->DrawRect( ent->pos, glm::vec2( 5, 5 ), 0.0f, glm::vec4( 0.2f, 0.8f, 0.2f, 0.5f ) );

    //                        localCameraPos = glm::mix( localCameraPos, ent->pos, dt * 4.0f );

    //                        // Spring forumla 
    //                        // f = -kx - bv
    //                        // k = spring constant
    //                        // x = displacement
    //                        // b = damping constant
    //                        // v = velocity
    //                        player.weaponPos = glm::mix( player.weaponPos, ent->pos, 0.25f );
    //                        player.weaponTimer -= dt;
    //                        if( player.weaponTimer < 0.0f ) {
    //                            player.weaponTimer = 0.0f;
    //                        }

    //                        glm::vec2 dir = glm::normalize( ent->pos - mousePosWorld );
    //                        if( player.weaponTimer == 0.0f && core->InputMouseButtonDown( MOUSE_BUTTON_1 ) == true ) {
    //                            player.weaponTimer += 0.1f;
    //                            player.weaponPos = player.weaponPos + dir * 2.0f;

    //                            Entity * b = SpawnEntity( core, EntityType::Make( EntityType::BULLET ), player.weaponPos - dir * 10.0f, -dir * 250.0f );
    //                        }


    //                        constexpr f32 pi = glm::pi<f32>();
    //                        constexpr f32 halfPi = pi / 2.0f;

    //                        f32 angle = glm::atan( dir.x, dir.y ) + halfPi;
    //                        if( angle >= -halfPi && angle <= halfPi ) {
    //                            player.weaponOri = angle;
    //                            player.weaponDir = 1.0f;
    //                        }
    //                        else {
    //                            player.weaponOri = angle - pi;
    //                            player.weaponDir = -1.0f;

    //                        }
    //                    }
    //                    else {
    //                        if( isMp ) {
    //                            player.weaponPos = ent->netVisualPos;
    //                        }
    //                    }

    //                    spriteDrawContext->DrawSprite( sprPlayerGun, 0, player.weaponPos, player.weaponOri, glm::vec2( player.weaponDir, 1.0f ) );
    //                } break;
    //                case EntityType::ENEMY_BOT_DRONE:
    //                {
    //                    UnitStuff & unit = ent->unitStuff;
    //                    Navigator & nav = ent->navigator;

    //                    const f32 alertRad = TILE_SIZE * 5.0f;
    //                    const f32 swarmRad = 55.0f;

    //                    switch( unit.state ) {
    //                        case UNIT_STATE_IDLE:
    //                        {
    //                            if( isAuthority == false ) {
    //                                break;
    //                            }

    //                            f32 distToPlayer = 0.0f;
    //                            glm::vec2 dir = glm::vec2( 0 );
    //                            Entity * closePlayer = ClosestPlayerTo( ent->pos, distToPlayer, dir );

    //                            //if( distToPlayer < alertRad ) {
    //                            if( LineOfSite( ent, closePlayer, alertRad ) ) {
    //                                const f32 r = swarmRad / 2.0f;
    //                                nav.dest = closePlayer->pos + glm::vec2( Random::Float( -r, r ), Random::Float( -r, r ) );
    //                                unit.state = UNIT_STATE_SWARM;
    //                                break;
    //                            }
    //                        } break;
    //                        case UNIT_STATE_TAKING_DAMAGE:
    //                        {
    //                            unit.takingDamageTimer -= dt;
    //                            if( unit.takingDamageTimer <= 0.0f ) {
    //                                unit.takingDamageTimer = 0.0f;
    //                                unit.state = UNIT_STATE_SWARM;
    //                                ent->colorMultiplier = glm::vec4( 1 );
    //                            }
    //                            else {
    //                                ent->colorMultiplier = glm::vec4( 100, 100, 100, 1 );
    //                            }
    //                        } break;
    //                        case UNIT_STATE_EXPLODING:
    //                        {
    //                            ent->spriteAnimator.SetSpriteIfDifferent( core, sprVFX_SmallExplody, false );
    //                            if( unit.playedDeathSound == false && ent->spriteAnimator.frameIndex == 1 ) {
    //                                unit.playedDeathSound = true;
    //                                core->AudioPlayRandom( nullptr, sndCloseExplody1, sndCloseExplody2 );
    //                            }

    //                            if( ent->spriteAnimator.loopCount > 0 ) {
    //                                DestroyEntity( core, ent );
    //                            }
    //                        } break;
    //                        case UNIT_STATE_SWARM:
    //                        {
    //                            if( isAuthority == false ) {
    //                                break;
    //                            }

    //                            // @SPEED
    //                            f32 dist = glm::distance( nav.dest, ent->pos );
    //                            if( dist < 5.0f ) {
    //                                f32 distToPlayer = 0.0f;
    //                                glm::vec2 dir = glm::vec2( 0 );
    //                                Entity * closePlayer = ClosestPlayerTo( ent->pos, distToPlayer, dir );
    //                                nav.dest = closePlayer->pos + glm::vec2( Random::Float( -swarmRad, swarmRad ), Random::Float( -swarmRad, swarmRad ) );
    //                            }
    //                            else {
    //                                const f32 speed = 1500.0f;
    //                                const f32 resistance = 5.0f;
    //                                ent->acc = glm::normalize( nav.dest - ent->pos ) * speed;
    //                            }

    //                        } break;
    //                    }

    //                    if( ent->particleSystem.emitting == true ) {
    //                        ParticleSystem & part = ent->particleSystem;
    //                        part.spawnTimer += dt;

    //                        for( i32 partIndex = 0; partIndex < part.count; partIndex++ ) {
    //                            Particle & p = part.particles[ partIndex ];
    //                            p.lifeTime -= dt;
    //                            if( p.lifeTime < 0 ) {
    //                                p.lifeTime = 0.0f;
    //                            }

    //                            f32 l = p.lifeTime / part.lifeTime;
    //                            p.scale = glm::mix( part.scaleMin, part.scaleMax, l );
    //                            p.pos += p.vel * dt;
    //                        }

    //                        bool allDead = true;
    //                        for( i32 partIndex = 0; partIndex < part.count; partIndex++ ) {
    //                            Particle & p = part.particles[ partIndex ];
    //                            if( p.lifeTime != 0.0f ) {
    //                                allDead = false;
    //                                spriteDrawContext->DrawTexture( part.texture, p.pos, 0.0f, glm::vec2( p.scale ) );
    //                            }
    //                        }
    //                    }

    //                    ent->acc.x -= ent->vel.x * ent->resistance;
    //                    ent->acc.y -= ent->vel.y * ent->resistance;
    //                    ent->vel += ent->acc * dt;
    //                    ent->pos += ent->vel * dt;

    //                    // Debug
    //                    //debugDrawContext->DrawRect( ent->pos, glm::vec2( alertRad ), 0.0f, glm::vec4( 0.2f, 0.2f, 0.75f, 0.76f ) );

    //                } break;
    //                case EntityType::ENEMY_BOT_TURRET:
    //                {
    //                    const f32 alertRad = TILE_SIZE * 6.0f;
    //                    const f32 attackRad = TILE_SIZE * 5.0f;
    //                    const f32 fireRate = 0.7f;
    //                    const glm::vec2 bulletPos = ent->pos + glm::vec2( 0.0f, TILE_SIZE * 0.25f );
    //                    UnitStuff & unit = ent->unitStuff;
    //                    
    //                    unit.fireRateTimer -= dt;
    //                    if( unit.fireRateTimer < 0.0f ) {
    //                        unit.fireRateTimer = 0.0f;
    //                    }

    //                    if( unit.fireRateTimer < 0.4f ) {
    //                        glm::vec2 pos = glm::mix( bulletPos, ent->pos, unit.fireRateTimer );
    //                        spriteDrawContext->DrawSprite( sprBotTurretBullet, 0, pos );
    //                    }

    //                    switch( unit.state ) {
    //                        case UNIT_STATE_IDLE:
    //                        {
    //                            f32  dist = 0.0f;
    //                            glm::vec2 dir = glm::vec2( 0 );
    //                            Entity * closePlayer = ClosestPlayerTo( ent->pos, dist, dir );
    //                            //if( dist < alertRad ) {
    //                            if( LineOfSite( ent, closePlayer, alertRad ) ) {
    //                                unit.state = UNIT_STATE_ALERT;
    //                                ent->spriteAnimator.SetSpriteIfDifferent( core, sprBotTurretTurnOn, false );
    //                            }
    //                        } break;
    //                        case UNIT_STATE_ALERT:
    //                        {
    //                            if( ent->spriteAnimator.loopCount == 1 ) {
    //                                ent->spriteAnimator.SetSpriteIfDifferent( core, sprBotTurretOn, false );
    //                                unit.state = UNIT_STATE_ATTACKING;
    //                            }
    //                        } break;
    //                        case UNIT_STATE_ATTACKING:
    //                        {
    //                            if( isAuthority == true ) {
    //                                f32 dist = 0.0f;
    //                                glm::vec2 dir = glm::vec2( 0 );
    //                                Entity * closePlayer = ClosestPlayerTo( bulletPos, dist, dir );
    //                                if( dist < attackRad ) {
    //                                    if( unit.fireRateTimer == 0.0f ) {
    //                                        Unit_Fire( core, ent, fireRate + Random::Float( -0.1f, 0.1f ) );
    //                                        SpawnEntity( core, EntityType::Make( EntityType::ENEMY_BULLET ), bulletPos, dir * 100.0f );
    //                                    }
    //                                }
    //                            }
    //                        } break;
    //                        case UNIT_STATE_TAKING_DAMAGE:
    //                        {
    //                            unit.takingDamageTimer -= dt;
    //                            if( unit.takingDamageTimer <= 0.0f ) {
    //                                unit.takingDamageTimer = 0.0f;
    //                                unit.state = UNIT_STATE_ATTACKING;
    //                                ent->colorMultiplier = glm::vec4( 1 );
    //                            }
    //                            else {
    //                                ent->colorMultiplier = glm::vec4( 10, 10, 10, 1 );
    //                            }
    //                        } break;
    //                        case UNIT_STATE_EXPLODING:
    //                        {
    //                            DestroyEntity( core, ent );
    //                        } break;
    //                    }
    //                        
    //                    
    //                } break;
    //                case EntityType::BULLET:
    //                {
    //                    if( collidedWithTiles == true ) {
    //                        DestroyEntity( core, ent );
    //                    }

    //                    ent->acc.x -= ent->vel.x * ent->resistance;
    //                    ent->acc.y -= ent->vel.y * ent->resistance;
    //                    ent->vel += ent->acc * dt;
    //                    ent->pos += ent->vel * dt;

    //                    for( i32 otherEntityIndex = 0; otherEntityIndex < entityCount; otherEntityIndex++ ) {
    //                        Entity * enemy = entities[ otherEntityIndex ];
    //                        if( EntityTypeIsEnemy( enemy->type ) ) {
    //                            if( enemy->active == false ) {
    //                                continue;
    //                            }

    //                            Collider2D bulletCollider = ent->GetWorldCollisionCollider();
    //                            Collider2D enemyCollider = enemy->GetWorldCollisionCollider();
    //                            if( bulletCollider.Intersects( enemyCollider ) ) {
    //                                Unit_TakeDamage( core, enemy, 50 );
    //                                DestroyEntity( core, ent );
    //                            }
    //                        }
    //                    }

    //                } break;
    //                case EntityType::ENEMY_BULLET:
    //                {
    //                    if( collidedWithTiles == true ) {
    //                        DestroyEntity( core, ent );
    //                    }

    //                    ent->acc.x -= ent->vel.x * ent->resistance;
    //                    ent->acc.y -= ent->vel.y * ent->resistance;
    //                    ent->vel += ent->acc * dt;
    //                    ent->pos += ent->vel * dt;

    //                    f32 rotDir = ent->handle.idx % 2 == 0 ? -1.0f : 1.0f;
    //                    ent->ori += rotDir * dt * 2.0f;

    //                    const i32 playerCount = players.GetCount();
    //                    for( i32 playerIndex = 0; playerIndex < playerCount; playerIndex++ ) {
    //                        Entity * playerEnt = players[ playerIndex ];
    //                        Collider2D playerCollider = playerEnt->GetWorldCollisionCollider();
    //                        if( playerCollider.Intersects( wsCollider ) == true ) {
    //                            playerEnt->currentHealth -= 2;
    //                            DestroyEntity( core, ent );
    //                        }
    //                    }
    //                } break;
    //            }
    //        }

    //    }

    //    if( false ) {
    //        for( int i = 0; i < entityCount; i++ ) {
    //            const Entity * ent = entities[ i ];
    //            const glm::vec4 selectionColor( 0.8f, 0.8f, 0.5f, 0.4f );
    //            switch( ent->selectionCollider.type ) {
    //                case COLLIDER_TYPE_CIRCLE:
    //                {
    //                    Collider2D c = ent->GetWorldSelectionCollider();
    //                    glm::vec2 r = glm::vec2( c.circle.rad );
    //                    debugDrawContext->DrawRect( c.circle.pos - r, c.circle.pos + r, selectionColor );

    //                    // @TODO: Fix
    //                    //spriteDrawContext->DrawCircle( glm::vec2( 200, 200 ), 100 );
    //                    //spriteDrawContext->DrawCircle( c.circle.pos, c.circle.rad );
    //                } break;
    //                case COLLIDER_TYPE_BOX:
    //                {
    //                    Collider2D b = ent->GetWorldSelectionCollider();
    //                    debugDrawContext->DrawRect( b.box.min, b.box.max, selectionColor );
    //                }
    //            }
    //        }
    //    }
    //    if( false ) {
    //        for( int i = 0; i < entityCount; i++ ) {
    //            const Entity * ent = entities[ i ];
    //            const glm::vec4 selectionColor( 0.8f, 0.8f, 0.5f, 0.4f );
    //            switch( ent->collisionCollider.type ) {
    //                case COLLIDER_TYPE_CIRCLE:
    //                {
    //                    Collider2D c = ent->GetWorldCollisionCollider();
    //                    glm::vec2 r = glm::vec2( c.circle.rad );
    //                    debugDrawContext->DrawRect( c.circle.pos - r, c.circle.pos + r, selectionColor );

    //                    // @TODO: Fix
    //                    //spriteDrawContext->DrawCircle( glm::vec2( 200, 200 ), 100 );
    //                    //spriteDrawContext->DrawCircle( c.circle.pos, c.circle.rad );
    //                } break;
    //                case COLLIDER_TYPE_BOX:
    //                {
    //                    Collider2D b = ent->GetWorldCollisionCollider();
    //                    debugDrawContext->DrawRect( b.box.min, b.box.max, selectionColor );
    //                }
    //            }
    //        }
    //    }

    //    uiDrawContext->DrawTextureTL( sprUIHealth, glm::vec2( 0, uiDrawContext->GetCameraHeight()  ) );
    //    const f32 startX = 8;
    //    const f32 endX = 52;
    //    const f32 percent = (f32)localPlayer->currentHealth / (f32)localPlayer->maxHealth;
    //    const f32 width = ( endX - startX ) * percent;
    //    uiDrawContext->DrawRect( glm::vec2( 8, uiDrawContext->GetCameraHeight() - 18 ), glm::vec2( startX + width, uiDrawContext->GetCameraHeight() - 6 ), Colors::NEON_RED );

    //    SmallString s = StringFormat::Small( "ping=%d", (i32)core->NetworkGetPing() );
    //    spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );
    //    s = StringFormat::Small( "dt=%f", dt );
    //    spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 160 ), 32, s.GetCStr() );
    //    s = StringFormat::Small( "fps=%f", 1.0f / dt );
    //    spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 200 ), 32, s.GetCStr() );

    //    if( EnumHasFlag( updateAndRenderFlags, UPDATE_AND_RENDER_FLAG_DONT_SUBMIT_RENDER ) == false ) {
    //        core->RenderSubmit( spriteDrawContext, true );
    //        core->RenderSubmit( uiDrawContext, false );
    //        core->RenderSubmit( debugDrawContext, false );
    //    }
    //}

    //void Map::SetupEntity( Core * core, Entity * entity, EntityType type, glm::vec2 pos, glm::vec2 vel ) {
    //    entity->active = true;
    //    entity->type = type;
    //    entity->facingDir = 1.0f;
    //    entity->resistance = 14.0f;
    //    entity->pos = pos;
    //    entity->vel = vel;
    //    entity->netVisualPos = pos;
    //    entity->map = this;
    //    entity->netStreamed = !isAuthority;
    //    entity->colorMultiplier = glm::vec4( 1.0f );
    //    
    //    if( entity->name.GetLength() == 0 ) {
    //        entity->name = "Mr No Name";
    //    }

    //    switch( type ) {
    //        case EntityType::PLAYER:
    //        {
    //            static SpriteResource * playerSprite = core->ResourceGetAndCreateSprite( "res/ents/player/forward.json", 1, 32, 32, 0 );
    //            entity->collisionCollider.type = COLLIDER_TYPE_BOX;
    //            entity->collisionCollider.box.CreateFromCenterSize( glm::vec2( 0, 0 ), glm::vec2( 16, 20 ) );
    //            entity->spriteAnimator.sprite = playerSprite;
    //            entity->maxHealth = 100;
    //            entity->currentHealth = entity->maxHealth;
    //        } break;
    //        case EntityType::ENEMY_BOT_DRONE:
    //        {
    //            entity->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
    //            entity->selectionCollider.circle.pos = glm::vec2( -0.5f, -0.5f );
    //            entity->selectionCollider.circle.rad = 5.0f;
    //            entity->collisionCollider = entity->selectionCollider;
    //            entity->maxHealth = 100;
    //            entity->currentHealth = entity->maxHealth;
    //            entity->netStreamer = true;
    //            static SpriteResource * spriteResource = core->ResourceGetAndCreateSprite( "res/ents/char_bot_drone/bot_drone_jap.json", 7, 16, 16, 10 );
    //            //static SpriteResource * spriteResource = core->ResourceGetAndCreateSprite( "res/ents/char_bot_drone/char_drone_01.json", 1, 32, 32, 1 );
    //            entity->spriteAnimator.SetSpriteIfDifferent( core, spriteResource, true );
    //        } break;
    //        case EntityType::BULLET:
    //        {
    //            static SpriteResource * spriteResource = core->ResourceGetAndCreateSprite( "res/ents/player/bullet.json", 1, 8, 8, 0 );
    //            entity->resistance = 0.0f;
    //            entity->spriteAnimator.sprite = spriteResource;
    //            entity->collisionCollider.type = COLLIDER_TYPE_BOX;
    //            entity->collisionCollider.box.min = glm::vec2( -5.0f );
    //            entity->collisionCollider.box.max = glm::vec2( 5.0f );
    //        } break;
    //        case EntityType::ENEMY_BULLET:
    //        {
    //            static SpriteResource * sprBotTurretBullet = core->ResourceGetAndLoadSprite( "res/ents/char_bot_turret/char_bot_turret_bullet.json" );
    //            entity->resistance = 0.0f;
    //            entity->spriteAnimator.sprite = sprBotTurretBullet;
    //            entity->collisionCollider.type = COLLIDER_TYPE_BOX;
    //            entity->collisionCollider.box.min = glm::vec2( -5.0f );
    //            entity->collisionCollider.box.max = glm::vec2( 5.0f );
    //        } break;
    //        case EntityType::ENEMY_BOT_TURRET:
    //        {
    //            entity->resistance = 0.0f;
    //            entity->spriteAnimator.sprite = core->ResourceGetAndLoadSprite( "res/ents/char_bot_turret/char_bot_turret_off.json" );
    //            entity->collisionCollider.type = COLLIDER_TYPE_BOX;
    //            entity->maxHealth = 250;
    //            entity->currentHealth = entity->maxHealth;

    //            // @TODO: We really should just find the bounding box of the pixels...
    //            entity->selectionCollider.type = COLLIDER_TYPE_BOX;
    //            entity->selectionCollider.box.min = -1.0f * glm::vec2( entity->spriteAnimator.sprite->frameWidth, entity->spriteAnimator.sprite->frameHeight ) * 0.5f * 0.5f;
    //            entity->selectionCollider.box.max =  1.0f * glm::vec2( entity->spriteAnimator.sprite->frameWidth, entity->spriteAnimator.sprite->frameHeight ) * 0.5f * 0.5f;
    //        } break;
    //        case EntityType::OBJ_TERMINAL:
    //        {
    //            entity->resistance = 0.0f;
    //            entity->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "res/ents/obj_terminal/obj_terminal.json", 1, 32, 32, 0 );
    //            entity->collisionCollider.type = COLLIDER_TYPE_BOX;
    //            entity->collisionCollider.box.min = -1.0f * glm::vec2( entity->spriteAnimator.sprite->frameWidth, entity->spriteAnimator.sprite->frameHeight ) * 0.5f;
    //            entity->collisionCollider.box.max = 1.0f * glm::vec2( entity->spriteAnimator.sprite->frameWidth, entity->spriteAnimator.sprite->frameHeight ) * 0.5f;
    //        } break;
    //    }
    //}

    //Entity * Map::SpawnEntitySim( Core * core, EntityType type, glm::vec2 pos, glm::vec2 vel ) {
    //    EntityHandle handle = {};
    //    Entity * entity = entityPool.Add( handle );
    //    AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
    //    if( entity != nullptr ) {
    //        ZeroStructPtr( entity );
    //        entity->handle = handle;
    //        SetupEntity( core, entity, type, pos, vel );
    //    }

    //    return entity;
    //}

    //Entity * Map::SpawnEntityResolve( Core * core, i32 netId ) {
    //    const i32 tempEntCount = temporySpawningEntities.GetCount();
    //    for( i32 i = 0; i < tempEntCount; i++ ) {
    //        Entity * ent = &temporySpawningEntities[ i ];
    //        if( ent->netTempId == netId ) {
    //            EntityHandle handle = {};
    //            Entity * entity = entityPool.Add( handle );
    //            AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
    //            if( entity != nullptr ) {
    //                memcpy( entity, ent, sizeof( Entity ) );
    //                entity->handle = handle;
    //                entity->netTempId = 0;
    //                temporySpawningEntities.RemoveIndex( i );
    //                return entity;
    //            }
    //            return nullptr;
    //        }
    //    }
    //    return nullptr;
    //}
    //
    //Entity * Map::SpawnEntity( Core * core, EntityType type, glm::vec2 pos, glm::vec2 vel ) {
    //    if( isStarting == true ) {
    //        return SpawnEntitySim( core, type, pos, vel );
    //    }

    //    Entity * entity = nullptr;
    //    if( isAuthority == true ) {
    //         entity = SpawnEntitySim( core, type, pos, vel );
    //        if( isMp == true ) {
    //            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //            msg.type = NetworkMessageType::ENTITY_SPAWN;
    //            NetworkMessagePush( msg, entity->handle );
    //            NetworkMessagePush( msg, (i32)entity->type );
    //            NetworkMessagePush( msg, pos );
    //            NetworkMessagePush( msg, vel );
    //            NetworkMessagePush( msg, 0 );
    //            core->NetworkSend( msg );
    //        }
    //    }
    //    else {
    //        Assert( isMp == true );

    //        entity = &temporySpawningEntities.AddEmpty();
    //        entity->netTempId = netTempId++;
    //        SetupEntity( core, entity, type, pos, vel );

    //        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //        msg.type = NetworkMessageType::ENTITY_SPAWN;
    //        NetworkMessagePush( msg, entity->handle );
    //        NetworkMessagePush( msg, (i32)entity->type );
    //        NetworkMessagePush( msg, pos );
    //        NetworkMessagePush( msg, vel );
    //        NetworkMessagePush( msg, entity->netTempId );
    //        core->NetworkSend( msg );
    //    }

    //    return entity;
    //}

    //void Map::DestroyEntitySim( Core * core, Entity * entity ) {
    //    if( entity != nullptr ) {
    //        entityPool.Remove( entity->handle );
    //    }
    //}

    //void Map::DestroyEntityResolve( Core * core, i32 netId ) {
    //    const i32 tempEntCount = temporyDestroyingEntities.GetCount();
    //    for( i32 i = 0; i < tempEntCount; i++ ) {
    //        if( temporyDestroyingEntities[ i ].netTempId == netId ) {
    //            Entity * entity = entityPool.Get( temporyDestroyingEntities[ i ].handle );
    //            DestroyEntitySim( core, entity );
    //            temporyDestroyingEntities.RemoveIndex( i );
    //            return;
    //        };
    //    }
    //}

    //void Map::DestroyEntity( Core * core, Entity * entity ) {
    //    entity->active = false; // Trick to hide the entity in laggy mp enviroments. We need to wait for the destroy entity rpc...
    //    if( isAuthority == true ) {
    //        if( isMp == true ) {
    //            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //            msg.type = NetworkMessageType::ENTITY_DESTROY;
    //            NetworkMessagePush( msg, entity->handle );
    //            NetworkMessagePush( msg, 0 );
    //            core->NetworkSend( msg );
    //        }

    //        DestroyEntitySim( core, entity );
    //    }
    //    else {
    //        Assert( isMp == true );
    //        entity->active = false;
    //        entity = temporyDestroyingEntities.Add_MemCpyPtr( entity );
    //        entity->netTempId = netTempId++;

    //        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //        msg.type = NetworkMessageType::ENTITY_DESTROY;
    //        NetworkMessagePush( msg, entity->handle );
    //        NetworkMessagePush( msg, entity->netTempId );
    //        core->NetworkSend( msg );
    //    }
    //}

    //void Map::Unit_Fire( Core * core, Entity * ent, f32 fireRate ) {
    //    UnitStuff &unit = ent->unitStuff;
    //    unit.fireRateTimer = fireRate;
    //    if( isMp == true && isAuthority == true ) {
    //        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //        msg.type = NetworkMessageType::ENTITY_RPC_UNIT_FIRE;
    //        NetworkMessagePush( msg, ent->handle );
    //        NetworkMessagePush( msg, fireRate );
    //        core->NetworkSend( msg );
    //    }
    //}

    //void Map::Unit_TakeDamageSim( Core * core, Entity * ent, i32 damage ) {
    //    ent->currentHealth -= damage;

    //    if( ent->currentHealth < 0 ) {
    //        ent->currentHealth = 0;
    //        ent->unitStuff.state = UNIT_STATE_EXPLODING;
    //        ent->spriteAnimator.frameDelaySkip = Random::Int( 3 );
    //        return;
    //    }

    //    ent->unitStuff.state = UNIT_STATE_TAKING_DAMAGE;
    //    ent->unitStuff.takingDamageTimer = 0.1f;

    //    ZeroStruct( ent->particleSystem );
    //    ent->particleSystem.count = 10;
    //    ent->particleSystem.texture = core->ResourceGetAndCreateTexture( "res/sprites/particle_single_white_1x1.png", false, false );;
    //    ent->particleSystem.lifeTime = 0.5f;
    //    ent->particleSystem.scaleMin = 1;
    //    ent->particleSystem.scaleMax = 2;
    //    ent->particleSystem.velMin = glm::vec2( -100.0f );
    //    ent->particleSystem.velMax = glm::vec2( 100.0f );
    //    ent->particleSystem.oneShot = true;

    //    ParticleSystem & part = ent->particleSystem;
    //    ent->particleSystem.emitting = true;
    //    for( i32 partIndex = 0; partIndex < ent->particleSystem.count; partIndex++ ) {
    //        Particle & p = part.particles[ partIndex ];
    //        p.pos = ent->pos;
    //        p.lifeTime = part.lifeTime;
    //        p.scale = Random::Float( part.scaleMin, part.scaleMax );
    //        p.vel = Random::Vec2( part.velMin, part.velMax );
    //    }
    //}

    //void Map::Unit_TakeDamage( Core * core, Entity * ent, i32 damage ) {
    //    if( isMp == true ) {
    //        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
    //        msg.type = NetworkMessageType::ENTITY_RPC_UNIT_TAKE_DAMAGE;
    //        NetworkMessagePush( msg, ent->handle );
    //        NetworkMessagePush( msg, damage );
    //        core->NetworkSend( msg );
    //    }

    //    Unit_TakeDamageSim( core, ent, damage );
    //}

    //bool Map::LineOfSite( Entity * a, Entity * b, f32 maxDist ) {
    //    if( a == nullptr || b == nullptr ) {
    //        return false;
    //    }

    //    if( glm::distance2( a->pos, b->pos ) > maxDist * maxDist ) {
    //        return false;
    //    }

    //    TileMapTile * startTile = tileMap.GetTile( a->pos );
    //    TileMapTile * endTile = tileMap.GetTile( b->pos );

    //    // Perform DDA algorithm
    //    i32 dx = endTile->xIndex - startTile->xIndex;
    //    i32 dy = endTile->yIndex - startTile->yIndex;
    //    i32 steps = abs( dx ) > abs( dy ) ? abs( dx ) : abs( dy );
    //    f32 Xinc = dx / (f32)steps;
    //    f32 Yinc = dy / (f32)steps;
    //    f32 x = (f32)startTile->xIndex;
    //    f32 y = (f32)startTile->yIndex;
    //    for( i32 i = 0; i <= steps; i++ ) {
    //        x += Xinc;
    //        y += Yinc;
    //        TileMapTile * tile = tileMap.GetTile( (i32)roundf(x), (i32)roundf(y) );
    //        if( tile == nullptr ) {
    //            return false;
    //        }
    //        if( EnumHasFlag( tile->flags, TILE_FLAG_NO_WALK ) == true ) {
    //            return false;
    //        }
    //        if( tile == endTile ) {
    //            return true;
    //        }
    //    }

    //    return false;
    //}

    //Entity * Map::ClosestPlayerTo( glm::vec2 p, f32 & dist, glm::vec2 & dir ) {
    //    dist = FLT_MAX;
    //    Entity * closePlayer = nullptr;
    //    const int playerCount = players.GetCount();
    //    for( i32 playerIndex = 0; playerIndex < playerCount; playerIndex++ ) {
    //        Entity * ent = players[ playerIndex ];
    //        if( ent != nullptr ) {
    //            glm::vec2 d = ent->pos - p;
    //            f32 l = glm::length( d );
    //            if( l < dist ) {
    //                dist = l;
    //                closePlayer = ent;
    //                dir = d / l;
    //            }
    //        }
    //    }

    //    return closePlayer;
    //}

    //inline static Collider2D ColliderForSpace( const Collider2D & base, glm::vec2 p ) {
    //    Collider2D c = base;
    //    switch( base.type ) {
    //        case COLLIDER_TYPE_CIRCLE:
    //        {
    //            c.circle.pos += p;
    //        } break;
    //        case COLLIDER_TYPE_BOX:
    //        {
    //            c.box.Translate( p );
    //        } break;
    //        default:
    //        {
    //            INVALID_CODE_PATH;
    //        } break;
    //    }

    //    return c;
    //}

    //Collider2D Entity::GetWorldCollisionCollider() const {
    //    return ColliderForSpace( collisionCollider, pos );
    //}

    //Collider2D Entity::GetWorldSelectionCollider() const {
    //    return ColliderForSpace( selectionCollider, pos );
    //}

    //void SpriteAnimator::SetFrameRate( f32 fps ) {
    //    frameDuration = 1.0f / fps;
    //}

    //bool SpriteAnimator::SetSpriteIfDifferent( Core * core, SpriteResource * sprite, bool loops ) {
    //    if( this->sprite != sprite ) {
    //        this->sprite = sprite;
    //        SetFrameRate( (f32)sprite->frameRate );
    //        frameIndex = 0;
    //        frameTimer = 0;
    //        loopCount = 0;
    //        this->loops = loops;
    //        TestFrameActuations( core );
    //        return true;
    //    }
    //    return false;
    //}

    //void SpriteAnimator::Update( Core * core, f32 dt ) {
    //    if( sprite != nullptr && sprite->frameCount > 1 ) {
    //        frameTimer += dt;
    //        if( frameTimer >= frameDuration ) {
    //            frameTimer -= frameDuration;
    //            if( frameDelaySkip == 0 ) {
    //                frameIndex++;

    //                TestFrameActuations( core );

    //                if( frameIndex >= sprite->frameCount ) {
    //                    if( loops == true ) {
    //                        frameIndex = 0;
    //                        loopCount++;
    //                    }
    //                    else {
    //                        if( frameIndex >= sprite->frameCount ) {
    //                            frameIndex = sprite->frameCount - 1;
    //                            loopCount = 1;
    //                        }
    //                    }
    //                }
    //            }
    //            else {
    //                frameDelaySkip--;
    //            }
    //        }
    //    }
    //}

    //void SpriteAnimator::TestFrameActuations( Core * core ) {
    //    const i32 frameActuationCount = sprite->frameActuations.GetCount();
    //    for( i32 frameActuationIndex = 0; frameActuationIndex < frameActuationCount; frameActuationIndex++ ) {
    //        SpriteActuation & frameActuation = sprite->frameActuations[ frameActuationIndex ];
    //        if( frameActuation.frameIndex == frameIndex ) {
    //            if( frameActuation.audioResources.GetCount() > 0 ) {
    //                core->AudioPlayRandom( frameActuation.audioResources );
    //            }
    //        }
    //    }
    //}



}

/*
=====================================================================
===========================EDITOR FUNCS==============================
=====================================================================
*/

#if ATTO_EDITOR

namespace atto {

    //void Map::Editor_MapTilePlace( i32 xIndex, i32 yIndex, i32 spriteX, i32 spriteY, i32 flags ) {
    //    tileMap.PlaceTile( xIndex, yIndex, spriteX, spriteY, flags );
    //}

    //void Map::Editor_MapTileFillBorder( i32 spriteX, i32 spriteY, i32 flags ) {
    //    for( i32 yIndex = 0; yIndex < tileMap.tileYCount; yIndex++ ) {
    //        for( i32 xIndex = 0; xIndex < tileMap.tileXCount; xIndex++ ) {
    //            if( xIndex == 0 || xIndex == tileMap.tileXCount - 1 || yIndex == 0 || yIndex == tileMap.tileYCount - 1 ) {
    //                Editor_MapTilePlace( xIndex, yIndex, spriteX, spriteY, flags );
    //            }
    //        }
    //    }
    //}



}

#endif
