#include "atto_game_mode_game.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static i32 YSortEntities( Entity *& a, Entity *& b ) {
        return (i32)( b->pos.y - a->pos.y );
    }

    GameModeType GameMode_Game::GetGameModeType() {
        return GameModeType::IN_GAME;
    }

    bool GameMode_Game::IsInitialized() {
        return true;
    }

    void GameMode_Game::Initialize( Core * core ) {
        map.Start( core, startParms );
    }

    void GameMode_Game::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        map.UpdateAndRender( core, dt, flags );
    }

    void GameMode_Game::Shutdown( Core * core ) {
    }

    void MapLocal::Start( Core * core, const GameStartParams & parms ) {
        isMp = parms.isMutliplayer;
        localPlayerNumber = parms.localPlayerNumber;
        otherPlayerNumber = parms.otherPlayerNumber;
        isStarting = true;
        SpriteResource * playerSprite = core->ResourceGetAndCreateSprite( "res/ents/player/forward.json", 1, 32, 32, 0 );

        if( isMp ) {
            if( localPlayerNumber == 1 ) {
                isAuthority = true;
                localPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), glm::vec2( 25 * 32 - 20, 25 * 32 ) );
                localPlayer->collisionCollider.type = COLLIDER_TYPE_BOX;
                localPlayer->collisionCollider.box.CreateFromCenterSize( glm::vec2( 0, 0 ), glm::vec2( 36, 36 ) );
                localPlayer->playerNumber = localPlayerNumber;
                localPlayer->spriteAnimator.sprite = playerSprite;

                otherPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), glm::vec2( 25 * 32 + 20, 25 * 32 ) );
                otherPlayer->playerNumber = parms.otherPlayerNumber;
                otherPlayer->collisionCollider.type = COLLIDER_TYPE_BOX;
                otherPlayer->collisionCollider.box.CreateFromCenterSize( glm::vec2( 0, 0 ), glm::vec2( 36, 36 ) );
                otherPlayer->netStreamed = true;
                otherPlayer->netVisualPos = otherPlayer->pos;
                otherPlayer->spriteAnimator.sprite = playerSprite;
            }
            else {
                otherPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), glm::vec2( 25 * 32 - 20, 25 * 32 ) );
                otherPlayer->playerNumber = parms.otherPlayerNumber;
                otherPlayer->collisionCollider.type = COLLIDER_TYPE_BOX;
                otherPlayer->collisionCollider.box.CreateFromCenterSize( glm::vec2( 0, 0 ), glm::vec2( 36, 36 ) );
                otherPlayer->netVisualPos = otherPlayer->pos;
                otherPlayer->spriteAnimator.sprite = playerSprite;
                otherPlayer->netStreamed = true;

                localPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), glm::vec2( 25 * 32 - 20, 25 * 32 ) );
                localPlayer->playerNumber = localPlayerNumber;
                localPlayer->pos = glm::vec2( 25 * 32 + 20, 25 * 32 );
                localPlayer->spriteAnimator.sprite = playerSprite;
                localPlayer->collisionCollider.type = COLLIDER_TYPE_BOX;
                localPlayer->collisionCollider.box.CreateFromCenterSize( glm::vec2( 0, 0 ), glm::vec2( 36, 36 ) );
            }
        }
        else {
            isAuthority = true;
            localPlayer = SpawnEntity( core, EntityType::Make( EntityType::PLAYER ), glm::vec2( 25 * 32 - 20, 25 * 32 ) );
            localPlayer->playerNumber = localPlayerNumber;
            localPlayer->pos = glm::vec2( 25 * 32 - 20, 25 * 32 );
            localPlayer->spriteAnimator.sprite = playerSprite;
            localPlayer->collisionCollider.type = COLLIDER_TYPE_BOX;
            localPlayer->collisionCollider.box.CreateFromCenterSize( glm::vec2( 0, 0 ), glm::vec2( 36, 36 ) );
        }

        players.Add( localPlayer );
        players.Add( otherPlayer );

        //Spawn_EnemyBotBig( core, glm::vec2( 300, 150 ) );

        static SpriteResource * sprTile_Stone = core->ResourceGetAndLoadSprite( "res/sprites/stone_tiles/stone_tiles.json" );
        static SpriteResource * sprTile_Grass = core->ResourceGetAndLoadSprite( "res/sprites/grass_tiles/grass_tiles.json" );

        tileMap.tileXCount = 50;
        tileMap.tileYCount = 50;
        tileMap.tiles.SetCount( tileMap.tileXCount * tileMap.tileYCount );
        for( i32 y = 0; y < tileMap.tileYCount; y++ ) {
            for( i32 x = 0; x < tileMap.tileXCount; x++ ) {
                Editor_MapTilePlace( x, y, sprTile_Grass, Random::Int( 0, 8 ), Random::Int( 0, 8 ), 0 );
            }
        }

        Editor_MapTileFillBorder( sprTile_Stone, 3, 3, SPRITE_TILE_FLAG_NO_WALK );

        for( i32 i = 0; i < 7; i++ ) {
            SpawnEntity( core, EntityType::Make( EntityType::ENEMY_BOT_DRONE ), glm::vec2( 25 * 32, 25 * 32 ) );
        }

        isStarting = false;
    }

    void MapLocal::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        static TextureResource * sprUiPanel             = core->ResourceGetAndCreateTexture( "res/sprites/ui_ability_panel.png", false, false );
        static TextureResource * sprCharDroneSelection  = core->ResourceGetAndCreateTexture( "res/sprites/char_drone_selection.png", false, false );
        static TextureResource * sprParticleSingleWhite = core->ResourceGetAndCreateTexture( "res/sprites/particle_single_white_1x1.png", false, false );

        static SpriteResource * sprPlayerGun = core->ResourceGetAndCreateSprite( "res/ents/player/gun.json", 1, 32, 32, 0 );
        static SpriteResource * sprPlayerBullet = core->ResourceGetAndCreateSprite( "res/ents/player/bullet.json", 1, 8, 8, 0 );
        
        static SpriteResource * sprVFX_SmallExplody = core->ResourceGetAndLoadSprite( "res/sprites/vfx_small_explody/vfx_small_explody.json" );

        const f32 soundMinDist = 400;
        const f32 soundMaxDist = 10000;
        static AudioResource * sndCloseExplody1     = core->ResourceGetAndCreateAudio( "res/sounds/tomwinandysfx_explosions_volume_i_closeexplosion_01.wav", true, true, soundMinDist, soundMaxDist );
        static AudioResource * sndCloseExplody2     = core->ResourceGetAndCreateAudio( "res/sounds/tomwinandysfx_explosions_volume_i_closeexplosion_01.wav", true, true, soundMinDist, soundMaxDist );
        static FontHandle fontHandle                = core->ResourceGetFont( "default" );

        if( isMp == true ) {
            if( core->NetworkIsConnected() == true ) {
                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                while( core->NetworkRecieve( msg ) ) {
                    switch( msg.type ) {
                        case NetworkMessageType::ENTITY_POS_UPDATE:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                ent->pos = NetworkMessagePop<glm::vec2>( msg, offset );
                                ent->vel = NetworkMessagePop<glm::vec2>( msg, offset );
                            }
                        } break;
                        case NetworkMessageType::ENTITY_ANIM_UPDATE:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                i64 spriteId = NetworkMessagePop<i64>( msg, offset );
                                f32 framDuration = NetworkMessagePop<f32>( msg, offset );
                                bool isLooping = NetworkMessagePop<bool>( msg, offset );
                                SpriteResource * sprite = core->ResourceGetLoadedSprite( spriteId );
                                ent->spriteAnimator.frameDuration = framDuration;
                                ent->spriteAnimator.SetSpriteIfDifferent( core, sprite, isLooping );
                            }
                        } break;
                        case NetworkMessageType::ENTITY_AUDIO_PLAY:
                        {
                            i32 offset = 0;
                            SmallString id = NetworkMessagePop<SmallString>( msg, offset );
                            AudioResource * r = core->ResourceGetAndCreateAudio( id.GetCStr(), true, false, 30, 800 );
                            if( r != nullptr ) {
                                core->AudioPlay( r );
                            }
                        } break;
                        case NetworkMessageType::ENTITY_SPAWN:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                EntityType type = EntityType::Make( (EntityType::_enumerated)NetworkMessagePop<i32>( msg, offset ) );
                                glm::vec2 pos = NetworkMessagePop<glm::vec2>( msg, offset );
                                glm::vec2 vel = NetworkMessagePop<glm::vec2>( msg, offset );
                                SpawnEntity( core, type, pos, vel );
                            }
                        } break;
                        case NetworkMessageType::ENTITY_DESTROY:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                DestroyEntity( core, ent );
                            }
                        } break;
                        case NetworkMessageType::ENTITY_RPC_UNIT_TAKE_DAMAGE:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                i32 damage = NetworkMessagePop<i32>( msg, offset );
                                ent->Unit_TakeDamage( core, false, damage );
                            }
                            else {
                                core->LogOutput( LogLevel::ERR, "ENTITY_RPC_UNIT_TAKE_DAMAGE :: Could not find entity" );
                            }
                        } break;
                    }
                }
            }

            const f32 tickTime = 0.016f; // 60z
            static f32 dtAccumulator = 0.0f;
            dtAccumulator += dt;
            if( dtAccumulator >= tickTime ) {
                dtAccumulator = 0.0f;
                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                msg.isUDP = true;
                msg.type = NetworkMessageType::ENTITY_POS_UPDATE;
                NetworkMessagePush( msg, localPlayer->handle );
                NetworkMessagePush( msg, localPlayer->pos );
                NetworkMessagePush( msg, localPlayer->vel );
                core->NetworkSend( msg );
            }
        }

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0 );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 1 );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 2 );

        EntList & entities = *core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );

        //localCameraPos = ent->pos;
        const glm::vec2 camMin = localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f;
        const glm::vec2 camMax = localCameraPos + spriteDrawContext->GetCameraDims() / 2.0f;
        const glm::vec2 worldMin = glm::vec2( 0, 0 );
        const glm::vec2 worldMax = glm::vec2( tileMap.tileXCount * 32, tileMap.tileYCount * 32 );

        if( camMin.x < worldMin.x ) { localCameraPos.x = spriteDrawContext->GetCameraDims().x / 2.0f; }
        if( camMin.y < worldMin.y ) { localCameraPos.y = spriteDrawContext->GetCameraDims().y / 2.0f; }
        if( camMax.x > worldMax.x ) { localCameraPos.x = worldMax.x - spriteDrawContext->GetCameraDims().x / 2.0f; }
        if( camMax.y > worldMax.y ) { localCameraPos.y = worldMax.y - spriteDrawContext->GetCameraDims().y / 2.0f; }

        spriteDrawContext->SetCameraPos( localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f );
        debugDrawContext->SetCameraPos( localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f );
        core->AudioSetListener( localCameraPos );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

        const BoxBounds2D cameraWsBounds = { camMin, camMax };

        const i32 tileCount = tileMap.tiles.GetCount();
        for( i32 i = 0; i < tileCount; i++ ) {
            SpriteTile & tile = tileMap.tiles[ i ];
            if( tile.spriteResource != nullptr ) {
                if( tile.wsBounds.Intersects( cameraWsBounds ) == true ) {
                    spriteDrawContext->DrawSprite( tile.spriteResource, tile.spriteTileIndexX, tile.spriteTileIndexY, tile.center );
                }
            }
        }


        // @HACK:
        if( dt > 0.5f ) {
            dt = 0.016f;
        }

        static f32 spawnTimer = 0;
        spawnTimer += dt;
        if( spawnTimer > 5.0f && isMp && isAuthority ) {
            spawnTimer = 0.0f;
            SpawnEntity( core, EntityType::Make( EntityType::ENEMY_BOT_DRONE ), glm::vec2( 25 * 32, 25 * 32 ) );
        }

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            Entity * ent = entities[ entityIndexA ];

            if( ent->active == false ) {
                continue;
            }

            if( ent->netStreamed ) {
                ent->netVisualPos = glm::mix( ent->netVisualPos, ent->pos, 0.25f );
            }

            ent->spriteAnimator.Update( core, dt );

            const f32 entVel = glm::length( ent->vel );
            if( entVel > 50.0f ) {
                ent->facingDir = glm::sign( ent->vel.x );
            }

            if( ent->facingDir == 0.0 ) {
                ent->facingDir = 1.0f;
            }

            glm::vec4 colorMultiplier = glm::vec4( 1, 1, 1, 1 );

            glm::vec2 drawPos = ent->netStreamed ? ent->netVisualPos : ent->pos;
            if( ent->spriteAnimator.sprite != nullptr ) {
                spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, ent->ori, glm::vec2( ent->facingDir, 1.0f ), colorMultiplier );
            }

            ent->acc.x -= ent->vel.x * ent->resistance;
            ent->acc.y -= ent->vel.y * ent->resistance;
            ent->vel += ent->acc * dt;
            ent->pos += ent->vel * dt;

            ent->acc = glm::vec2( 0.0 );

            switch( ent->type ) {
                case EntityType::PLAYER:
                {
                    if( ent->playerNumber == localPlayerNumber ) {

                        PlayerStuff & player = ent->playerStuff;

                        const f32 playerSpeed = 2500.0f / 2.0f;

                        bool getInput = true;

                        if( getInput == true ) {
                            if( core->InputKeyDown( KeyCode::KEY_CODE_W ) ) {
                                ent->acc.y += 1;
                            }
                            if( core->InputKeyDown( KeyCode::KEY_CODE_S ) ) {
                                ent->acc.y -= 1;
                            }
                            if( core->InputKeyDown( KeyCode::KEY_CODE_A ) ) {
                                ent->acc.x -= 1;
                            }
                            if( core->InputKeyDown( KeyCode::KEY_CODE_D ) ) {
                                ent->acc.x += 1;
                            }

                            if( ent->acc != glm::vec2( 0, 0 ) ) {
                                ent->acc = glm::normalize( ent->acc ) * playerSpeed;
                            }
                        }

                        //debugDrawContext->DrawRect( ent->pos, glm::vec2( 5, 5 ), 0.0f, glm::vec4( 0.2f, 0.8f, 0.2f, 0.5f ) );

                        Collider2D wsCollider = ent->GetWorldCollisionCollider();

                        FixedList<SpriteTile *, 9> apron = {};
                        i32 entTileXPos = (i32)( ent->pos.x / 32.0f );
                        i32 entTileYPos = (i32)( ent->pos.y / 32.0f );
                        tileMap.GetApron( entTileXPos, entTileYPos, apron );
                        const i32 apronCount = apron.GetCount();
                        for( i32 tileIndex = 0; tileIndex < apronCount; tileIndex++ ) {
                            SpriteTile * tile = apron[ tileIndex ];
                            //debugDrawContext->DrawRect( tile->center, glm::vec2( 30, 30 ), 0.0f, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );
                            //debugDrawContext->DrawRect( tile->wsBounds.min, tile->wsBounds.max, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );
                            if( EnumHasFlag( tile->flags, SPRITE_TILE_FLAG_NO_WALK ) == true ) {
                                Manifold2D mani = {};
                                if( wsCollider.Collision( tile->wsBounds, mani ) == true ) {
                                    ent->pos -= mani.normal * mani.penetration; 
                                    //ent->vel = glm::dot( ent->vel, -mani.normal ) * ent->vel; TODO: Adjust velo accordingly
                                    wsCollider = ent->GetWorldCollisionCollider();
                                }
                            }
                        }

                        //debugDrawContext->DrawRect( wsCollider.box.min, wsCollider.box.max, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );

                        localCameraPos = glm::mix( localCameraPos, ent->pos, dt * 4.0f );

                        // Spring forumla 
                        // f = -kx - bv
                        // k = spring constant
                        // x = displacement
                        // b = damping constant
                        // v = velocity
                        player.weaponPos = glm::mix( player.weaponPos, ent->pos, 0.25f );
                        player.weaponTimer -= dt;
                        if( player.weaponTimer < 0.0f ) {
                            player.weaponTimer = 0.0f;
                        }

                        glm::vec2 dir = glm::normalize( ent->pos - mousePosWorld );
                        if( player.weaponTimer == 0.0f && core->InputMouseButtonDown( MOUSE_BUTTON_1 ) == true ) {
                            player.weaponTimer += 0.1f;
                            player.weaponPos = player.weaponPos + dir * 2.0f;
                            
                            Entity * b = SpawnEntity( core, EntityType::Make( EntityType::BULLET ), player.weaponPos - dir * 10.0f, -dir * 250.0f );
                            
                        }


                        constexpr f32 pi = glm::pi<f32>();
                        constexpr f32 halfPi = pi / 2.0f;

                        f32 angle = glm::atan( dir.x, dir.y ) + halfPi;
                        if( angle >= -halfPi && angle <= halfPi ) {
                            spriteDrawContext->DrawSprite( sprPlayerGun, 0, player.weaponPos, angle, glm::vec2( 1.0f, 1.0f ) );
                        }
                        else {
                            spriteDrawContext->DrawSprite( sprPlayerGun, 0, player.weaponPos, angle - pi, glm::vec2( -1.0f, 1.0f ) );
                        }
                    }

                    //{
                    //    ent->spriteAnimator.sprite = sprWarriorStab;
                    //    static int frameIndex = 0;
                    //    if( core->InputKeyJustPressed( KeyCode::KEY_CODE_Z ) ) {
                    //        frameIndex++;
                    //    }
                    //    frameIndex = frameIndex % ent->spriteAnimator.sprite->frameCount;
                    //    spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, frameIndex, ent->pos, ent->ori, glm::vec2( ent->facingDir, 1.0f ) );
                    //}

                    //SmallString vv = StringFormat::Small( "%f", glm::length( ent->vel ) );
                    //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 200 ), 32, vv.GetCStr() );

                } break;
                case EntityType::ENEMY_BOT_DRONE:
                {
                    UnitStuff & unit = ent->unitStuff;
                    Navigator & nav = ent->navigator;

                    const f32 alertRad = 55.0f;

                    switch( unit.state ) {
                        case UNIT_STATE_IDLE:
                        {
                            nav.dest = ent->pos + glm::vec2( Random::Float( -alertRad, alertRad ), Random::Float( -alertRad, alertRad ) );
                            unit.state = UNIT_STATE_WANDERING;
                        } break;
                        case UNIT_STATE_TAKING_DAMAGE:
                        {
                            unit.takingDamageTimer -= dt;
                            if( unit.takingDamageTimer <= 0.0f ) {
                                unit.takingDamageTimer = 0.0f;
                                unit.state = UNIT_STATE_WANDERING;
                            }
                            else {
                                colorMultiplier = glm::vec4( 100, 100, 100, 1 );
                            }
                        } break;
                        case UNIT_STATE_EXPLODING:
                        {
                            ent->spriteAnimator.SetSpriteIfDifferent( core, sprVFX_SmallExplody, false );
                            if( unit.playedDeathSound == false && ent->spriteAnimator.frameIndex == 1 ) {
                                unit.playedDeathSound = true;
                                core->AudioPlayRandom( nullptr, sndCloseExplody1, sndCloseExplody2 );
                            }

                            if( ent->spriteAnimator.loopCount > 0 ) {
                                ent->active = false; // Trick to hide the entity in laggy mp enviroments. We need to wait for the destroy entity rpc...
                                DestroyEntity( core, ent );
                            }
                        } break;
                        case UNIT_STATE_WANDERING:
                        {
                            if( isAuthority == false ) {
                                break;
                            }

                            f32 distToPlayer = 0.0f;
                            Entity * closePlayer = ClosestPlayerTo( ent->pos, distToPlayer );

                            if( distToPlayer < 50.0f ) {
                                const f32 r = 25.0f;
                                nav.dest = closePlayer->pos + glm::vec2( Random::Float( -r, r ), Random::Float( -r, r ) );
                                unit.state = UNIT_STATE_SWARM;
                                break;
                            }

                            // @SPEED
                            //debugDrawContext->DrawRect( nav.dest, glm::vec2( 5 ), 0.0f );
                            f32 dist = glm::distance( nav.dest, ent->pos );
                            if( dist < 5.0f ) {
                                nav.dest = ent->pos + glm::vec2( Random::Float( -alertRad, alertRad ), Random::Float( -alertRad, alertRad ) );
                                unit.state = UNIT_STATE_WANDERING;
                            }
                            else {
                                const f32 speed = 150.0f;
                                const f32 resistance = 5.0f;

                                ent->acc = glm::normalize( nav.dest - ent->pos ) * speed;
                            }
                        } break;
                        case UNIT_STATE_SWARM:
                        {
                            if( isAuthority == false ) {
                                break;
                            }

                            // @SPEED
                            f32 dist = glm::distance( nav.dest, ent->pos );
                            if( dist < 5.0f ) {
                                const f32 r = alertRad;
                                f32 distToPlayer = 0.0f;
                                Entity * closePlayer = ClosestPlayerTo( ent->pos, distToPlayer );
                                nav.dest = closePlayer->pos + glm::vec2( Random::Float( -r, r ), Random::Float( -r, r ) );
                            }
                            else {
                                const f32 speed = 1500.0f;
                                const f32 resistance = 5.0f;
                                ent->acc = glm::normalize( nav.dest - ent->pos ) * speed;
                            }

                        } break;
                    }

                    if( ent->particleSystem.emitting == true ) {
                        ParticleSystem & part = ent->particleSystem;
                        part.spawnTimer += dt;

                        for( i32 partIndex = 0; partIndex < part.count; partIndex++ ) {
                            Particle & p = part.particles[ partIndex ];
                            p.lifeTime -= dt;
                            if( p.lifeTime < 0 ) {
                                p.lifeTime = 0.0f;
                            }

                            f32 l = p.lifeTime / part.lifeTime;
                            p.scale = glm::mix( part.scaleMin, part.scaleMax, l );
                            p.pos += p.vel * dt;
                        }

                        bool allDead = true;
                        for( i32 partIndex = 0; partIndex < part.count; partIndex++ ) {
                            Particle & p = part.particles[ partIndex ];
                            if( p.lifeTime != 0.0f ) {
                                allDead = false;
                                spriteDrawContext->DrawTexture( part.texture, p.pos, 0.0f, glm::vec2( p.scale ) );
                            }
                        }
                    }

                    if( isMp == true && isAuthority == true ) {
                        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                        msg.type = NetworkMessageType::ENTITY_POS_UPDATE;
                        NetworkMessagePush( msg, ent->handle );
                        NetworkMessagePush( msg, ent->pos );
                        NetworkMessagePush( msg, ent->vel );
                        core->NetworkSend( msg );
                    }

                    // Debug
                    //debugDrawContext->DrawRect( ent->pos, glm::vec2( alertRad ), 0.0f, glm::vec4( 0.2f, 0.2f, 0.75f, 0.76f ) );

                } break;
                case EntityType::BULLET:
                {
                    for( int otherEntityIndex = 0; otherEntityIndex < entityCount; otherEntityIndex++ ) {
                        Entity * enemy = entities[ otherEntityIndex ];
                        if( enemy->type == EntityType::ENEMY_BOT_DRONE ) {
                            if( enemy->active == false ) {
                                continue;
                            }

                            Collider2D bulletCollider = ent->GetWorldCollisionCollider();
                            Collider2D enemyCollider = enemy->GetWorldCollisionCollider();
                            if( bulletCollider.Intersects( enemyCollider ) ) {
                                DestroyEntity( core, ent );
                                enemy->Unit_TakeDamage( core, true, 50 );
                            }
                        }
                    }
                } break;
            }
         
        }

        if( false ) {
            for( int i = 0; i < entityCount; i++ ) {
                const Entity * ent = entities[ i ];
                const glm::vec4 selectionColor( 0.8f, 0.8f, 0.5f, 0.4f );
                switch( ent->selectionCollider.type ) {
                    case COLLIDER_TYPE_CIRCLE:
                    {
                        Collider2D c = ent->GetWorldSelectionCollider();
                        glm::vec2 r = glm::vec2( c.circle.rad );
                        spriteDrawContext->DrawRect( c.circle.pos - r, c.circle.pos + r, selectionColor );

                        // @TODO: Fix
                        //spriteDrawContext->DrawCircle( glm::vec2( 200, 200 ), 100 );
                        //spriteDrawContext->DrawCircle( c.circle.pos, c.circle.rad );
                    } break;
                    case COLLIDER_TYPE_BOX:
                    {
                        Collider2D b = ent->GetWorldSelectionCollider();
                        spriteDrawContext->DrawRect( b.box.min, b.box.max, selectionColor );
                    }
                }
            }
        }

        //spriteDrawContext->DrawSprite( sprCharge, 1, mousePosWorld );

        //SmallString s =StringFormat::Small( "d=%d", (i32)core->InputKeyDown( KeyCode::KEY_CODE_D ) );
        //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );
        //SmallString ss = StringFormat::Small( "a=%d", (i32)core->InputKeyDown( KeyCode::KEY_CODE_A ) );
        //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 168 ), 32, ss.GetCStr() );

        SmallString s = StringFormat::Small( "ping=%d", (i32)core->NetworkGetPing() );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );
        s = StringFormat::Small( "dt=%f", dt );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 160 ), 32, s.GetCStr() );
        s = StringFormat::Small( "fps=%f", 1.0f / dt );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 200 ), 32, s.GetCStr() );

        core->RenderSubmit( spriteDrawContext, true );
        core->RenderSubmit( uiDrawContext, false );
        core->RenderSubmit( debugDrawContext, false );


        //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, "Hello World" );
        //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, startParms.localPlayerNumber == 1 ? "1" : "2" );
    }

    Entity * MapLocal::SpawnEntity( Core * core, EntityType type, glm::vec2 pos, glm::vec2 vel ) {
        if( isAuthority == true || isStarting == true ) {
            EntityHandle handle = {};
            Entity * entity = entityPool.Add( handle );
            AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
            if( entity != nullptr ) {
                ZeroStructPtr( entity );
                entity->active = true;
                entity->handle = handle;
                entity->type = type;
                entity->facingDir = 1.0f;
                entity->name = "Mr No Name";
                entity->resistance = 14.0f;
                entity->pos = pos;
                entity->vel = vel;
                entity->netVisualPos = pos;
                entity->map = this;
                entity->netStreamed = !isAuthority;

                switch( type ) {
                    case EntityType::ENEMY_BOT_DRONE:
                    {
                        entity->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
                        entity->selectionCollider.circle.pos = glm::vec2( -0.5f, -0.5f );
                        entity->selectionCollider.circle.rad = 5.0f;
                        entity->collisionCollider = entity->selectionCollider;
                        entity->maxHealth = 100;
                        entity->currentHealth = entity->maxHealth;
                        entity->currentHealth = entity->maxHealth;
                        static SpriteResource * spriteResource = core->ResourceGetAndCreateSprite( "res/ents/char_bot_drone/bot_drone_smol.json", 7, 16, 16, 10 );
                        entity->spriteAnimator.SetSpriteIfDifferent( core, spriteResource, false );
                    } break;
                    case EntityType::BULLET:
                    {
                        entity->resistance = 0.0f;
                        entity->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "res/ents/player/bullet.json", 1, 8, 8, 0 );
                        entity->collisionCollider.type = COLLIDER_TYPE_BOX;
                        entity->collisionCollider.box.min = glm::vec2( -5.0f );
                        entity->collisionCollider.box.max = glm::vec2( 5.0f );
                    } break;
                }

                if( isMp == true && isStarting == false ) {
                    NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                    msg.type = NetworkMessageType::ENTITY_SPAWN;
                    NetworkMessagePush( msg, entity->handle );
                    NetworkMessagePush( msg, (i32)entity->type );
                    NetworkMessagePush( msg, pos );
                    NetworkMessagePush( msg, vel );
                    core->NetworkSend( msg );
                }
            }

            return entity;
        }
        else {
            Entity & entity = temporyEntities.AddEmpty();
            entity.netTempId = netTempId;
            if( isMp == true && isStarting == false ) {
                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                msg.type = NetworkMessageType::ENTITY_SPAWN_REQUEST;
                NetworkMessagePush( msg, netTempId );
                NetworkMessagePush( msg, (i32)entity.type );
                NetworkMessagePush( msg, pos );
                NetworkMessagePush( msg, vel );
                NetworkMessagePush( msg, netTempId );
                core->NetworkSend( msg );
            }

            netTempId++;
        }
    }

    void MapLocal::DestroyEntity( Core * core, Entity * entity ) {
        if( entity != nullptr ) {
            entityPool.Remove( entity->handle );
            if( isMp == true && isAuthority == true ) {
                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                msg.type = NetworkMessageType::ENTITY_DESTROY;
                NetworkMessagePush( msg, entity->handle );
                core->NetworkSend( msg );
            }
        }
    }

    Entity * MapLocal::ClosestPlayerTo( glm::vec2 p, f32 & dist ) {
        dist = FLT_MAX;
        Entity * closePlayer = nullptr;
        const int playerCount = players.GetCount();
        for( i32 playerIndex = 0; playerIndex < playerCount; playerIndex++ ) {
            Entity * ent = players[ playerIndex ];
            if( ent != nullptr ) {
                f32 d = glm::distance( ent->pos, p );
                if( d < dist ) {
                    dist = d;
                    closePlayer = ent;
                }
            }
        }

        return closePlayer;
    }

    inline static Collider2D ColliderForSpace( const Collider2D & base, glm::vec2 p ) {
        Collider2D c = base;
        switch( base.type ) {
            case COLLIDER_TYPE_CIRCLE:
            {
                c.circle.pos += p;
            } break;
            case COLLIDER_TYPE_BOX:
            {
                c.box.Translate( p );
            } break;
            default:
            {
                INVALID_CODE_PATH;
            } break;
        }

        return c;
    }


    void Entity::Unit_TakeDamage( Core * core, bool sendPacket, i32 damage ) {
        currentHealth -= damage;

        if( currentHealth < 0 ) {
            currentHealth = 0;
            unitStuff.state = UNIT_STATE_EXPLODING;
            spriteAnimator.frameDelaySkip = Random::Int( 3 );
            return;
        }

        unitStuff.state = UNIT_STATE_TAKING_DAMAGE;
        unitStuff.takingDamageTimer = 0.1f;

        ZeroStruct( particleSystem );
        particleSystem.count = 10;
        particleSystem.texture = core->ResourceGetAndCreateTexture( "res/sprites/particle_single_white_1x1.png", false, false );;
        particleSystem.lifeTime = 0.5f;
        particleSystem.scaleMin = 1;
        particleSystem.scaleMax = 2;
        particleSystem.velMin = glm::vec2( -100.0f );
        particleSystem.velMax = glm::vec2( 100.0f );
        particleSystem.oneShot = true;

        ParticleSystem & part = particleSystem;
        particleSystem.emitting = true;
        for( i32 partIndex = 0; partIndex < particleSystem.count; partIndex++ ) {
            Particle & p = part.particles[ partIndex ];
            p.pos = pos;
            p.lifeTime = part.lifeTime;
            p.scale = Random::Float( part.scaleMin, part.scaleMax );
            p.vel = Random::Vec2( part.velMin, part.velMax );
        }

        if( sendPacket == true ) {
            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
            msg.type = NetworkMessageType::ENTITY_RPC_UNIT_TAKE_DAMAGE;
            NetworkMessagePush( msg, handle );
            NetworkMessagePush( msg, damage );
            core->NetworkSend( msg );
        }
    }

    Collider2D Entity::GetWorldCollisionCollider() const {
        return ColliderForSpace( collisionCollider, pos );
    }

    Collider2D Entity::GetWorldSelectionCollider() const {
        return ColliderForSpace( selectionCollider, pos );
    }

    void SpriteAnimator::SetFrameRate( f32 fps ) {
        frameDuration = 1.0f / fps;
    }

    bool SpriteAnimator::SetSpriteIfDifferent( Core * core, SpriteResource * sprite, bool loops ) {
        if( this->sprite != sprite ) {
            this->sprite = sprite;
            SetFrameRate( (f32)sprite->frameRate );
            frameIndex = 0;
            frameTimer = 0;
            loopCount = 0;
            this->loops = loops;
            TestFrameActuations( core );
            return true;
        }
        return false;
    }

    void SpriteAnimator::Update( Core * core, f32 dt ) {
        if( sprite != nullptr && sprite->frameCount > 1 ) {
            frameTimer += dt;
            if( frameTimer >= frameDuration ) {
                frameTimer -= frameDuration;
                if( frameDelaySkip == 0 ) {
                    frameIndex++;

                    TestFrameActuations( core );

                    if( frameIndex >= sprite->frameCount ) {
                        if( loops == true ) {
                            frameIndex = 0;
                            loopCount++;
                        }
                        else {
                            if( frameIndex >= sprite->frameCount ) {
                                frameIndex = sprite->frameCount - 1;
                                loopCount = 1;
                            }
                        }
                    }
                }
                else {
                    frameDelaySkip--;
                }
            }
        }
    }

    void SpriteAnimator::TestFrameActuations( Core * core ) {
        const i32 frameActuationCount = sprite->frameActuations.GetCount();
        for( i32 frameActuationIndex = 0; frameActuationIndex < frameActuationCount; frameActuationIndex++ ) {
            SpriteActuation & frameActuation = sprite->frameActuations[ frameActuationIndex ];
            if( frameActuation.frameIndex == frameIndex ) {
                if( frameActuation.audioResources.GetCount() > 0 ) {
                    core->AudioPlayRandom( frameActuation.audioResources );
                }
            }
        }
    }

    void SpriteTileMap::GetApron( i32 x, i32 y, FixedList<SpriteTile *, 9> & apron ) {
        for( i32 yIndex = -1; yIndex <= 1; yIndex++ ) {
            for( i32 xIndex = -1; xIndex <= 1; xIndex++ ) {
                i32 xCheck = x + xIndex;
                i32 yCheck = y + yIndex;
                if( xCheck >= 0 && xCheck < tileXCount && yCheck >= 0 && yCheck < tileYCount ) {
                    i32 flatIndex = yCheck * tileXCount + xCheck;
                    SpriteTile * tile = &tiles[ flatIndex ];
                    apron.Add( tile );
                }
            }
        }
    }

}

