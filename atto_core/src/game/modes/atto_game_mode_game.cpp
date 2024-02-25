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
        map.Start( core );
    }

    void GameMode_Game::UpdateAndRender( Core * core, f32 dt, UpdateAndRenderFlags flags ) {
        map.UpdateAndRender( core, dt, flags );
    }

    void GameMode_Game::Shutdown( Core * core ) {
    }

    void Map::Start( Core * core ) {
        localPlayer = SpawnEntity( ENTITY_TYPE_PLAYER );
        localPlayer->spriteAnimator.sprite = core->ResourceGetAndCreateSprite( "temp", "asset_pack_01/player_idle/player_idle.png", 10, 48, 48 );
        
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

        static SpriteResource * sprWarriorIdle = core->ResourceGetAndCreateSprite( "idle", "asset_pack_01/player_idle/player_idle.png", 10, 48, 48 );
        static SpriteResource * sprWarriorRun = core->ResourceGetAndCreateSprite( "run", "asset_pack_01/player_run/player_run.png", 8, 48, 48 );
        static SpriteResource * sprWarriorStab = core->ResourceGetAndCreateSprite( "stab", "asset_pack_01/player_sword_stab/player_sword_stab.png", 7, 96, 48 );
        static SpriteResource * sprWarriorStrike = core->ResourceGetAndCreateSprite( "strike", "asset_pack_01/basic_sword_attack/basic_sword_attack.png", 6, 64, 64 );
        static SpriteResource * sprWarriorCharge = core->ResourceGetAndCreateSprite( "charge", "asset_pack_01/player_katana_continuous_attack/player_katana_continuous_attack.png", 9, 80, 64 );

        static SpriteResource * sprCharDrone = core->ResourceGetAndCreateSprite( "drone", "char_drone_01.png", 1, 32, 32 );
        static SpriteResource * sprVFX_SmallExplody= core->ResourceGetAndCreateSprite( "vfx_small_explody", "vfx_small_explody.png", 3, 32, 32 );

        static AudioResource * sndWarriorStrike1 = core->ResourceGetAndLoadAudio( "not_legal/lightsaber_quick_1.wav" );
        static AudioResource * sndWarriorStrike2 = core->ResourceGetAndLoadAudio( "not_legal/lightsaber_quick_3.wav" );
        static AudioResource * sndWarriorStab1 = core->ResourceGetAndLoadAudio( "not_legal/lightsaber_quick_2.wav" );
        static AudioResource * sndWarriorStab2 = core->ResourceGetAndLoadAudio( "not_legal/lightsaber_quick_4.wav" );
        static AudioResource * sndWarriorCharge1 = core->ResourceGetAndLoadAudio( "not_legal/lightsaber_clash_1.wav" );
        static AudioResource * sndWarriorCharge2 = core->ResourceGetAndLoadAudio( "not_legal/lightsaber_clash_2.wav" );

        static FontHandle fontHandle = core->ResourceGetFont( "default" );
        
        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0 );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 1 );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 2 );

        EntList & entities = * core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

        spriteDrawContext->SetCameraPos( glm::vec2( 0, 0 ) );
        for( int y = 0; y < 10; y++ ) {
            for( int x = 0; x < 10; x++ ) {
                spriteDrawContext->DrawTexture( tile, glm::vec2( x * 32, y * 32 ) );
            }
        }

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            Entity * ent = entities[ entityIndexA ];
            switch( ent->type ) {
                case ENTITY_TYPE_PLAYER: {
                    const f32 tickTime = 0.016f; // 60z
                    const i32 maxTickIterations = 3;

                    PlayerStuff & player = ent->playerStuff;

                    static f32 dtAccumulator = 0.0f;
                    dtAccumulator += dt;
                    i32 tickIteration = 0;
                    while( dtAccumulator >= tickTime && tickIteration < maxTickIterations ) {
                        dtAccumulator -= tickTime;
                        tickIteration++;

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

                        ent->vel += acc * tickTime;
                        ent->pos += ent->vel * tickTime;

                        for( int abilityIndex = 0; abilityIndex < MAX_ABILITIES; abilityIndex++ ) {
                            Ability & ab = ent->playerStuff.abilities[ abilityIndex ];
                            ab.cooldownTimer -= tickTime;
                            if( ab.cooldownTimer < 0.0f ) {
                                ab.cooldownTimer = 0.0f;
                            }
                        }
                    }

                    const f32 spriteFrameRate = 10.0f;
                    ent->spriteAnimator.frameTimer += dt;
                    if( ent->spriteAnimator.frameTimer >= ent->spriteAnimator.frameDuration ) {
                        ent->spriteAnimator.frameTimer -= ent->spriteAnimator.frameDuration;
                        ent->spriteAnimator.frameIndex++;
                        if( ent->spriteAnimator.frameIndex >= ent->spriteAnimator.sprite->frameCount ) {
                            ent->spriteAnimator.frameIndex = 0;
                            ent->spriteAnimator.loopCount++;
                        }
                    }

                    const f32 playerVel = glm::length( ent->vel );
                    if( playerVel > 50.0f ) {
                        ent->facingDir = glm::sign( ent->vel.x );
                    }

                    if( ent->facingDir == 0.0 ) {
                        ent->facingDir = 1.0f;
                    }

                    if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                        Entity * targetEnt = entityPool.Get( ent->targetEntity );
                        Ability & ab = ent->playerStuff.abilities[ 0 ];
                        if( ab.cooldownTimer == 0.0f ) {
                            ab.cooldown = 0.5f;
                            ab.type = ABILITY_TYPE_WARRIOR_STRIKE;

                            ab.cooldownTimer = ab.cooldown;
                            ab.sprite = sprWarriorStrike;
                            player.state = PLAYER_STATE_ATTACKING;
                            player.currentAbility = &ab;
                            core->AudioPlayRandom( 1.0f, false, sndWarriorStrike1, sndWarriorStrike2 );
                        }
                    }

                    if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
                        Entity * targetEnt = entityPool.Get( ent->targetEntity );
                        Ability & ab = ent->playerStuff.abilities[ 1 ];
                        if( ab.cooldownTimer == 0.0f ) {
                            ab.cooldown = 1.0f;
                            ab.type = ABILITY_TYPE_WARRIOR_STAB;

                            ab.cooldownTimer = ab.cooldown;
                            ab.sprite = sprWarriorStab;
                            player.currentAbility = &ab;
                            player.state = PLAYER_STATE_ATTACKING;
                            core->AudioPlayRandom( 1.0f, false, sndWarriorStab1, sndWarriorStab2 );
                        }
                    }

                    if( core->InputKeyDown( KeyCode::KEY_CODE_F ) == true ) {
                        Ability & ab = ent->playerStuff.abilities[ 2 ];
                        if( ab.cooldownTimer == 0.0f ) {
                            ab.cooldown = 6.0f;
                            ab.type = ABILITY_TYPE_WARRIOR_CHARGE;
                            ab.stopsMovement = true;

                            ab.cooldownTimer = ab.cooldown;
                            ab.sprite = sprWarriorCharge;
                            player.currentAbility = &ab;
                            player.state = PLAYER_STATE_ATTACKING;
                            
                            glm::vec2 dir = glm::normalize( mousePosWorld - ent->pos );
                            ent->vel += dir * 2500.0f;

                            core->AudioPlayRandom( 1.0f, false, sndWarriorCharge1, sndWarriorCharge2 );
                        }
                    }

                    bool stateDone = false;
                    while( stateDone == false ) {
                        stateDone = true;

                        switch( player.state ) {
                            case PLAYER_STATE_IDLE:
                            {
                                ent->spriteAnimator.SetSpriteIfDifferent( sprWarriorIdle );
                                ent->spriteAnimator.SetFrameRate( 10 );
                                if( playerVel > 50.0f ) {
                                    player.state = PLAYER_STATE_MOVING;
                                }
                            } break;
                            case PLAYER_STATE_MOVING:
                            {
                                ent->spriteAnimator.SetSpriteIfDifferent( sprWarriorRun );
                                ent->spriteAnimator.SetFrameRate( 10 );
                                if( playerVel <= 50.0f ) {
                                    player.state = PLAYER_STATE_IDLE;
                                }
                            } break;
                            case PLAYER_STATE_ATTACKING:
                            {
                                if( player.currentAbility != NULL ) {
                                    ent->spriteAnimator.SetSpriteIfDifferent( player.currentAbility->sprite );
                                    ent->spriteAnimator.SetFrameRate( 16 );

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

                                                            //enemy->unitStuff.state = UNIT_STATE_TAKING_DAMAGE;
                                                            enemy->unitStuff.state = UNIT_STATE_EXPLODING;
                                                            enemy->unitStuff.takingDamageTimer = 0.1f;

                                                            ZeroStruct( enemy->particleSystem );
                                                            enemy->particleSystem.count = 10;
                                                            enemy->particleSystem.texture = sprParticleSingleWhite;
                                                            enemy->particleSystem.lifeTime = 0.5f;
                                                            enemy->particleSystem.scaleMin = 1;
                                                            enemy->particleSystem.scaleMax = 2;
                                                            enemy->particleSystem.velMin = glm::vec2( -100.0f );
                                                            enemy->particleSystem.velMax = glm::vec2( 100.0f );
                                                            enemy->particleSystem.oneShot = true;

                                                            ParticleSystem & part = enemy->particleSystem;
                                                            enemy->particleSystem.emitting = true;
                                                            for( i32 partIndex = 0; partIndex < enemy->particleSystem.count; partIndex++ ) {
                                                                Particle & p = part.particles[ partIndex ];
                                                                p.pos = enemy->pos;
                                                                p.lifeTime = part.lifeTime;
                                                                p.scale = Random::Float( part.scaleMin, part.scaleMax );
                                                                p.vel = Random::Vec2( part.velMin, part.velMax );
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

                    spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, ent->pos, ent->ori, glm::vec2( ent->facingDir, 1.0f ) );

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
                case ENTITY_TYPE_ENEMY_DRONE_01:
                {
                    UnitStuff & unit = ent->unitStuff;

                    glm::vec4 colorMultiplier = glm::vec4( 1, 1, 1, 1 );

                    if( ent->spriteAnimator.sprite == nullptr ) {
                        ent->spriteAnimator.SetSpriteIfDifferent( sprCharDrone );
                    }

                    ent->spriteAnimator.SetFrameRate( 7 );
                    if( ent->spriteAnimator.sprite != nullptr && ent->spriteAnimator.sprite->frameCount > 1 ) {
                        ent->spriteAnimator.frameTimer += dt;
                        if( ent->spriteAnimator.frameTimer >= ent->spriteAnimator.frameDuration ) {
                            ent->spriteAnimator.frameTimer -= ent->spriteAnimator.frameDuration;
                            ent->spriteAnimator.frameIndex++;
                            if( ent->spriteAnimator.frameIndex >= ent->spriteAnimator.sprite->frameCount ) {
                                ent->spriteAnimator.frameIndex = 0;
                                ent->spriteAnimator.loopCount++;
                            }
                        }
                    }

                    const f32 entVel = glm::length( ent->vel );
                    if( entVel > 50.0f ) {
                        ent->facingDir = glm::sign( ent->vel.x );
                    }

                    if( ent->facingDir == 0.0 ) {
                        ent->facingDir = 1.0f;
                    }

                    switch( unit.state ) {
                        case UNIT_STATE_TAKING_DAMAGE:
                        {
                            unit.takingDamageTimer -= dt;
                            if( unit.takingDamageTimer <= 0.0f ) {
                                unit.takingDamageTimer = 0.0f;
                                unit.state = UNIT_STATE_IDLE;
                            }
                            else {
                                colorMultiplier = glm::vec4( 100, 100, 100, 1 );
                            }

                        } break;

                        case UNIT_STATE_EXPLODING:
                        {
                            ent->spriteAnimator.SetSpriteIfDifferent( sprVFX_SmallExplody );
                            if( ent->spriteAnimator.loopCount > 0 ) {
                                entityPool.Remove( ent->handle );
                            }
                        } break;
                    }

                    spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, ent->pos, ent->ori, glm::vec2( ent->facingDir, 1.0f ) );

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


                } break;
            }
        }

        for( int i = 0; i < 12; i++ ) {
            spriteDrawContext->DrawTexture( sprUiPanel, glm::vec2( 50 + i * 19, 10 ) );
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
            entity->handle = handle;
            entity->type = type;
        }

        return entity;
    }


    Entity * Map::SpawnDrone( glm::vec2 pos ) {
        Entity * entity = SpawnEntity( ENTITY_TYPE_ENEMY_DRONE_01 );
        if( entity != nullptr ) {
            entity->pos = pos;

            entity->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
            entity->selectionCollider.circle.pos = glm::vec2( -0.5f, -0.5f );
            entity->selectionCollider.circle.rad = 5.0f;
            entity->collisionCollider = entity->selectionCollider;
        }

        return entity;
    }

    void Map::EntityUpdatePlayer( Core * core, Entity * ent, EntList & activeEnts ) {
        
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

    Collider2D Entity::GetWorldCollisionCollider() const {
        return ColliderForSpace( collisionCollider, pos );
    }

    Collider2D Entity::GetWorldSelectionCollider() const {
        return ColliderForSpace( selectionCollider, pos );
    }

}
