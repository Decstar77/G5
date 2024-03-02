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

    void Map::Start( Core * core, const GameStartParams & parms ) {
        isMp = parms.isMutliplayer;
        localPlayerNumber = parms.localPlayerNumber;
        otherPlayerNumber = parms.otherPlayerNumber;

        if( isMp ) {
            if( localPlayerNumber == 1 ) {
                hasAuthority = true;
                localPlayer = SpawnEntity( ENTITY_TYPE_PLAYER );
                localPlayer->playerNumber = localPlayerNumber;
                localPlayer->pos = glm::vec2( 200, 230 );
                localPlayer->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "asset_pack_01/player_idle", 10, 48, 48, 16 );

                otherPlayer = SpawnEntity( ENTITY_TYPE_PLAYER );
                otherPlayer->playerNumber = parms.otherPlayerNumber;
                otherPlayer->netStreamed = true;
                otherPlayer->pos = glm::vec2( 100, 230 );
                otherPlayer->netVisualPos = otherPlayer->pos;
                otherPlayer->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "asset_pack_01/player_idle", 10, 48, 48, 16 );
            }
            else {
                otherPlayer = SpawnEntity( ENTITY_TYPE_PLAYER );
                otherPlayer->playerNumber = parms.otherPlayerNumber;
                otherPlayer->pos = glm::vec2( 200, 230 );
                otherPlayer->netVisualPos = otherPlayer->pos;
                otherPlayer->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "asset_pack_01/player_idle", 10, 48, 48, 16 );
                otherPlayer->netStreamed = true;

                localPlayer = SpawnEntity( ENTITY_TYPE_PLAYER );
                localPlayer->playerNumber = localPlayerNumber;
                localPlayer->pos = glm::vec2( 100, 230 );
                localPlayer->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "asset_pack_01/player_idle", 10, 48, 48, 16 );
            }
        }
        else {
            hasAuthority = true;
            localPlayer = SpawnEntity( ENTITY_TYPE_PLAYER );
            localPlayer->playerNumber = localPlayerNumber;
            localPlayer->pos = glm::vec2( 200, 230 );
            localPlayer->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "asset_pack_01/player_idle", 10, 48, 48, 16 );
        }

        players.Add( localPlayer );
        players.Add( otherPlayer );

        SpawnDrone( glm::vec2( 200, 100 ) );
        SpawnDrone( glm::vec2( 220, 100 ) );
        SpawnDrone( glm::vec2( 240, 100 ) );
        SpawnDrone( glm::vec2( 260, 100 ) );
        SpawnDrone( glm::vec2( 280, 100 ) );
        SpawnDrone( glm::vec2( 300, 100 ) );
    }

    void Map::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        static TextureResource * tile = core->ResourceGetAndLoadTexture( "tile_dark_metal_1.png", false, false );
        static TextureResource * sprUiPanel = core->ResourceGetAndLoadTexture( "ui_ability_panel.png", false, false );

        static TextureResource * sprCharDroneSelection = core->ResourceGetAndLoadTexture( "char_drone_selection.png", false, false );
        static TextureResource * sprParticleSingleWhite = core->ResourceGetAndLoadTexture( "particle_single_white_1x1.png", false, false );

        static SpriteResource * sprWarriorIdle = core->ResourceGetAndCreateSprite( "asset_pack_01/player_idle", 10, 48, 48, 16 );
        static SpriteResource * sprWarriorRun = core->ResourceGetAndCreateSprite( "asset_pack_01/player_run", 8, 48, 48, 16 );
        static SpriteResource * sprWarriorStab = core->ResourceGetAndCreateSprite( "asset_pack_01/player_sword_stab", 7, 96, 48, 16 );
        static SpriteResource * sprWarriorStrike = core->ResourceGetAndCreateSprite( "asset_pack_01/basic_sword_attack", 6, 64, 64, 16 );
        //static SpriteResource * sprWarriorCharge = core->ResourceGetAndCreateSprite( "asset_pack_01/player_katana_continuous_attack", 9, 80, 64, 16 );
        static SpriteResource * sprWarriorCharge = core->ResourceGetAndLoadSprite( "asset_pack_01/player_katana_continuous_attack" );

        static SpriteResource * sprCharDrone = core->ResourceGetAndCreateSprite( "char_drone_01", 1, 32, 32, 1 );
        //static SpriteResource * sprVFX_SmallExplody= core->ResourceGetAndCreateSprite( "vfx_small_explody", 3, 32, 32, 10 );
        static SpriteResource * sprVFX_SmallExplody = core->ResourceGetAndLoadSprite( "vfx_small_explody" );
        
        static AudioResource * sndWarriorStrike1 = core->ResourceGetAndLoadAudio( "res/sounds/not_legal/lightsaber_quick_1.wav" );
        static AudioResource * sndWarriorStrike2 = core->ResourceGetAndLoadAudio( "res/sounds/not_legal/lightsaber_quick_3.wav" );
        static AudioResource * sndWarriorStab1 = core->ResourceGetAndLoadAudio( "res/sounds/not_legal/lightsaber_quick_2.wav" );
        static AudioResource * sndWarriorStab2 = core->ResourceGetAndLoadAudio( "res/sounds/not_legal/lightsaber_quick_4.wav" );
        static AudioResource * sndWarriorCharge1 = core->ResourceGetAndLoadAudio( "res/sounds/not_legal/lightsaber_clash_1.wav" );
        static AudioResource * sndWarriorCharge2 = core->ResourceGetAndLoadAudio( "res/sounds/not_legal/lightsaber_clash_2.wav" );

        static AudioResource * sndCloseExplody1 = core->ResourceGetAndLoadAudio( "res/sounds/tomwinandysfx_explosions_volume_i_closeexplosion_01.wav" );
        static AudioResource * sndCloseExplody2 = core->ResourceGetAndLoadAudio( "res/sounds/tomwinandysfx_explosions_volume_i_closeexplosion_01.wav" );

        static FontHandle fontHandle = core->ResourceGetFont( "default" );

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
                            AudioResource * r = core->ResourceGetAndLoadAudio( id.GetCStr() );
                            if( r != nullptr ) {
                                core->AudioPlay( r );
                            }

                        } break;
                        case NetworkMessageType::ENTITY_DESTROY:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            entityPool.Remove( handle );
                        } break;
                        case NetworkMessageType::ENTITY_RPC_UNIT_TAKE_DAMAGE:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                i32 damage= NetworkMessagePop<i32>( msg, offset );
                                ent->Unit_TakeDamage( core, false, damage );
                            }
                        } break;
                        case NetworkMessageType::ENTITY_RPC_UNIT_DIE:
                        {
                            i32 offset = 0;
                            EntityHandle handle = NetworkMessagePop<EntityHandle>( msg, offset );
                            Entity * ent = entityPool.Get( handle );
                            if( ent != nullptr ) {
                                ent->Unit_Die( core, false );
                            }
                        } break;
                    }
                }
            }

            const f32 tickTime = 0.016f; // 60z
            static f32 dtAccumulator = 0.0f;
            dtAccumulator += dt;
            if ( dtAccumulator >= tickTime ) {
                dtAccumulator = 0.0f;
                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                msg.type = NetworkMessageType::ENTITY_POS_UPDATE;
                NetworkMessagePush( msg, localPlayer->handle );
                NetworkMessagePush( msg, localPlayer->pos );
                NetworkMessagePush( msg, localPlayer->vel );
                core->NetworkSend( msg );

                ZeroStruct( msg );
                msg.type = NetworkMessageType::ENTITY_ANIM_UPDATE;
                NetworkMessagePush( msg, localPlayer->handle );
                NetworkMessagePush( msg, localPlayer->spriteAnimator.sprite->spriteId );
                NetworkMessagePush( msg, localPlayer->spriteAnimator.frameDuration ); // This is temporary
                NetworkMessagePush( msg, localPlayer->spriteAnimator.loops );// This is temporary
                core->NetworkSend( msg );
            }
        }

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0 );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 1 );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 2 );

        EntList & entities = * core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );
        spriteDrawContext->SetCameraPos( localCameraPos - spriteDrawContext->GetCameraDims() / 2.0f );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

        for( int y = 0; y < 10; y++ ) {
            for( int x = 0; x < 10; x++ ) {
                spriteDrawContext->DrawTexture( tile, glm::vec2( x * 32, y * 32 ) );
            }
        }


        // @HACK:
        if( dt > 0.5f ) {
            dt = 0.016f;
        }

        const i32 entityCount =  entities.GetCount();
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            Entity * ent = entities[ entityIndexA ];

            if( ent->active == false ) {
                continue;
            }

            if( ent->netStreamed ) {
                //f32 extrapoFactor = 0.025f;
                ent->netVisualPos = glm::mix( ent->netVisualPos, ent->pos, 0.25f );
                //ent->pos = ent->netDesiredPos;
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

            switch( ent->type ) {
                case ENTITY_TYPE_PLAYER: {
                    if( ent->playerNumber == localPlayerNumber ) {
                        PlayerStuff & player = ent->playerStuff;

                        const f32 playerSpeed = 2500.0f;
                        const f32 resistance = 14.0f;

                        bool getInput = true;
                        if( player.currentAbility != NULL ) {
                            getInput = !player.currentAbility->stopsMovement;
                        }

                        glm::vec2 acc = glm::vec2( 0 );

                        if( getInput == true ) {
                            if( core->InputKeyDown( KeyCode::KEY_CODE_W ) ) {
                                acc.y += 1;
                            }
                            if( core->InputKeyDown( KeyCode::KEY_CODE_S ) ) {
                                acc.y -= 1;
                            }
                            if( core->InputKeyDown( KeyCode::KEY_CODE_A ) ) {
                                acc.x -= 1;
                            }
                            if( core->InputKeyDown( KeyCode::KEY_CODE_D ) ) {
                                acc.x += 1;
                            }

                            if( acc != glm::vec2( 0, 0 ) ) {
                                acc = glm::normalize( acc ) * playerSpeed;
                            }
                        }

                        acc.x -= ent->vel.x * resistance;
                        acc.y -= ent->vel.y * resistance;

                        ent->vel += acc * dt;
                        ent->pos += ent->vel * dt;


                        for( int abilityIndex = 0; abilityIndex < MAX_ABILITIES; abilityIndex++ ) {
                            Ability & ab = ent->playerStuff.abilities[ abilityIndex ];
                            ab.cooldownTimer -= dt;
                            if( ab.cooldownTimer < 0.0f ) {
                                ab.cooldownTimer = 0.0f;
                            }
                        }

                        localCameraPos = glm::mix( localCameraPos, ent->pos, dt );

                        const f32 playerVel = glm::length( ent->vel );

                        if( player.primingAbility != nullptr ) {
                            spriteDrawContext->DrawTexture( sprCharDroneSelection, mousePosWorld, 0.0f );
                        }

                        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                            if( player.primingAbility == nullptr ) {
                                Ability & ab = ent->playerStuff.abilities[ 0 ];
                                if( ab.cooldownTimer == 0.0f ) {
                                    // @HACK(SETTINGS):
                                    ab.cooldown = 0.5f;
                                    ab.type = ABILITY_TYPE_WARRIOR_STRIKE;

                                    ab.cooldownTimer = ab.cooldown;
                                    ab.sprite = sprWarriorStrike;
                                    player.state = PLAYER_STATE_ATTACKING;
                                    player.currentAbility = &ab;
                                    AudioResource * playedSound = core->AudioPlayRandom( 1.0f, false, sndWarriorStrike1, sndWarriorStrike2 );

                                    if( isMp ) {
                                        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                                        msg.type = NetworkMessageType::ENTITY_AUDIO_PLAY;
                                        NetworkMessagePush( msg, playedSound->name );
                                        core->NetworkSend( msg );
                                    }
                                }
                            }
                            else {
                                Ability & ab = *player.primingAbility;
                                ab.cooldownTimer = ab.cooldown;
                                ab.sprite = sprWarriorCharge;

                                player.primingAbility = nullptr;
                                player.currentAbility = &ab;
                                player.state = PLAYER_STATE_ATTACKING;

                                glm::vec2 dir = glm::normalize( mousePosWorld - ent->pos );
                                ent->vel += dir * 2500.0f;
                            }
                        }

                        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
                            if( player.primingAbility == nullptr ) {
                                Ability & ab = ent->playerStuff.abilities[ 1 ];
                                if( ab.cooldownTimer == 0.0f ) {
                                    // @HACK(SETTINGS):
                                    ab.cooldown = 1.0f;
                                    ab.type = ABILITY_TYPE_WARRIOR_STAB;

                                    ab.cooldownTimer = ab.cooldown;
                                    ab.sprite = sprWarriorStab;
                                    player.currentAbility = &ab;
                                    player.state = PLAYER_STATE_ATTACKING;
                                    AudioResource * playedSound = core->AudioPlayRandom( 1.0f, false, sndWarriorStab1, sndWarriorStab2 );

                                    if( isMp ) {
                                        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                                        msg.type = NetworkMessageType::ENTITY_AUDIO_PLAY;
                                        NetworkMessagePush( msg, playedSound->name );
                                        core->NetworkSend( msg );
                                    }
                                }
                            }
                            else {
                                player.primingAbility = nullptr;
                            }
                        }

                        if( core->InputKeyDown( KeyCode::KEY_CODE_F ) == true ) {
                            Ability & ab = ent->playerStuff.abilities[ 2 ];
                            if( ab.cooldownTimer == 0.0f ) {
                                // @HACK(SETTINGS):
                                ab.cooldown = 6.0f;
                                ab.type = ABILITY_TYPE_WARRIOR_CHARGE;
                                ab.stopsMovement = true;
                                player.primingAbility = &ab;

                                //ab.cooldownTimer = ab.cooldown;
                                //ab.sprite = sprWarriorCharge;
                                //player.currentAbility = &ab;
                                //player.state = PLAYER_STATE_ATTACKING;
                                //
                                //glm::vec2 dir = glm::normalize( mousePosWorld - ent->pos );
                                //ent->vel += dir * 2500.0f;
                                //core->AudioPlayRandom( 1.0f, false, sndWarriorCharge1, sndWarriorCharge2 );
                            }
                        }

                        bool stateDone = false;
                        while( stateDone == false ) {
                            stateDone = true;
                            switch( player.state ) {
                                case PLAYER_STATE_IDLE:
                                {
                                    ent->spriteAnimator.SetSpriteIfDifferent( core, sprWarriorIdle, true );
                                    ent->spriteAnimator.SetFrameRate( 10 );
                                    if( playerVel > 50.0f ) {
                                        player.state = PLAYER_STATE_MOVING;
                                    }
                                } break;
                                case PLAYER_STATE_MOVING:
                                {
                                    ent->spriteAnimator.SetSpriteIfDifferent( core, sprWarriorRun, true );
                                    ent->spriteAnimator.SetFrameRate( 10 );
                                    if( playerVel <= 50.0f ) {
                                        player.state = PLAYER_STATE_IDLE;
                                    }
                                } break;
                                case PLAYER_STATE_ATTACKING:
                                {
                                    if( player.currentAbility != NULL ) {
                                        ent->spriteAnimator.SetSpriteIfDifferent( core, player.currentAbility->sprite, false );

                                        i32 appliedDamged = 0;
                                        BoxBounds2D bb = {};
                                        switch( player.currentAbility->type ) {
                                            case ABILITY_TYPE_WARRIOR_STRIKE:
                                            {
                                                if( ent->spriteAnimator.frameIndex == 2 || ent->spriteAnimator.frameIndex == 3 ) {
                                                    glm::vec2 t1 = ent->pos + glm::vec2( ent->facingDir * 5, -20 ); // bl
                                                    glm::vec2 t2 = ent->pos + glm::vec2( ent->facingDir * 35, 30 ); // tr
                                                    glm::vec2 bl = glm::min( t1, t2 );
                                                    glm::vec2 tr = glm::max( t1, t2 );
                                                    bb.min = bl;
                                                    bb.max = tr;
                                                    appliedDamged = 70;
                                                }
                                            } break;
                                            case ABILITY_TYPE_WARRIOR_STAB:
                                            {
                                                if( ent->spriteAnimator.frameIndex == 2 || ent->spriteAnimator.frameIndex == 3 ) {
                                                    glm::vec2 t1 = ent->pos + glm::vec2( ent->facingDir * 0, -10 ); // bl
                                                    glm::vec2 t2 = ent->pos + glm::vec2( ent->facingDir * 40, 5 ); // tr
                                                    glm::vec2 bl = glm::min( t1, t2 );
                                                    glm::vec2 tr = glm::max( t1, t2 );
                                                    bb.min = bl;
                                                    bb.max = tr;
                                                    appliedDamged = 100;
                                                }
                                            } break;
                                            case ABILITY_TYPE_WARRIOR_CHARGE:
                                            {
                                                glm::vec2 t1 = ent->pos + glm::vec2( ent->facingDir * -8, -20 ); // bl
                                                glm::vec2 t2 = ent->pos + glm::vec2( ent->facingDir * 20, 20 ); // tr
                                                glm::vec2 bl = glm::min( t1, t2 );
                                                glm::vec2 tr = glm::max( t1, t2 );
                                                bb.min = bl;
                                                bb.max = tr;
                                                appliedDamged = 120;
                                            } break;
                                        }

                                        if( bb.min != bb.max ) {
                                            for( i32 entityIndexB = 0; entityIndexB < entityCount; entityIndexB++ ) {
                                                if( entityIndexB == entityIndexA ) {
                                                    continue;
                                                }

                                                Entity * enemy = entities[ entityIndexB ];
                                                switch( enemy->type ) {
                                                    case ENTITY_TYPE_ENEMY_DRONE_01:
                                                    {
                                                        if( player.currentAbility->hits.Contains( enemy->handle ) == false ) {
                                                            Collider2D c = enemy->GetWorldCollisionCollider();
                                                            if( c.Intersects( bb ) == true ) {
                                                                player.currentAbility->hits.Add( enemy->handle );

                                                                if( enemy->currentHealth - appliedDamged <= 0 ) {
                                                                    enemy->Unit_Die( core, isMp );
                                                                }
                                                                else {
                                                                    enemy->Unit_TakeDamage( core, isMp, appliedDamged );
                                                                }
                                                            }
                                                        }
                                                    } break;
                                                }
                                            }
                                            //debugDrawContext->DrawRect( bb.min, bb.max, glm::vec4( 0.8f, 0.2f, 0.2f, 0.5f ) );
                                        }

                                        if( ent->spriteAnimator.loopCount >= 1 ) {
                                            stateDone = false;
                                            ent->playerStuff.currentAbility->hits.Clear();
                                            ent->playerStuff.currentAbility = NULL;
                                            if( playerVel > 50.0f ) {
                                                player.state = PLAYER_STATE_MOVING;
                                            }
                                            else {
                                                player.state = PLAYER_STATE_IDLE;
                                            }
                                        }
                                    }
                                } break;
                            }
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
                case ENTITY_TYPE_ENEMY_DRONE_01: {
                    UnitStuff & unit = ent->unitStuff;
                    Navigator & nav = ent->navigator;

                    if( ent->spriteAnimator.sprite == nullptr ) {
                        ent->spriteAnimator.SetSpriteIfDifferent( core, sprCharDrone, false );
                    }

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
                                core->AudioPlayRandom( 1.0f, false, sndCloseExplody1, sndCloseExplody2 );
                            }

                            if( ent->spriteAnimator.loopCount > 0 ) {
                                ent->active = false; // Trick to hide the entity in laggy mp enviroments. We need to wait for the destroy entity rpc...

                                if( isMp == true && hasAuthority == true ) {
                                    entityPool.Remove( ent->handle );

                                    NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                                    msg.type = NetworkMessageType::ENTITY_DESTROY;
                                    NetworkMessagePush( msg, ent->handle );
                                    core->NetworkSend( msg );
                                }
                            }
                        } break;
                        case UNIT_STATE_WANDERING:
                        {
                            if( hasAuthority == false ) {
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

                                glm::vec2 acc = glm::normalize( nav.dest - ent->pos ) * speed;

                                acc.x -= ent->vel.x * resistance;
                                acc.y -= ent->vel.y * resistance;

                                ent->vel += acc * dt;
                            }

                            ent->pos += ent->vel * dt;
                        } break;
                        case UNIT_STATE_SWARM:
                        {
                            if( hasAuthority == false ) {
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

                                glm::vec2 acc = glm::normalize( nav.dest - ent->pos ) * speed;

                                acc.x -= ent->vel.x * resistance;
                                acc.y -= ent->vel.y * resistance;

                                ent->vel += acc * dt;
                            }

                            ent->pos += ent->vel * dt;

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

                    if( isMp == true && hasAuthority == true ) {
                        NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                        msg.type = NetworkMessageType::ENTITY_POS_UPDATE;
                        NetworkMessagePush( msg, ent->handle );
                        NetworkMessagePush( msg, ent->pos );
                        NetworkMessagePush( msg, ent->vel );
                        core->NetworkSend( msg );
                    }
                    
                    // Debug
                    spriteDrawContext->DrawRect( ent->pos, glm::vec2( alertRad ), 0.0f, glm::vec4( 0.2f, 0.2f, 0.75f, 0.76f ) );

                } break;
            }

            glm::vec2 drawPos = ent->netStreamed ? ent->netVisualPos : ent->pos;
            spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, ent->ori, glm::vec2( ent->facingDir, 1.0f ), colorMultiplier );
        }

        if ( false )
        {
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

        SmallString s = StringFormat::Small( "d=%d", (i32)core->NetworkGetPing() );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );

        core->RenderSubmit( spriteDrawContext, true );
        core->RenderSubmit( debugDrawContext, false );


        //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, "Hello World" );
        //spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, startParms.localPlayerNumber == 1 ? "1" : "2" );
    }

    Entity * Map::SpawnEntity( EntityType type ) {
        EntityHandle handle = {};
        Entity * entity = entityPool.Add( handle );
        AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
        if( entity != nullptr ) {
            ZeroStructPtr( entity );
            entity->active = true;
            entity->handle = handle;
            entity->type = type;
            entity->facingDir = 1.0f;
        }

        return entity;
    }


    Entity * Map::SpawnDrone( glm::vec2 pos ) {
        Entity * entity = SpawnEntity( ENTITY_TYPE_ENEMY_DRONE_01 );
        if( entity != nullptr ) {
            entity->pos = pos;
            entity->netStreamed = !hasAuthority;
            entity->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
            entity->selectionCollider.circle.pos = glm::vec2( -0.5f, -0.5f );
            entity->selectionCollider.circle.rad = 5.0f;
            entity->collisionCollider = entity->selectionCollider;
            entity->maxHealth = 100;
            entity->currentHealth = entity->maxHealth;
        }

        return entity;
    }

    void Map::EntityUpdatePlayer( Core * core, Entity * ent, EntList & activeEnts ) {
        
    }

    Entity * Map::ClosestPlayerTo( glm::vec2 p, f32 & dist ) {
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
            }
            default:
            {
                INVALID_CODE_PATH;
            } break;
        }

        return c;
    }


    void Entity::Unit_TakeDamage( Core * core, bool sendPacket, i32 damage ) {
        currentHealth -= damage;

        unitStuff.state = UNIT_STATE_TAKING_DAMAGE;
        unitStuff.takingDamageTimer = 0.1f;

        ZeroStruct( particleSystem );
        particleSystem.count = 10;
        particleSystem.texture = core->ResourceGetAndLoadTexture( "particle_single_white_1x1.png", false, false );;
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

    void Entity::Unit_Die( Core * core, bool sendPacket ) {
        currentHealth = 0;
        unitStuff.state = UNIT_STATE_EXPLODING;
        spriteAnimator.frameDelaySkip = Random::Int( 3 );

        if( sendPacket == true ) {
            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
            msg.type = NetworkMessageType::ENTITY_RPC_UNIT_DIE;
            NetworkMessagePush( msg, handle );
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

    void SpriteAnimator::SetSpriteIfDifferent( Core * core, SpriteResource * sprite, bool loops ) {
        if( this->sprite != sprite ) {
            this->sprite = sprite;
            SetFrameRate( (f32)sprite->frameRate );
            frameIndex = 0;
            frameTimer = 0;
            loopCount = 0;
            this->loops = loops;
            TestFrameActuations( core );
        }
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

}