/*
=====================================================================
===========================EDITOR FUNCS==============================
=====================================================================
*/

#if ATTO_EDITOR

namespace atto {

    void MapLocal::Editor_MapTilePlace( i32 xIndex, i32 yIndex, SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags ) {
        SpriteTile tile = {};
        tile.xIndex = xIndex;
        tile.yIndex = yIndex;
        tile.flatIndex = yIndex * tileMap.tileXCount + xIndex;
        tile.spriteResource = sprite;
        tile.spriteTileIndexX = spriteX;
        tile.spriteTileIndexY = spriteY;
        tile.flags = flags;

        const f32 TILE_SIZE = 32.0f;
        tile.center = glm::vec2( xIndex * TILE_SIZE + TILE_SIZE * 0.5f, yIndex * TILE_SIZE + TILE_SIZE * 0.5f );
        tile.wsBounds.min = tile.center + glm::vec2( -TILE_SIZE / 2.0f, -TILE_SIZE / 2.0f );
        tile.wsBounds.max = tile.center + glm::vec2( TILE_SIZE / 2.0f, TILE_SIZE / 2.0f );
        tileMap.tiles[ tile.flatIndex ] = tile;
    }

    void MapLocal::Editor_MapTileFillBorder( SpriteResource * sprite, i32 spriteX, i32 spriteY, i32 flags ) {
        for( i32 yIndex = 0; yIndex < tileMap.tileYCount; yIndex++ ) {
            for( i32 xIndex = 0; xIndex < tileMap.tileXCount; xIndex++ ) {
                if( xIndex == 0 || xIndex == tileMap.tileXCount - 1 || yIndex == 0 || yIndex == tileMap.tileYCount - 1 ) {
                    Editor_MapTilePlace( xIndex, yIndex, sprite, spriteX, spriteY, flags );
                }
            }
        }
    }

}

#endif
