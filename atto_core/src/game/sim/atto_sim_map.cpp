#include "atto_sim_map.h"
#include "../../shared/atto_core.h"
#include "../atto_map_communicator.h"

#include "../../shared/atto_rpc.h"

namespace atto {
    void SimMap::Initialize( Core * core ) {
        if( rpcTable[ 1 ] == nullptr ) {
            rpcTable[ (i32)MapActionType::PLAYER_SELECTION ]   = new RpcMemberFunction( this, &SimMap::SimAction_Select );
            rpcTable[ (i32)MapActionType::PLAYER_MOVE ]        = new RpcMemberFunction( this, &SimMap::SimAction_Move );
            rpcTable[ (i32)MapActionType::PLAYER_ATTACK ]      = new RpcMemberFunction( this, &SimMap::SimAction_Attack );

            rpcTable[ (i32)MapActionType::SIM_ENTITY_SPAWN ]   = new RpcMemberFunction( this, &SimMap::SimAction_SpawnEntity );
        }

        this->core = core;
        syncQueues.Start();
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), 1, 1, glm::vec2( 100.0f ), glm::vec2( 0.0f ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), 2, 2, glm::vec2( 500.0f ), glm::vec2( 0.0f ) );
    }

    void SimMap::Update( Core * core, f32 dt ) {
        if( core->NetworkIsConnected() == true ) {
            NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
            while( core->NetworkRecieve( msg ) ) {
                switch( msg.type ) {
                    case NetworkMessageType::MAP_TURN:
                    {
                        i32 offset = 0;
                        MapTurn turn = NetworkMessagePop<MapTurn>( msg, offset );
                        syncQueues.AddTurn( turn.playerNumber, turn );

                        //core->LogOutput( LogLevel::INFO, "Received turn %d ", turn.turnNumber );
                    } break;
                }
            }

            dtAccumulator += dt;
            if( dtAccumulator > TURNS_DELTA ) {
                dtAccumulator -= TURNS_DELTA;
                if( syncQueues.CanTurn() == false ) {
                    syncTurnAttempts++;

                    if( syncTurnAttempts >= syncQueues.GetSlidingWindowWidth() * 4 ) {
                        syncWaitTurnCounter = syncQueues.GetSlidingWindowWidth() * 2;
                        syncTurnAttempts = 0;
                    }

                    core->LogOutput( LogLevel::WARN, "Can't turn, going to wait" );
                    return;
                }

                if( syncWaitTurnCounter > 0 ) {
                    syncWaitTurnCounter--;
                    core->LogOutput( LogLevel::WARN, "Waiting for %d turns", syncWaitTurnCounter );
                    return;
                }

                syncTurnAttempts = 0;

                MapTurn * playerOneTurn = syncQueues.GetNextTurn( 1 );
                MapTurn * playerTwoTurn = syncQueues.GetNextTurn( 2 );

                SimTick( playerOneTurn, playerTwoTurn );

                syncQueues.FinishTurn();

                localMapTurn.playerNumber = localPlayerNumber;
                localMapTurn.turnNumber = turnNumber + syncQueues.GetSlidingWindowWidth();
                localMapTurn.checkSum = 0;
                localMapTurn.actions = localActionBuffer; // TODO: Use a memcpy ?

                syncQueues.AddTurn( localPlayerNumber, localMapTurn );

                ZeroStruct( msg );
                msg.type = NetworkMessageType::MAP_TURN;
                NetworkMessagePush( msg, localMapTurn );
                core->NetworkSend( msg );
                //core->LogOutput( LogLevel::INFO, "Sending turn %d", localMapTurn.turnNumber );

                ZeroStruct( localMapTurn );
                ZeroStruct( localActionBuffer );
            }
        }
        else {
            dtAccumulator += dt;
            if( dtAccumulator > TURNS_DELTA ) {
                dtAccumulator -= TURNS_DELTA;

                localMapTurn.playerNumber = localPlayerNumber;
                localMapTurn.turnNumber = turnNumber;
                localMapTurn.checkSum = 0;
                localMapTurn.actions = localActionBuffer; // TODO: Use a memcpy ?

                SimTick( &localMapTurn, nullptr );

                ZeroStruct( localMapTurn );
                ZeroStruct( localActionBuffer );
            }
        }

        static FontHandle fontHandle = core->ResourceGetFont( "default" );
        static TextureResource * background = core->ResourceGetAndLoadTexture( "res/maps/dark_blue.png" );
        static TextureResource * sprTurretSmol = core->ResourceGetAndLoadTexture( "res/ents/test/turret_smol.png" );
        static TextureResource * sprTurretMed = core->ResourceGetAndLoadTexture( "res/ents/test/turret_med.png" );

        const f32 cameraSpeed = 20.0f;
        if( core->InputKeyDown( KEY_CODE_W ) == true ) {
            localCameraPos.y += cameraSpeed;
        }
        if( core->InputKeyDown( KEY_CODE_S ) == true ) {
            localCameraPos.y -= cameraSpeed;
        }
        if( core->InputKeyDown( KEY_CODE_A ) == true ) {
            localCameraPos.x -= cameraSpeed;
        }
        if( core->InputKeyDown( KEY_CODE_D ) == true ) {
            localCameraPos.x += cameraSpeed;
        }

        f32 zoomDelta = core->InputMouseWheelDelta();

        // 16 : 9 resolutions
        static glm::vec2 resolutions[] = {
            glm::vec2( 640, 360 ),
            glm::vec2( 960, 540 ),
            glm::vec2( 1280, 720 ),
            glm::vec2( 1600, 900 ),
            glm::vec2( 1920, 1080 ),
        };

        if( zoomDelta > 0 ) {
            localCameraZoomIndex--;
        }
        else if ( zoomDelta < 0 ) {
            localCameraZoomIndex++;
        }

        localCameraZoomIndex = glm::clamp( localCameraZoomIndex, 0, 4 );

        glm::vec2 wantedResolution = resolutions[ localCameraZoomIndex ];
        glm::vec2 oldCameraWidth = localCameraZoomLerp;
        localCameraZoomLerp = glm::mix( localCameraZoomLerp, wantedResolution, dt * 10.f );
        if( glm::abs( localCameraZoomLerp.x - wantedResolution.x ) < 0.1f ) {
            localCameraZoomLerp = wantedResolution;
        }

        localCameraPos -= ( localCameraZoomLerp - oldCameraWidth ) * 0.5f;

        core->RenderSetCameraDims( localCameraZoomLerp.x, localCameraZoomLerp.y );

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, true );
        DrawContext * backgroundDrawContext = core->RenderGetDrawContext( 1, true );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 2, true );
        
        const glm::vec2 mapMin = glm::vec2( 0.0f );
        const glm::vec2 mapMax = glm::vec2( 3000.0f ) - glm::vec2( spriteDrawContext->GetCameraWidth(), spriteDrawContext->GetCameraHeight() );
        localCameraPos = glm::clamp( localCameraPos, mapMin, mapMax );

        spriteDrawContext->SetCameraPos( localCameraPos );
        debugDrawContext->SetCameraPos( localCameraPos );

        spriteDrawContext->DrawTextureBL( background, glm::vec2( 0, 0 ) );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );
        const double currentTime = core->GetTheCurrentTime();

        EntList & entities = *core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            SimEntity * ent = entities[ entityIndexA ];

            ent->visPos = glm::mix( ent->visPos, ent->pos, dt * 7.0f );
            ent->visOri = glm::mix( ent->visOri, ent->ori, dt * 7.0f );

            glm::vec2 drawPos = ent->visPos;
            f32 drawOri = ent->visOri;

            if( ent->selectedBy.Contains( localPlayerNumber ) && ent->selectionAnimator.sprite != nullptr ) {
                //spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, ent->pos, ent->ori );
                spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, drawPos, drawOri );
            }

            if( ent->spriteAnimator.sprite != nullptr ) {
                spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, drawOri );
            }

            if( IsUnitType( ent->type ) ) {
                const Unit & unit = ent->unit;
                const i32 turretCount = unit.turrets.GetCount();
                for( i32 turretIndex = 0; turretIndex < turretCount; turretIndex++ ) {
                    UnitTurret & turret = unit.turrets[ turretIndex ];
                    glm::vec2 worldPos = ent->visPos + glm::rotate( turret.posOffset, -ent->visOri );

                    if( turret.size == WeaponSize::SMALL ) {
                        spriteDrawContext->DrawTexture( sprTurretSmol, worldPos, turret.ori );
                    }
                    else if( turret.size == WeaponSize::MEDIUM ) {
                        spriteDrawContext->DrawTexture( sprTurretMed, worldPos, turret.ori );
                    }
                    //glm::vec2 worldPos = ;

                }
            }

            if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                Collider2D selectionCollider = ent->GetWorldSelectionCollider();
                if( selectionCollider.Contains( mousePosWorld ) ) {
                    EntHandleList & selection = *core->MemoryAllocateTransient<EntHandleList>();
                    selection.Add( ent->handle );
                    localActionBuffer.AddAction( MapActionType::PLAYER_SELECTION, localPlayerNumber, selection, EntitySelectionChange::SET );
                }
            }
        }

        if( core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
            bool inputMade = false;
            for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                const SimEntity * ent = entities[ entityIndexA ];
                const Collider2D selectionCollider = ent->GetWorldSelectionCollider();
                if( selectionCollider.Contains( mousePosWorld ) ) {
                    if( ent->teamNumber != localPlayerTeamNumber ) {
                        localActionBuffer.AddAction( MapActionType::PLAYER_ATTACK, localPlayerNumber, ent->handle );
                    }
                    else {
                     // Follow
                    }

                    inputMade = true;
                    break;
                }
            }

            if( inputMade == false ) {
                localActionBuffer.AddAction( MapActionType::PLAYER_MOVE, localPlayerNumber, mousePosWorld );
            }
        }
        
        SmallString s = StringFormat::Small( "ping=%d", (i32)core->NetworkGetPing() );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );
        s = StringFormat::Small( "dt=%f", dt );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 160 ), 32, s.GetCStr() );
        s = StringFormat::Small( "fps=%f", 1.0f / dt );
        spriteDrawContext->DrawText2D( fontHandle, glm::vec2( 128, 200 ), 32, s.GetCStr() );

        core->RenderSubmit( spriteDrawContext, true );
        core->RenderSubmit( backgroundDrawContext, false );
        core->RenderSubmit( debugDrawContext, false );
    }

    SimEntity * SimMap::SpawnEntity( EntityType type, i32 playerNumber, i32 teamNumber, glm::vec2 pos, glm::vec2 vel ) {
        EntityHandle handle = {};
        SimEntity * entity = entityPool.Add( handle );
        AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
        if( entity != nullptr ) {
            ZeroStructPtr( entity );
            entity->handle = handle;

            entity->active = true;
            entity->type = type;
            entity->resistance = 14.0f;
            entity->pos = pos;
            entity->visPos = pos;
            entity->vel = vel;
            entity->teamNumber = teamNumber;
            entity->playerNumber = playerNumber;

            switch( entity->type ) {
                case EntityType::UNIT_TEST:
                {
                    entity->selectionCollider.type = COLLIDER_TYPE_BOX;
                    entity->selectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 36 ) );
                    entity->unit.averageFiringRange = 400.0f;

                    static SpriteResource * mainSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship.png", 1, 48, 48, 0 );
                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
                    entity->spriteAnimator.SetSpriteIfDifferent( core, mainSprite, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( core, selectionSprite, false );

                    UnitTurret turret1 = {};
                    turret1.size = WeaponSize::SMALL;
                    turret1.posOffset = glm::vec2( -4, 7 );
                    turret1.fireRate = 0.5f;
                    turret1.fireRange = 400.0f;
                    turret1.fireDamage = 1.0f;
                    entity->unit.turrets.Add( turret1 );

                    UnitTurret turret2 = {};
                    turret2.size = WeaponSize::SMALL;
                    turret2.posOffset = glm::vec2( 4, 7 );
                    turret2.fireRate = 0.5f;
                    turret2.fireRange = 400.0f;
                    turret2.fireDamage = 1.0f;
                    entity->unit.turrets.Add( turret2 );

                    UnitTurret turret3 = {};
                    turret3.size = WeaponSize::MEDIUM;
                    turret3.posOffset = glm::vec2( -7, -5 );
                    turret3.fireRate = 0.5f;
                    turret3.fireRange = 400.0f;
                    turret3.fireDamage = 1.0f;
                    entity->unit.turrets.Add( turret3 );

                    UnitTurret turret4 = {};
                    turret4.size = WeaponSize::MEDIUM;
                    turret4.posOffset = glm::vec2( 7, -5 );
                    turret4.fireRate = 0.5f;
                    turret4.fireRange = 400.0f;
                    turret4.fireDamage = 1.0f;
                    entity->unit.turrets.Add( turret4 );
                } break;
            }
        }

        return entity;
    }

    void SimMap::SimAction_SpawnEntity( i32 * typePtr, i32 * playerNumberPtr, i32 * teamNumberPtr, glm::vec2 * posPtr, glm::vec2 * velPtr ) {
        EntityType type = EntityType::Make( (EntityType::_enumerated)( * typePtr) );
        i32 playerNumber = *playerNumberPtr;
        i32 teamNumber = *teamNumberPtr;
        glm::vec2 pos = *posPtr;
        glm::vec2 vel = *velPtr;
        SpawnEntity( type, playerNumber, teamNumber, pos, vel );
    }

    void SimMap::SimAction_Select( i32 * playerNumberPtr, EntHandleList * selection, EntitySelectionChange * changePtr ) {
        i32 playerNumber = *playerNumberPtr;
        EntitySelectionChange change = *changePtr;

        // @SPEED:
        activeEntities.Clear( false );
        entityPool.GatherActiveObjs( activeEntities );

        if( change == EntitySelectionChange::SET ) {
            const i32 entityCount = activeEntities.GetCount();
            for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = activeEntities[ entityIndex ];
                ent->selectedBy.RemoveValue( playerNumber );
            }
        }

        const i32 selectionCount = selection->GetCount();
        if( change == EntitySelectionChange::SET || change == EntitySelectionChange::ADD ) {
            for( i32 entityHandleIndex = 0; entityHandleIndex < selectionCount; entityHandleIndex++ ) {
                EntityHandle handle = *selection->Get( entityHandleIndex );
                SimEntity * ent = entityPool.Get( handle );
                if( ent != nullptr ) {
                    ent->selectedBy.Add( playerNumber );
                }
            }
        }

        if( change == EntitySelectionChange::REMOVE ) {
            for( i32 entityHandleIndex = 0; entityHandleIndex < selectionCount; entityHandleIndex++ ) {
                EntityHandle handle = *selection->Get( entityHandleIndex );
                SimEntity * ent = entityPool.Get( handle );
                if( ent != nullptr ) {
                    ent->selectedBy.RemoveValue( playerNumber );
                }
            }
        }
    }

    void SimMap::SimAction_Move( i32 * playerNumberPtr, glm::vec2 * posPtr ) {
        i32 playerNumber = *playerNumberPtr;
        glm::vec2 pos = *posPtr;

        // @SPEED:
        activeEntities.Clear( false );
        entityPool.GatherActiveObjs( activeEntities );

        const i32 entityCount = activeEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = activeEntities[ entityIndex ];
            if( ent->playerNumber == playerNumber && ent->selectedBy.Contains( playerNumber ) ) {
                ent->navigator.hasDest = true;
                ent->navigator.dest = pos;
                ent->navigator.slowRad = 100.0f;
            }
        }
    }

    void SimMap::SimAction_Attack( i32 * playerNumberPtr, EntityHandle * targetPtr ) {
        i32 playerNumber = *playerNumberPtr;
        EntityHandle target = *targetPtr;

        // @SPEED:
        activeEntities.Clear( false );
        entityPool.GatherActiveObjs( activeEntities );

        SimEntity * targetEnt = entityPool.Get( target );
        if( targetEnt == nullptr ) {
            return;
        }

        const i32 entityCount = activeEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = activeEntities[ entityIndex ];
            if( ent->playerNumber == playerNumber && ent->selectedBy.Contains( playerNumber ) ) {
                ent->navigator.hasDest = true;
                glm::vec2 d = ( ent->pos - targetEnt->pos );
                f32 dist = glm::length( d );

                 if( dist > ent->unit.averageFiringRange - 10.0f ) {
                     glm::vec2 dir = d / dist;
                     ent->navigator.dest = targetEnt->pos + dir * ent->unit.averageFiringRange - 10.0f; // @HACK: The 10 is because firing range is from center ent so move in more forward for cannons at the back
                 } else {
                     ent->navigator.dest = ent->pos;
                 }

                ent->navigator.slowRad = 100.0f;
            }
        }
    }

    static void SimTick_SelfUpdate( const ConstEntList * entities, i32 index, SimEntity * ent ) {
        const f32 playerSpeed = 2500.0f / 100.0f;
        const f32 maxForce = 20.0f;
        const f32 invMass = 1.0f / 10.0f;

        ZeroStruct( ent->actions );

        // Boid arrival
        if( ent->type == EntityType::UNIT_TEST ) {
            if( ent->navigator.hasDest == true ) {
                const glm::vec2 targetPos = ent->navigator.dest;
                glm::vec2 desiredVel = ( targetPos - ent->pos );
                f32 dist = glm::length( desiredVel );

                if( dist < ent->navigator.slowRad ) {
                    if( dist < 5 ) {
                        desiredVel = glm::vec2( 0.0f );
                    }
                    else {
                        desiredVel = glm::normalize( desiredVel ) * playerSpeed * ( dist / ent->navigator.slowRad );
                    }
                }
                else {
                    desiredVel = glm::normalize( desiredVel ) * playerSpeed;
                }

            #if 0
                if( dist > ent->navigator.slowRad ) {
                    constexpr f32 maxSteerAngle = glm::radians( 15.0f );

                    glm::vec2 current = ent->vel;
                    if( glm::length2( current ) < 1.0f ) {
                        current = glm::vec2( glm::sin( ent->ori ), glm::cos( ent->ori ) );
                    }

                    f32 fullSteers = glm::acos( glm::clamp( glm::dot( glm::normalize( desiredVel ), glm::normalize( current ) ), -1.0f, 1.0f ) ) / maxSteerAngle;
                    f32 angl = glm::clamp( glm::sign( glm::dot( LeftPerp( desiredVel ), current ) ) * fullSteers, -1.0f, 1.0f );
                    ent->ori += angl * maxSteerAngle;

                    glm::vec2 face = glm::vec2( glm::sin( ent->ori ), glm::cos( ent->ori ) );
                    desiredVel = face * playerSpeed;
                }
            #endif

                if( glm::length2( ent->vel ) >= 1.0f ) {
                    glm::vec2 nvel = glm::normalize( ent->vel );
                    ent->ori = atan2f( nvel.x, nvel.y );
                }


                glm::vec2 steering = desiredVel - ent->vel;

                steering = Truncate( steering, maxForce );
                steering *= invMass;

                ent->vel = Truncate( ent->vel + steering, playerSpeed );
            }

            /*
            f32 distToTarget = glm::length( desiredVel );
            if( distToTarget < ent->navigator.slowRad ) {
                ent->acc = glm::vec2( 0.0f );
                ent->vel = glm::normalize( toTarget ) * playerSpeed * ( distToTarget / ent->navigator.slowRad );
            }
            else {
                ent->acc += glm::normalize( toTarget ) * playerSpeed;
            }*/

            Unit & unit = ent->unit;
            const i32 turretCount = unit.turrets.GetCount();
            for( i32 turretIndex = 0; turretIndex < turretCount; turretIndex++ ) {
                UnitTurret & turret = unit.turrets[ turretIndex ];
                glm::vec2 worldPos = ent->pos + glm::rotate( turret.posOffset, -ent->ori );

                bool hasTarget = false;
                for( i32 entityIndexB = 0; entityIndexB < entities->GetCount(); entityIndexB++ ) {
                    if( entityIndexB == index ) {
                        continue;
                    }

                    const SimEntity * otherEnt = *entities->Get( entityIndexB );

                    f32 dist2 = glm::distance2( worldPos, otherEnt->pos );
                    if( dist2 <= turret.fireRange * turret.fireRange ) {
                        glm::vec2 nvel = glm::normalize( otherEnt->pos - worldPos );
                        turret.ori = atan2f( nvel.x, nvel.y );
                        hasTarget = true;
                        
                        if( turret.fireTimer == 0.0f ) {
                            turret.fireTimer = turret.fireRate;
                            ent->actions.AddAction( MapActionType::SIM_ENTITY_SPAWN, (i32)EntityType::BULLET_SMOL, ent->playerNumber, ent->teamNumber, worldPos, glm::vec2( 0.0f ) );
                            break;
                        }
                    }
                }

                if( hasTarget == false ) {
                    turret.ori = ent->ori;
                }
            }
        }

        //ent->acc.x -= ent->vel.x * ent->resistance;
        //ent->acc.y -= ent->vel.y * ent->resistance;
        ent->vel += ent->acc * TURNS_DELTA;
        ent->pos += ent->vel * TURNS_DELTA;
    }

    class SimMap_UpdateTask : public enki::ITaskSet {
    public:
        i32                                         startIndex = 0;
        i32                                         endIndex = 0;
        ConstEntList *                              entities = nullptr;
        EntCacheList *                              entityCache = nullptr;

        SimMap_UpdateTask() {}
        
        SimMap_UpdateTask( const SimMap_UpdateTask & other ) 
        { entities = other.entities;  entityCache = other.entityCache; startIndex = other.startIndex; endIndex = other.endIndex; };
        
        SimMap_UpdateTask( EntCacheList * entityCache, ConstEntList * entities, i32 startIndex, i32 endIndex ) :
            entityCache( entityCache ), entities( entities ), startIndex( startIndex ), endIndex( endIndex ) {}

        virtual void ExecuteRange( enki::TaskSetPartition range_, uint32_t threadnum_ ) override {
            for( i32 entityIndex = startIndex; entityIndex < endIndex; entityIndex++ ) {
                const SimEntity * ent = *entities->Get( entityIndex );
                SimEntity * cachedEnt = entityCache->Set_MemCpyPtr( entityIndex, ent );
                SimTick_SelfUpdate( entities, entityIndex, cachedEnt );
            }
        }
    };

    void SimMap::SimTick( MapTurn * turn1, MapTurn * turn2 ) {
        //ScopedClock timer( "SimTick", core );

        if( turn2 != nullptr ) {
           //core->LogOutput( LogLevel::INFO, "Ticking %d, p1=%d, p2=%d", turnNumber, turn1->turnNumber, turn2->turnNumber );
        }

        if( turn1 != nullptr ) {
            Sim_ApplyActions( &turn1->actions );
        }
        
        if( turn2 != nullptr ) {
            Sim_ApplyActions( &turn2->actions );
        }

        EntCacheList * entityCache = core->MemoryAllocateTransient<EntCacheList>();
        ConstEntList * entities = core->MemoryAllocateTransient<ConstEntList>();
        entityPool.GatherActiveObjs( entities );
        entityPool.GatherActiveObjs_MemCopy( entityCache );

    #if 0
        const i32 entityCount = entities->GetCount();
        const i32 threadCount = (i32)core->taskScheduler.GetNumTaskThreads();
        const i32 partitonCount = ( entityCount + threadCount - 1 ) / threadCount;

        std::vector<SimMap_UpdateTask> updateTasks; // @SPEED: This is a bit of a waste...
        updateTasks.reserve( threadCount );
        for( i32 threadIndex = 0; threadIndex < threadCount; threadIndex++ ) {
            i32 start = threadIndex * partitonCount;
            i32 end = glm::min( start + partitonCount, entityCount );
            updateTasks.emplace_back( entityCache, entities, start, end );
            core->taskScheduler.AddTaskSetToPipe( &updateTasks[ threadIndex ] );
        }

        for( i32 threadIndex = 0; threadIndex < threadCount; threadIndex++ ) {
            core->taskScheduler.WaitforTaskSet( &updateTasks[ threadIndex ] );
        }
    #else 
        const i32 entityCount = entities->GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            const SimEntity * ent = *entities->Get( entityIndex );
            SimEntity * cachedEnt = entityCache->Set_MemCpyPtr( entityIndex, ent );
            SimTick_SelfUpdate( entities, entityIndex, cachedEnt );
        }
    #endif

        // Apply map actions
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = entityCache->Get( entityIndex );
            if( ent->actions.data.GetSize() != 0 ) {
                Sim_ApplyActions( &ent->actions );
            }
        }

        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * cachedEnt = entityCache->Get( entityIndex );
            SimEntity * readEnt = const_cast< SimEntity * > ( *entities->Get( entityIndex ) ); // @NOTE: Being very sneaky here by const casting away the const.
            *readEnt = *cachedEnt;
        }

        turnNumber++;
    }

    void SimMap::Sim_ApplyActions( MapActionBuffer * actionBuffer ) {
        const i32 turnSize = actionBuffer->data.GetSize();
        char * turnData = actionBuffer->data.GetBuffer();
        i32 offset = 0;
        while( offset < turnSize ) {
            MapActionType actionType = (MapActionType)turnData[ offset ];
            if( actionType == MapActionType::NONE ) {
                core->LogOutput( LogLevel::ERR, "Can't apply a none turn" );
                break;
            }

            offset += sizeof( MapActionType );

            RpcHolder * holder = rpcTable[ (i32)actionType ];
            holder->Call( turnData + offset );
            i32 lastCallSize = holder->GetLastCallSize();
            offset += lastCallSize;
        }
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

    Collider2D SimEntity::GetWorldCollisionCollider() const {
        return ColliderForSpace( collisionCollider, pos );
    }

    Collider2D SimEntity::GetWorldSelectionCollider() const {
        return ColliderForSpace( selectionCollider, pos );
    }

    void SyncQueues::Start() {
        player1Turns.Clear();
        player2Turns.Clear();

        MapTurn turn = {};
        for( i32 i = 1; i <= slidingWindowWidth; i++ ) {
            turn.turnNumber = i;
            player1Turns.Enqueue( turn );
            player2Turns.Enqueue( turn );
        }
    }

    bool SyncQueues::CanTurn() {
        i32 player1Count = player1Turns.GetCount();
        i32 player2Count = player2Turns.GetCount();

        if( player1Count > 0 && player2Count > 0 ) {
            MapTurn * player1Turn = player1Turns.Peek();
            MapTurn * player2Turn = player2Turns.Peek();
            Assert( player1Turn->turnNumber == player2Turn->turnNumber );
            Assert( player1Turn->checkSum == player2Turn->checkSum );
            return true;
        }

        return false;
    }

    void SyncQueues::AddTurn( i32 playerNumber, const MapTurn & turn ) {
        if( playerNumber == 1 ) {
            player1Turns.Enqueue( turn );
        }
        else {
            player2Turns.Enqueue( turn );
        }
    }

    MapTurn * SyncQueues::GetNextTurn( i32 playerNumber ) {
        if( playerNumber == 1 ) {
            return player1Turns.Peek();
        }
        else {
            return player2Turns.Peek();
        }
    }

    void SyncQueues::FinishTurn() {
        player1Turns.Dequeue();
        player2Turns.Dequeue();
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

}

