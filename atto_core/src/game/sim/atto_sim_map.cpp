#include "atto_sim_map.h"

#include "../../shared/atto_rpc.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static FixedList<glm::vec2, 5 * 3> ui_LeftPanelCenters;
    static FixedList<glm::vec2, 5 * 3> ui_RightPanelCenters;

    static AudioResource * sndLaser1[23] = {};
    static AudioResource * sndLaser2[6] = {};

    EntityListFilter * EntityListFilter::Begin( EntList * activeEntities ) {
        this->activeEntities = activeEntities;
        const i32 entCount = activeEntities->GetCount();
        this->marks.SetCount( entCount );
        for ( i32 i = 0; i < entCount; i++ ) {
            this->marks[ i ] = true;
        }
        return this;
    }

    EntityListFilter * EntityListFilter::OwnedBy( PlayerNumber playerNumber ) {
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                SimEntity * ent = *activeEntities->Get( entityIndex );
                if ( ent->playerNumber != playerNumber ) {
                    marks[ entityIndex ] = false;
                }
            }
        }

        return this;
    }

    EntityListFilter * EntityListFilter::SelectedBy( PlayerNumber playerNumber ) {
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                SimEntity * ent = *activeEntities->Get( entityIndex );
                if ( ent->selectedBy.Contains( playerNumber ) == false ) {
                    marks[ entityIndex ] = false;
                }
            }
        }

        return this;
    }

    EntityListFilter * EntityListFilter::Type( EntityType::_enumerated type ) {
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                SimEntity * ent = *activeEntities->Get( entityIndex );
                if ( ent->type != type ) {
                    marks[ entityIndex ] = false;
                }
            }
        }
        return this;
    }

    EntityListFilter * EntityListFilter::IsTypeRange( EntityType::_enumerated start, EntityType::_enumerated end ) {
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                SimEntity * ent = *activeEntities->Get( entityIndex );
                if( !( ent->type > start && ent->type < end ) ) {
                    marks[ entityIndex ] = false;
                }
            }
        }
        return this;
    }

    EntityListFilter * EntityListFilter::End() {
        result.Clear();
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                result.Add ( *activeEntities->Get( entityIndex ) );
            }
        }
        return this;
    }

    struct TypeDescriptor_EntitySelectionChange : TypeDescriptor {
        TypeDescriptor_EntitySelectionChange() {
            name = "EntitySelectionChange";
            size = sizeof( EntitySelectionChange );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            INVALID_CODE_PATH;
            return nlohmann::json();
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH;
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            INVALID_CODE_PATH;
        }

        virtual LargeString ToString( const void * obj ) override {
            i32 index = (i32)(*(EntitySelectionChange *)obj);
            return LargeString::FromLiteral( EntitySelectionChangeStrings[ index ] );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<EntitySelectionChange>() {
        static TypeDescriptor_EntitySelectionChange typeDesc;
        return &typeDesc;
    }

    struct TypeDescriptor_PlanetPlacementType : TypeDescriptor {
        TypeDescriptor_PlanetPlacementType() {
            name = "PlanetPlacementType";
            size = sizeof( PlanetPlacementType );
        }

        virtual nlohmann::json JSON_Write( const void * obj ) override {
            INVALID_CODE_PATH;
            return nlohmann::json();
        }

        virtual void JSON_Read( const nlohmann::json & j, const void * obj ) override {
            INVALID_CODE_PATH;
        }

        virtual void Imgui_Draw( const void * obj, const char * memberName ) override {
            INVALID_CODE_PATH;
        }

        virtual LargeString ToString( const void * obj ) override {
            i32 index = (i32)( *(PlanetPlacementType *)obj );
            return LargeString::FromLiteral( PlanetPlacementTypeStrings[ index ] );
        }
    };

    template <>
    TypeDescriptor * GetPrimitiveDescriptor<PlanetPlacementType>() {
        static TypeDescriptor_PlanetPlacementType typeDesc;
        return &typeDesc;
    }

    bool EntityListFilter::ContainsOnlyType( EntityType::_enumerated type ) {
        const i32 entCount = result.GetCount();
        if( entCount == 0 ) {
            return false;
        }

        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            SimEntity * ent = *result.Get( entityIndex );
            if ( ent->type != type ) {
                return false;
            }
        }
        return true;
    }

    bool EntityListFilter::ContainsOnlyOneOfType( EntityType::_enumerated type ) {
        i32 count = 0;
        const i32 entCount = result.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            SimEntity * ent = *result.Get( entityIndex );
            if ( ent->type == type ) {
                count++;
                if ( count > 1 ) {
                    return false;
                }
            } else {
                return false;
            }
        }
        return count == 1;
    }

    void SimMap::Initialize( Core * core ) {
        // @TODO: This won't work if we have more than one map instance !!
        if( rpcTable[ 1 ] == nullptr ) {
            rpcTable[ ( i32 )MapActionType::PLAYER_SELECTION ]                          = new RpcMemberFunction( this, &SimMap::SimAction_PlayerSelect );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_MOVE ]              = new RpcMemberFunction( this, &SimMap::SimAction_Move );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK ]            = new RpcMemberFunction( this, &SimMap::SimAction_Attack );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING ] = new RpcMemberFunction( this, &SimMap::SimAction_ContructBuilding );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING ] = new RpcMemberFunction( this, &SimMap::SimAction_ContructExistingBuilding );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT]  = new RpcMemberFunction( this, &SimMap::SimAction_PlanetPlacePlacement );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT ]    = new RpcMemberFunction( this, &SimMap::SimAction_BuildingTrainUnit );

            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_SPAWN ]                          = new RpcMemberFunction( this, &SimMap::SimAction_SpawnEntity );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_DESTROY ]                        = new RpcMemberFunction( this, &SimMap::SimAction_DestroyEntity );

            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_APPLY_DAMAGE ]                   = new RpcMemberFunction( this, &SimMap::SimAction_ApplyDamage );
            rpcTable[ ( i32 )MapActionType::SIM_ENTITY_APPLY_CONSTRUCTION ]             = new RpcMemberFunction( this, &SimMap::SimAction_ApplyContruction );

            rpcTable[ ( i32 )MapActionType::SIM_MAP_MONIES_GIVE_CREDITS ]               = new RpcMemberFunction( this, &SimMap::SimAction_GiveCredits );
            rpcTable[ ( i32 )MapActionType::SIM_MAP_MONIES_GIVE_ENERGY ]                = new RpcMemberFunction( this, &SimMap::SimAction_GiveEnergy );
            rpcTable[ ( i32 )MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE ]               = new RpcMemberFunction( this, &SimMap::SimAction_GiveCompute );
        }

        mapReplay.Prepare();

        const f32 panelDim = 16 + 1;
        const glm::vec2 ui_LeftFirst = glm::vec2( 193, 44 );
        for( i32 x = 0; x < 5; x++ ) {
            for( i32 y = 0; y < 3; y++ ) {
                i32 flatIndex = x + y * 5;
                ui_LeftPanelCenters[ flatIndex ] = ui_LeftFirst + glm::vec2( x * panelDim, -y * panelDim ) + glm::vec2( 0.5f, -0.5f );
            }
        }

        const glm::vec2 ui_RightFirst = glm::vec2( 378, 44 );
        for ( i32 x = 0; x < 5; x++ ) {
            for ( i32 y = 0; y < 3; y++ ) {
                i32 flatIndex = x + y * 5;
                ui_RightPanelCenters[ flatIndex ] = ui_RightFirst + glm::vec2( x * panelDim, -y * panelDim ) + glm::vec2( 0.5f, -0.5f );
            }
        }

        this->core = core;
        syncQueues.Start();

        for ( i32 i = 0; i < 23; i++ ) {
            LargeString name = StringFormat::Large( "res/sounds/laser_1/blue_laser_%d.wav", i + 1 );
            sndLaser1[ i ] = core->ResourceGetAndCreateAudio( name.GetCStr(), true, false, 4000.0f, 10000.0f );
        }

        for ( i32 i = 0; i < 6; i++ ) {
            LargeString name = StringFormat::Large( "res/sounds/laser_2/sci-fi_weapon_plasma_pistol_0%d.wav", i + 1 );
            sndLaser2[ i ] = core->ResourceGetAndCreateAudio( name.GetCStr(), true, false, 4000.0f, 10000.0f );
        }

        playerNumbers.Add( PlayerNumber::Create( 1 ) );
        playerNumbers.Add( PlayerNumber::Create( 2 ) );

        PlayerNumber p0 = PlayerNumber::Create( 0 );
        TeamNumber t0 = TeamNumber::Create( 0 );
        PlayerNumber p1 = PlayerNumber::Create( 1 );
        TeamNumber t1 = TeamNumber::Create( 1 );
        PlayerNumber p2 = PlayerNumber::Create( 2 );
        TeamNumber t2 = TeamNumber::Create( 2 );

        SpawnEntity( EntityType::Make( EntityType::PLANET ), p1, t1, Fp2( 500.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_WORKER ), p1, t1, Fp2( 700.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        
        SpawnEntity( EntityType::Make( EntityType::PLANET ), p2, t2, Fp2( 2500.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_WORKER ), p2, t2, Fp2( 2400.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        
        SpawnEntity( EntityType::Make( EntityType::STAR ), p0, t0, Fp2( 1500.0f, 1200.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        
        SpawnEntity( EntityType::Make( EntityType::BUILDING_STATION ), p1, t1, Fp2( 500.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) )->building.isBuilding = false;
        
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, Fp2( 800.2f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, Fp2( 900.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, Fp2( 1000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, Fp2( 1100.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p2, t2, Fp2( 2300.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p2, t2, Fp2( 2200.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p2, t2, Fp2( 2100.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p2, t2, Fp2( 2000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
    }

    void SimMap::Update( Core * core, f32 dt ) {
        //ScopedClock timer( "Update", core );

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
            if( dtAccumulator > SIM_DT_FLOAT ) {
                dtAccumulator -= SIM_DT_FLOAT;
                if( syncQueues.CanTurn() == false ) {
                    syncTurnAttempts++;

                    if( syncTurnAttempts >= syncQueues.GetSlidingWindowWidth() * 4 ) {
                        syncWaitTurnCounter = syncQueues.GetSlidingWindowWidth() * 2;
                        syncTurnAttempts = 0;
                    }

                    core->LogOutput( LogLevel::WARN, "Can't turn, going to wait" );
                    return;
                }

                if ( syncWaitTurnCounter > 0 ) {
                    syncWaitTurnCounter--;
                    core->LogOutput( LogLevel::WARN, "Waiting for %d turns", syncWaitTurnCounter );
                    return;
                }

                syncTurnAttempts = 0;

                MapTurn * playerOneTurn = syncQueues.GetNextTurn( PlayerNumber::Create( 1 ) );
                MapTurn * playerTwoTurn = syncQueues.GetNextTurn( PlayerNumber::Create( 2 ) );

                SimTick( playerOneTurn, playerTwoTurn );

                syncQueues.FinishTurn();

                localMapTurn.playerNumber = localPlayerNumber;
                localMapTurn.turnNumber = turnNumber + syncQueues.GetSlidingWindowWidth();
                localMapTurn.checkSum = Sim_CheckSum();
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
            if ( dtAccumulator > SIM_DT_FLOAT ) {
                dtAccumulator -= SIM_DT_FLOAT;

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
        static TextureResource * background = core->ResourceGetAndLoadTexture( "res/game/backgrounds/dark_blue.png" );
        static TextureResource * sprTurretSmol = core->ResourceGetAndLoadTexture( "res/ents/test/turret_smol.png" );
        static TextureResource * sprTurretMed = core->ResourceGetAndLoadTexture( "res/ents/test/turret_med.png" );
        static TextureResource * sprUIMock = core->ResourceGetAndLoadTexture( "res/ents/test/ui_mock_01.png" );
        static TextureResource * sprMoveLocation = core->ResourceGetAndLoadTexture( "res/ents/test/move_location.png" );

        const f32 cameraSpeed = 20.0f;
        if ( core->InputKeyDown( KEY_CODE_W ) == true ) {
            localCameraPos.y += cameraSpeed;
        }
        if ( core->InputKeyDown( KEY_CODE_S ) == true ) {
            localCameraPos.y -= cameraSpeed;
        }
        if ( core->InputKeyDown( KEY_CODE_A ) == true ) {
            localCameraPos.x -= cameraSpeed;
        }
        if ( core->InputKeyDown( KEY_CODE_D ) == true ) {
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
        if( glm::abs( localCameraZoomLerp.x - wantedResolution.x ) < 5 ) {
            localCameraZoomLerp = wantedResolution;
        }

        localCameraPos -= ( localCameraZoomLerp - oldCameraWidth ) * 0.5f;

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, true );
        //DrawContext * backgroundDrawContext = core->RenderGetDrawContext( 1, true );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 2, true );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 3, true );

        spriteDrawContext->SetCameraDims( localCameraZoomLerp.x, localCameraZoomLerp.y );
        uiDrawContext->SetCameraDims( 640, 360 );
        debugDrawContext->SetCameraDims( localCameraZoomLerp.x, localCameraZoomLerp.y );

        const glm::vec2 mapMin = glm::vec2( 0.0f );
        const glm::vec2 mapMax = glm::vec2( (f32)MAX_MAP_SIZE ) - glm::vec2( spriteDrawContext->GetCameraWidth(), spriteDrawContext->GetCameraHeight() );
        localCameraPos = glm::clamp( localCameraPos, mapMin, mapMax );

        spriteDrawContext->SetCameraPos( localCameraPos );
        debugDrawContext->SetCameraPos( localCameraPos );

        const glm::vec2 localCameraCenter = localCameraPos + glm::vec2( spriteDrawContext->GetCameraWidth(), spriteDrawContext->GetCameraHeight() ) / 2.0f;
        core->AudioSetListener( localCameraCenter );
        //core->LogOutput( LogLevel::INFO, "%f, %f", localCameraCenter.x, localCameraCenter.y );

        spriteDrawContext->DrawTextureBL( background, glm::vec2( 0, 0 ) );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );
        const glm::vec2 mousePosUISpace = uiDrawContext->ScreenPosToWorldPos( mousePosPix );
        const fp2 mousePosWorldFp = Fp2( mousePosWorld );

        BoxBounds2D uiBounds = {};
        uiBounds.min = glm::vec2( 114, 0 );
        uiBounds.max = glm::vec2( 524, 60 );
        bool isMouseOverUI = uiBounds.Contains( mousePosUISpace );

        EntList & entities = *core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );

        EntityListFilter * entityFilter = core->MemoryAllocateTransient<EntityListFilter>();

        SmallString creditsStr = StringFormat::Small( "Credits: %d", playerMonies[ ( localPlayerNumber - PlayerNumber::Create( 1 ) ).value ].credits );
        SmallString energyStr = StringFormat::Small( "Energy: %d", playerMonies[ ( localPlayerNumber - PlayerNumber::Create( 1 ) ).value ].energy );
        SmallString computeStr = StringFormat::Small( "Compute: %d", playerMonies[ (localPlayerNumber - PlayerNumber::Create( 1 )).value ].compute );

        uiDrawContext->DrawTexture( sprUIMock, glm::vec2( 640 / 2, 360 / 2 ) );
        gameUI.Begin( uiDrawContext->GetCameraDims() );
        gameUI.LablePix( "Planet.", glm::vec2( 228, 58 ) );
        gameUI.LablePix( "Production.", glm::vec2( 420, 58 ) );

        gameUI.LablePix( creditsStr.GetCStr(), glm::vec2( 488, 34 ) );
        gameUI.LablePix( energyStr.GetCStr(), glm::vec2( 488, 34 - 12  ) );
        gameUI.LablePix( computeStr.GetCStr(), glm::vec2( 488, 34 -12 -12 ) );

        const bool onlyPlanetSelected = entityFilter->Begin( &entities )->
            OwnedBy( localPlayerNumber )->
            SelectedBy( localPlayerNumber )->
            End()->
            ContainsOnlyOneOfType( EntityType::PLANET );

        const bool onlyWorkerSelected = entityFilter->Begin( &entities )->
            OwnedBy( localPlayerNumber )->
            SelectedBy( localPlayerNumber )->
            End()->
            ContainsOnlyType( EntityType::UNIT_WORKER );

        const bool onlyBuildingSelected = entityFilter->Begin( &entities )->
            OwnedBy( localPlayerNumber )->
            SelectedBy( localPlayerNumber )->
            End()->
            ContainsOnlyType( EntityType::BUILDING_STATION );

        if ( onlyPlanetSelected == true ) {
            SimEntity * ent = entityFilter->result[ 0 ];
            Planet & planet = ent->planet;
            if ( planetPlacementSubMenu == false ) {
                const i32 placementCount = planet.placements.GetCapcity();
                for ( i32 placementIndex = 0; placementIndex < placementCount; placementIndex++ ) {
                    const PlanetPlacementType & placementType = planet.placements[ placementIndex ];
                    const glm::vec2 pos = ui_LeftPanelCenters[ placementIndex ];
                    const glm::vec2 size = glm::vec2( 15 );

                    switch ( placementType ) {
                        case PlanetPlacementType::INVALID: {
                            gameUI.ColorBlockPix( placementIndex, pos, size, Colors::BLACK );
                        } break;
                        case PlanetPlacementType::BLOCKED: {
                            gameUI.ColorBlockPix( placementIndex, pos, size, Colors::ALIZARIN );
                        } break;
                        case PlanetPlacementType::OPEN: {
                            if ( gameUI.ButtonPix( placementIndex, "O", pos, size, Colors::MIDNIGHT_BLUE ) ) {
                                planetPlacementSubMenu = true;
                                planetPlacementSubMenuIndex = placementIndex;
                            }
                        } break;
                        case PlanetPlacementType::CREDIT_GENERATOR: {
                            gameUI.ColorBlockPix( placementIndex, pos, size, Colors::GREEN );
                        } break;
                        case PlanetPlacementType::ENERGY_GENERATOR: {
                            gameUI.ColorBlockPix( placementIndex, pos, size, Colors::GOLD );
                        } break;
                        case PlanetPlacementType::COMPUTE_GENERATOR: {
                            gameUI.ColorBlockPix( placementIndex, pos, size, Colors::SKY_BLUE );
                        } break;
                    }
                }
            } else {
                const glm::vec2 s = glm::vec2( 15 );
                if ( gameUI.ButtonPix( 145, "", ui_LeftPanelCenters[0], s, Colors::GREEN ) ) {
                    localActionBuffer.AddAction( MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::CREDIT_GENERATOR );
                    planetPlacementSubMenu = false;
                    planetPlacementSubMenuIndex = -1;
                }
                if ( gameUI.ButtonPix( 146, "", ui_LeftPanelCenters[1], s, Colors::GOLD ) ) {
                    localActionBuffer.AddAction( MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::ENERGY_GENERATOR );
                    planetPlacementSubMenu = false;
                    planetPlacementSubMenuIndex = -1;
                }
                if ( gameUI.ButtonPix( 147, "", ui_LeftPanelCenters[2], s, Colors::SKY_BLUE ) ) {
                    localActionBuffer.AddAction( MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::COMPUTE_GENERATOR );
                    planetPlacementSubMenu = false;
                    planetPlacementSubMenuIndex = -1;
                }
            }
        } else {
            planetPlacementSubMenu = false;
            planetPlacementSubMenuIndex = -1;
        }

        if ( onlyWorkerSelected == true ) {
            SimEntity * ent = entityFilter->result[ 0 ];
            Unit & unit = ent->unit;

            if ( isPlacingBuilding == false ) {
                const glm::vec2 s = glm::vec2( 15 );
                if( core->InputKeyJustPressed( KEY_CODE_Q ) || gameUI.ButtonPix( 232, "Q", ui_RightPanelCenters[ 0 ], s, Colors::SKY_BLUE ) ) {
                    isPlacingBuilding = true;
                    placingBuildingType = EntityType::BUILDING_STATION;
                }
                if ( core->InputKeyJustPressed( KEY_CODE_E ) || gameUI.ButtonPix( 233, "E", ui_RightPanelCenters[ 1 ], s, Colors::SKY_BLUE ) ) {
                    isPlacingBuilding = true;
                    placingBuildingType = EntityType::BUILDING_SOLAR_ARRAY;
                }
                if ( core->InputKeyJustPressed( KEY_CODE_C ) || gameUI.ButtonPix( 234, "C", ui_RightPanelCenters[ 2 ], s, Colors::SKY_BLUE ) ) {
                    isPlacingBuilding = true;
                    placingBuildingType = EntityType::BUILDING_COMPUTE;
                }
            }
        }

        if ( onlyBuildingSelected == true ) {
            const glm::vec2 s = glm::vec2( 15 );
            if ( core->InputKeyJustPressed( KEY_CODE_Q ) || gameUI.ButtonPix( 242, "Q", ui_RightPanelCenters[ 0 ], s, Colors::SKY_BLUE ) ) {
                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, (i32)EntityType::UNIT_WORKER );
            }
            if ( core->InputKeyJustPressed( KEY_CODE_E ) || gameUI.ButtonPix( 243, "E", ui_RightPanelCenters[ 1 ], s, Colors::SKY_BLUE ) ) {
                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, (i32)EntityType::UNIT_TEST );
            }
        }

        gameUI.UpdateAndRender( core, uiDrawContext );
        gameUI.End();

        isMouseOverUI |= gameUI.mouseOverAnyElements;

        if( isMouseOverUI == false ) {
            if ( isPlacingBuilding == true ) {
                if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
                    isPlacingBuilding = false;
                }

                if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                    isPlacingBuilding = false;
                    localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING, localPlayerNumber, ( i32 )placingBuildingType, mousePosWorldFp );
                }
            } else {
                if ( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                    localIsDragging = true;
                    localStartDrag = mousePosWorld;
                    localEndDrag = mousePosWorld;
                    localDragSelection.Clear();
                }
            }
        }

        if ( localIsDragging == true ) {
            localEndDrag = mousePosWorld;
            if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                localIsDragging = false;

                bool hasLocalPlayerUnits = false;
                bool hasPlanetType = false;
                bool hasBuildingType = false;
                bool hasUnitType = false;

                // @NOTE: Selection Priority
                i32 selectionCount = localDragSelection.GetCount();
                for ( i32 selectionIndex = 0; selectionIndex < selectionCount; selectionIndex++ ) {
                    EntityHandle handle = localDragSelection[ selectionIndex ];
                    SimEntity * selectionEnt = entityPool.Get( handle );
                    if ( selectionEnt != nullptr ) {
                        if ( selectionEnt->playerNumber == localPlayerNumber ) {
                            hasLocalPlayerUnits = true;
                        }
                        if ( selectionEnt->type == EntityType::PLANET ) {
                            hasPlanetType = true;
                        }
                        else if ( IsBuildingType( selectionEnt->type ) == true ) {
                            hasBuildingType = true;
                        }
                        else if ( IsUnitType( selectionEnt->type ) == true ) {
                            hasUnitType = true;
                        }
                    }
                }

                for ( i32 selectionIndex = 0; selectionIndex < localDragSelection.GetCount(); selectionIndex++ ) {
                    EntityHandle handle = localDragSelection[ selectionIndex ];
                    SimEntity * selectionEnt = entityPool.Get( handle );
                    if ( selectionEnt != nullptr ) {
                        if ( hasLocalPlayerUnits == true && selectionEnt->playerNumber != localPlayerNumber ) {
                            localDragSelection.RemoveIndex( selectionIndex );
                            selectionIndex--;
                        }
                        else if ( hasUnitType == true ) {
                            if ( IsBuildingType( selectionEnt->type ) == true || selectionEnt->type == EntityType::PLANET ) {
                                localDragSelection.RemoveIndex( selectionIndex );
                                selectionIndex--;
                            }
                        } else if ( hasBuildingType == true ) {
                            if ( selectionEnt->type == EntityType::PLANET ) {
                                localDragSelection.RemoveIndex( selectionIndex );
                                selectionIndex--;
                            }
                        }
                    }
                }

                VisAction_PlayerSelect( &localPlayerNumber, &localDragSelection,  EntitySelectionChange::SET );
                localActionBuffer.AddAction( MapActionType::PLAYER_SELECTION, localPlayerNumber, localDragSelection, EntitySelectionChange::SET );
            }
        }

        BoxBounds2D selectionBounds = {};
        selectionBounds.min = glm::min( localStartDrag, localEndDrag );
        selectionBounds.max = glm::max( localStartDrag, localEndDrag );

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            SimEntity * ent = entities[ entityIndexA ];

            ent->visPos = glm::mix( ent->visPos, ToVec2( ent->pos ), dt * 7.0f );
            ent->visOri = glm::mix( ent->visOri, ToFloat( ent->ori ), dt * 7.0f );

            glm::vec2 drawPos = ent->visPos;
            f32 drawOri = ent->visOri;

            if( ent->visSelectedBy.Contains( localPlayerNumber ) && ent->playerNumber == localPlayerNumber && ent->navigator.hasDest == true ) {
                glm::vec2 dest = ToVec2( ent->navigator.dest );
                spriteDrawContext->DrawTexture( sprMoveLocation, dest, 0.0f );
            }

            if( ent->visSelectedBy.Contains( localPlayerNumber ) && ent->selectionAnimator.sprite != nullptr ) {
                //spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, ent->pos, ent->ori );
                spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, drawPos, drawOri );
            }

            ent->spriteAnimator.Update( core, dt );
            if( ent->spriteAnimator.sprite != nullptr ) {
                spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, drawOri, glm::vec2( 1 ), ent->spriteAnimator.color );
            }

            if( IsUnitType( ent->type ) ) {
                const Unit & unit = ent->unit;
                const i32 turretCount = unit.turrets.GetCount();
                for( i32 turretIndex = 0; turretIndex < turretCount; turretIndex++ ) {
                    UnitTurret & turret = unit.turrets[ turretIndex ];
                    glm::vec2 worldPos = ent->visPos + glm::rotate( ToVec2( turret.posOffset ), -ent->visOri );

                    if( turret.size == WeaponSize::SMALL ) {
                       // spriteDrawContext->DrawTexture( sprTurretSmol, worldPos, ToFloat( turret.ori ) );
                    }
                    else if( turret.size == WeaponSize::MEDIUM ) {
                       // spriteDrawContext->DrawTexture( sprTurretMed, worldPos, ToFloat( turret.ori ) );
                    }
                }
            }
            else if ( IsBuildingType( ent->type ) ) {
                const Building & building = ent->building;
                if ( building.isBuilding == true ) {
                    const f32 f = ( f32 ) building.turn / building.timeToBuildTurns;
                    glm::vec2 bl = ent->visPos + glm::vec2( -50, 20 );
                    glm::vec2 tr = ent->visPos + glm::vec2( 50, 30 );
                    spriteDrawContext->DrawRect( bl, tr, glm::vec4( 0.7f ) );
                    tr.x = glm::mix( bl.x, tr.x, f );
                    spriteDrawContext->DrawRect( bl, tr, glm::vec4( 0.9f ) );
                } else if ( building.isTraining == true ) {
                    ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteBank[ 1 ], false );
                    const f32 f = ( f32 ) building.turn / building.timeToTrainTurns;
                    glm::vec2 bl = ent->visPos + glm::vec2( -50, 20 );
                    glm::vec2 tr = ent->visPos + glm::vec2( 50, 30 );
                    spriteDrawContext->DrawRect( bl, tr, glm::vec4( 0.7f ) );
                    tr.x = glm::mix( bl.x, tr.x, f );
                    spriteDrawContext->DrawRect( bl, tr, glm::vec4( 0.9f ) );
                } else {
                    ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteBank[ 0 ], false );
                }
            }

            if ( localIsDragging == true && ent->isSelectable == true ) {
                Collider2D collider = ent->GetWorldSelectionCollider();
                if ( collider.Intersects( selectionBounds ) == true ) {
                    localDragSelection.AddUnique( ent->handle );
                }
            }

            if( isPlacingBuilding == true ) {
                spriteDrawContext->DrawTexture( sprMoveLocation, mousePosWorld );
            }

            // DEBUG
            #if ATTO_DEBUG
            if ( false ) {
                if ( ent->selectionCollider.type == ColliderType::COLLIDER_TYPE_CIRCLE ) {
                    debugDrawContext->DrawCircle( ent->visPos, ent->selectionCollider.circle.rad );
                } else if (ent->selectionCollider.type == ColliderType::COLLIDER_TYPE_AXIS_BOX ) {
                    Collider2D c = ent->GetWorldSelectionCollider();
                    debugDrawContext->DrawRect( c.box.min, c.box.max );
                }
            }

            #endif
        }

        if( isMouseOverUI == false && core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
            bool inputMade = false;
            for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                const SimEntity * ent = entities[ entityIndexA ];
                if( ent->isSelectable == true ) {
                    const Collider2D selectionCollider = ent->GetWorldSelectionCollider();
                    if( selectionCollider.Contains( mousePosWorld ) ) {
                        if ( IsUnitType( ent->type ) ) {
                            if ( ent->teamNumber != localPlayerTeamNumber ) {
                                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK, localPlayerNumber, ent->handle );
                            }
                            else {
                                // @TODO: Follow
                                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_MOVE, localPlayerNumber, ent->pos );
                            }
                        } else if ( IsBuildingType( ent->type ) == true && ent->building.isBuilding == true ) {
                            if ( ent->teamNumber != localPlayerTeamNumber ) {
                                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK, localPlayerNumber, ent->handle );
                            } else {
                                for ( i32 entityIndexB = 0; entityIndexB < entities.GetCount(); entityIndexB++ ) {
                                    const SimEntity * otherEnt = *entities.Get( entityIndexB );
                                    if ( otherEnt->playerNumber == localPlayerNumber && otherEnt->type == EntityType::UNIT_WORKER ) {
                                        localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING, localPlayerNumber, ent->handle );
                                        break;
                                    }
                                }
                            }
                        }

                        inputMade = true;
                        break;
                    }
                }
            }

            if( inputMade == false ) {
                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_MOVE, localPlayerNumber, mousePosWorldFp );
            }
        }

        if ( localIsDragging == true ) {
            glm::vec2 minBox = glm::min( localStartDrag, localEndDrag );
            glm::vec2 maxBox = glm::max( localStartDrag, localEndDrag );
            spriteDrawContext->DrawRect( minBox, maxBox, Colors::BOX_SELECTION_COLOR );
        }

        if( core->InputKeyJustPressed( KEY_CODE_F2 ) == true ) {
            mapReplay.PrintActions( core );
        }

        SmallString s = StringFormat::Small( "ping=%d", (i32)core->NetworkGetPing() );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 128, 128 ), 32, s.GetCStr() );
        s = StringFormat::Small( "dt=%f", dt );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 128, 160 ), 32, s.GetCStr() );
        s = StringFormat::Small( "fps=%f", 1.0f / dt );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 128, 200 ), 32, s.GetCStr() );

        //ScopedClock renderingClock( "Rendering clock", core );
        core->RenderSubmit( spriteDrawContext, true );
        //core->RenderSubmit( backgroundDrawContext, false );
        core->RenderSubmit( uiDrawContext, false );
        core->RenderSubmit( debugDrawContext, false );
    }

    SimEntity * SimMap::SpawnEntity( EntityType type, PlayerNumber playerNumber, TeamNumber teamNumber, fp2 pos, fp ori, fp2 vel ) {
        EntityHandle handle = {};
        SimEntity * entity = entityPool.Add( handle );
        AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
        if( entity != nullptr ) {
            ZeroStructPtr( entity );
            entity->handle = handle;

            entity->active = true;
            entity->type = type;
            entity->resistance = ToFP( 14.0f );
            entity->pos = pos;
            entity->visPos = ToVec2( pos );
            entity->ori = ori;
            entity->visOri = ToFloat( ori );
            entity->vel = vel;
            entity->teamNumber = teamNumber;
            entity->playerNumber = playerNumber;

            switch( entity->type ) {
                case EntityType::UNIT_WORKER: {
                    static SpriteResource * blueSprite = core->ResourceGetAndCreateSprite( "res/ents/test/worker_blue.png", 1, 32, 32, 0 );
                    static SpriteResource * redSprite = core->ResourceGetAndCreateSprite( "res/ents/test/worker_red.png", 1, 32, 32, 0 );
                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
                    SpriteResource * mainSprite = teamNumber.value == 1 ? blueSprite : redSprite;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( selectionSprite, false );

                    entity->isSelectable = true;
                    entity->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->selectionCollider.circle.pos = glm::vec2( 0, 0 );
                    entity->selectionCollider.circle.rad = 16.0f;
                    
                    entity->unit.averageRange = ToFP( 40 );
                    entity->unit.maxHealth = 50;
                    entity->unit.currentHealth = entity->unit.maxHealth;
                } break;
                case EntityType::UNIT_TEST:
                {
                    static SpriteResource * blueSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_blue.png", 1, 48, 48, 0 );
                    static SpriteResource * redSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_red.png", 1, 48, 48, 0 );
                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
                    SpriteResource * mainSprite = teamNumber.value == 1 ? blueSprite : redSprite;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( selectionSprite, false );

                    entity->isSelectable = true;
                    entity->selectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->selectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 36 ) );

                    entity->unit.averageRange = ToFP( 400 );
                    entity->unit.maxHealth = 100;
                    entity->unit.currentHealth = entity->unit.maxHealth;

                    UnitTurret turret1 = {};
                    turret1.size = WeaponSize::SMALL;
                    turret1.posOffset = Fp2( -4, 7 );
                    turret1.fireRate = Fp( 1.25f );
                    turret1.fireRange = Fp( 400 );
                    entity->unit.turrets.Add( turret1 );

                    UnitTurret turret2 = {};
                    turret2.size = WeaponSize::SMALL;
                    turret2.posOffset = Fp2( 4, 7 );
                    turret2.fireRate = Fp( 1.25f );
                    turret2.fireRange = Fp( 400 );
                    entity->unit.turrets.Add( turret2 );

                    UnitTurret turret3 = {};
                    turret3.size = WeaponSize::MEDIUM;
                    turret3.posOffset = Fp2( -7, -5 );
                    turret3.fireRate = Fp( 1.25f );
                    turret3.fireRange = Fp( 400 );
                    entity->unit.turrets.Add( turret3 );

                    UnitTurret turret4 = {};
                    turret4.size = WeaponSize::MEDIUM;
                    turret4.posOffset = Fp2( 7, -5 );
                    turret4.fireRate = Fp( 1.25f );
                    turret4.fireRange = Fp( 400 );
                    entity->unit.turrets.Add( turret4 );
                } break;
                case EntityType::BULLET_SMOL:
                {
                    static SpriteResource * mainSprite = core->ResourceGetAndCreateSprite( "res/ents/test/bullet_smol.png", 1, 7, 7, 0 );
                    static SpriteResource * sprVFX_SmallExplody = core->ResourceGetAndCreateSprite( "res/ents/test/bullet_hit_smol.png", 3, 16, 16, 10 );
                    static AudioResource * audVFX_SmallExplody = core->ResourceGetAndCreateAudio( "res/sounds/tomwinandysfx_explosions_volume_i_closeexplosion_01.wav", true, false, 400.0f, 1000.0f );

                    entity->spriteBank[0] = mainSprite;
                    entity->spriteBank[1] = sprVFX_SmallExplody;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );

                    entity->bullet.aliveTime = Fp( 1.87f );
                    entity->bullet.damage = 5;

                    core->AudioPlay( sndLaser1[ Random::Int( 6 ) ] );
                } break;
                case EntityType::BULLET_MED:
                {
                    static SpriteResource * mainSprite = core->ResourceGetAndCreateSprite( "res/ents/test/bullet_med.png", 1, 8, 8, 0 );
                    static SpriteResource * sprVFX_SmallExplody = core->ResourceGetAndLoadSprite( "res/sprites/vfx_small_explody/vfx_small_explody.json" );
                    entity->spriteBank[0] = mainSprite;
                    entity->spriteBank[1] = sprVFX_SmallExplody;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );
                    
                    entity->bullet.aliveTime = Fp( 3 );
                    entity->bullet.damage = 8;
                } break;
                case EntityType::STAR:
                {
                    static SpriteResource * mainSprite = core->ResourceGetAndCreateSprite( "res/ents/test/star.png", 1, 500, 500, 0 );
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, true );
                } break;
                case EntityType::PLANET:
                {
                    static SpriteResource * mainSprite = core->ResourceGetAndCreateSprite( "res/ents/test/planet_big.png", 1, 250, 250, 0 );
                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/planet_big_selection.png", 1, 250, 250, 0 );
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( selectionSprite, false );

                    entity->isSelectable = true;
                    entity->selectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->selectionCollider.circle.pos = glm::vec2( 0, 0 );
                    entity->selectionCollider.circle.rad = 125.0f;

                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                } break;
                case EntityType::BUILDING_STATION:
                {
                    static SpriteResource * blueSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_blue_off.png", 1, 64, 64, 0 );
                    static SpriteResource * blueSpriteOn =  core->ResourceGetAndCreateSprite( "res/ents/test/building_station_blue_on.png", 1, 64, 64, 0 );
                    static SpriteResource * redSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_red_off.png", 1, 64, 64, 0 );
                    static SpriteResource * redSpriteOn  = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_red_on.png", 1, 64, 64, 0 );
                    
                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
                    entity->selectionAnimator.SetSpriteIfDifferent( selectionSprite, false );

                    if ( teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( blueSpriteOff, false );
                        entity->spriteBank.Add( blueSpriteOff );
                        entity->spriteBank.Add( blueSpriteOn );
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( redSpriteOff, false );
                        entity->spriteBank.Add( redSpriteOff );
                        entity->spriteBank.Add( redSpriteOn );
                    }

                    entity->isSelectable = true;
                    entity->selectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->selectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 64 ) );

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 60 );
                } break;
                case EntityType::BUILDING_SOLAR_ARRAY:
                {
                    static SpriteResource * blueSprite = core->ResourceGetAndCreateSprite( "res/ents/test/building_solar_array_blue.png", 1, 64, 32, 0 );
                    static SpriteResource * redSprite = core->ResourceGetAndCreateSprite( "res/ents/test/building_solar_array_red.png", 1, 64, 32, 0 );

                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
                    entity->selectionAnimator.SetSpriteIfDifferent( selectionSprite, false );

                    if ( teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( blueSprite, false );
                        entity->spriteBank.Add( blueSprite );
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( redSprite, false );
                        entity->spriteBank.Add( redSprite );
                    }

                    SpriteResource * mainSprite = teamNumber.value == 1 ? blueSprite : redSprite;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );

                    entity->isSelectable = true;
                    entity->selectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->selectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 32 ) );

                    // @SPEED:
                    simAction_ActiveEntities.Clear( false );
                    entityPool.GatherActiveObjs( simAction_ActiveEntities );
                    for( i32 starIndex = 0; starIndex < simAction_ActiveEntities.GetCount(); starIndex++ ) {
                        if( simAction_ActiveEntities[ starIndex ]->type == EntityType::STAR ) {
                            fp2 dir = simAction_ActiveEntities[ starIndex ]->pos - pos;
                            entity->ori = FpATan2( dir.x, dir.y );

                            // @SPEED:
                            const fp dist = FpLength( dir );
                            const fp minDist = Fp( 600 ); // @NOTE: Based of start 500
                            const fp maxDist = Fp( 1250 );
                            const fp t = FpClamp( dist, minDist, maxDist );
                            // Make 0 - 1
                            const fp t01 = ( t - minDist ) / ( maxDist - minDist );
                            entity->building.giveEnergyAmount = (i32)( Fp( 1 ) + Fp( 4 ) * FpCos( t01 ) );
                            entity->building.isBuilding = true;
                            entity->building.timeToBuildTurns = SecondsToTurns( 30 );
                            break;
                        }
                    }
                } break;
                case EntityType::BUILDING_COMPUTE: 
                {
                    static SpriteResource * blueSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_blue_off.png", 1, 48, 48, 0 );
                    static SpriteResource * blueSpriteOn =  core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_blue_on.png", 1, 48, 48, 0 );
                    static SpriteResource * redSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_red_off.png", 1, 48, 48, 0 );
                    static SpriteResource * redSpriteOn  = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_red_on.png", 1, 48, 48, 0 );

                    static SpriteResource * selectionSprite = core->ResourceGetAndCreateSprite( "res/ents/test/ship_selected.png", 1, 48, 48, 0 );
                    entity->selectionAnimator.SetSpriteIfDifferent( selectionSprite, false );

                    if ( teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( blueSpriteOff, false );
                        entity->spriteBank.Add( blueSpriteOff );
                        entity->spriteBank.Add( blueSpriteOn );
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( redSpriteOff, false );
                        entity->spriteBank.Add( redSpriteOff );
                        entity->spriteBank.Add( redSpriteOn );
                    }

                    entity->isSelectable = true;
                    entity->selectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->selectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 64 ) );

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 40 );
                }
            }
        }

        return entity;
    }

    void SimMap::DestroyEntity( SimEntity * entity ) {
        if( entity != nullptr ) {
            entityPool.Remove( entity->handle );
        }
    }

    void SimMap::SimAction_SpawnEntity( i32 * typePtr, PlayerNumber * playerNumberPtr, TeamNumber * teamNumberPtr, fp2 * posPtr, fp * oriPtr, fp2 * velPtr ) {
        EntityType type = EntityType::Make( (EntityType::_enumerated)( * typePtr) );
        PlayerNumber playerNumber = *playerNumberPtr;
        TeamNumber teamNumber = *teamNumberPtr;
        fp ori = *oriPtr;
        fp2 pos = *posPtr;
        fp2 vel = *velPtr;
        SpawnEntity( type, playerNumber, teamNumber, pos, ori, vel );
    }

    void SimMap::SimAction_DestroyEntity( EntityHandle * handlePtr ) {
        EntityHandle handle = *handlePtr;
        SimEntity * ent = entityPool.Get( handle );
        DestroyEntity( ent );
    }

    void SimMap::SimAction_PlayerSelect( PlayerNumber * playerNumberPtr, EntHandleList * selection, EntitySelectionChange * changePtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        EntitySelectionChange change = *changePtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        if( change == EntitySelectionChange::SET ) {
            const i32 entityCount = simAction_ActiveEntities.GetCount();
            for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
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

        VisAction_PlayerSelect( playerNumberPtr, selection, change );
    }

    void SimMap::SimAction_Move( PlayerNumber * playerNumberPtr, fp2 * posPtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        fp2 pos = *posPtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        const i32 entityCount = simAction_ActiveEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && IsUnitType( ent->type ) && ent->selectedBy.Contains( playerNumber ) ) {
                ent->unit.command.type = UnitCommandType::MOVE;
                ent->unit.command.targetPos = pos;
                ent->unit.command.targetEnt = EntityHandle::INVALID;
            }
        }
    }

    void SimMap::SimAction_Attack( PlayerNumber * playerNumberPtr, EntityHandle * targetPtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        EntityHandle target = *targetPtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        SimEntity * targetEnt = entityPool.Get( target );
        if( targetEnt == nullptr ) {
            return;
        }

        const i32 entityCount = simAction_ActiveEntities.GetCount();
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
            if( ent->playerNumber == playerNumber && ent->selectedBy.Contains( playerNumber ) ) {
                ent->navigator.hasDest = true;
                fp2 d = ( ent->pos - targetEnt->pos );
                fp dist = FpLength( d );

                if( dist > ent->unit.averageRange - Fp( 10 ) ) {
                     fp2 dir = d / dist;
                     ent->navigator.dest = targetEnt->pos + dir * ent->unit.averageRange; // @HACK: The 10 is because firing range is from center ent so move in more forward for cannons at the back
                 } else {
                     ent->navigator.dest = ent->pos;
                 }

                ent->navigator.slowRad = Fp( 100 );
            }
        }
    }

    void SimMap::SimAction_ContructBuilding( PlayerNumber * playerNumberPtr, i32 * typePtr, fp2 * posPtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        EntityType type = EntityType::Make( (EntityType::_enumerated)( * typePtr) );
        fp2 pos = *posPtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        const i32 entityCount = simAction_ActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && ent->type == EntityType::UNIT_WORKER && ent->selectedBy.Contains( playerNumber ) ) {
                SimEntity * structure = SpawnEntity( type, playerNumber, ent->teamNumber, pos, Fp( 0 ), Fp2( 0, 0 ) );
                ent->unit.command.type = UnitCommandType::CONTRUCT_BUILDING;
                ent->unit.command.targetEnt = structure->handle;
                break;
            }
        }
    }

    void SimMap::SimAction_ContructExistingBuilding( PlayerNumber * playerNumberPtr, EntityHandle * targetPtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        const EntityHandle target = *targetPtr;
        
        SimEntity * targetEnt = entityPool.Get( target );

        if ( targetEnt != nullptr ) {

            // @SPEED:
            simAction_ActiveEntities.Clear( false );
            entityPool.GatherActiveObjs( simAction_ActiveEntities );

            Assert( IsBuildingType( targetEnt->type ) == true );
            if ( targetEnt->building.isBuilding == true ) {
                const i32 entityCount = simAction_ActiveEntities.GetCount();
                for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                    SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
                    if ( ent->playerNumber == playerNumber && ent->type == EntityType::UNIT_WORKER && ent->selectedBy.Contains( playerNumber ) ) {
                        ent->unit.command.type = UnitCommandType::CONTRUCT_BUILDING;
                        ent->unit.command.targetEnt = targetEnt->handle;
                        break;
                    }
                }
            }
        }
    }

    void SimMap::SimAction_PlanetPlacePlacement( PlayerNumber * playerNumberPtr, i32 * placementIndexPtr, PlanetPlacementType * placementTypePtr ) {
        const PlayerNumber playerNumber = *playerNumberPtr;
        const i32 placementIndex = *placementIndexPtr;
        const PlanetPlacementType placementType = *placementTypePtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        simAction_EntityFilter.Begin( &simAction_ActiveEntities )->
            SelectedBy( playerNumber )->
            OwnedBy( playerNumber )->
            Type( EntityType::PLANET )->
            End();

        const i32 count = simAction_EntityFilter.result.GetCount();
        for ( i32 entityIndex = 0; entityIndex < count; entityIndex++ ) {
            SimEntity * ent = *simAction_EntityFilter.result.Get( entityIndex );
            if ( ent->type == EntityType::PLANET ) {
                Planet & planet = ent->planet;
                planet.placements[ placementIndex ] = placementType;
                break;
            }
        }
    }

    void SimMap::SimAction_BuildingTrainUnit( PlayerNumber * playerNumberPtr , i32 * typePtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        EntityType type = EntityType::Make( (EntityType::_enumerated)( * typePtr) );

        Assert( IsUnitType( type ) == true );

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        simAction_EntityFilter.Begin( &simAction_ActiveEntities )->
            SelectedBy( playerNumber )->
            OwnedBy( playerNumber )->
            Type( EntityType::BUILDING_STATION )->
            End();

        const i32 count = simAction_EntityFilter.result.GetCount();
        for ( i32 entityIndex = 0; entityIndex < count; entityIndex++ ) {
            SimEntity * ent = *simAction_EntityFilter.result.Get( entityIndex );
            Assert( ent->type == EntityType::BUILDING_STATION );

            Building & building = ent->building;

            // @HACK:
            // @TODO: We should have a queue here.
            if ( building.isTraining == false ) {
                building.turn = 0;
                building.isTraining = true;
                building.trainingEnt = type;
                building.timeToTrainTurns = type == EntityType::UNIT_WORKER ? SecondsToTurns( 12 ) :SecondsToTurns( 38 ) ;
            }
        }
    }

    void SimMap::SimAction_ApplyDamage( i32 * damagePtr, EntityHandle * targetPtr ) {
        const i32 damage = *damagePtr;
        const EntityHandle target = *targetPtr;
        SimEntity * targetEnt = entityPool.Get( target );
        if( targetEnt != nullptr ) {
            targetEnt->unit.currentHealth -= damage;
            if( targetEnt->unit.currentHealth <= 0 ) {
                targetEnt->unit.currentHealth = 0;
            }
        }
        //core->LogOutput( LogLevel::INFO, "SimAction_ApplyDamage: Applying damage %d to %d, %d", damage, target.idx, target.gen );
    }

    void SimMap::SimAction_ApplyContruction( EntityHandle * targetPtr ) {
        const EntityHandle target = *targetPtr;
        SimEntity * targetEnt = entityPool.Get( target );
        if ( targetEnt != nullptr ) {
            Assert( IsBuildingType( targetEnt->type ) == true );
            if ( targetEnt->building.isBuilding == true ) {
                targetEnt->building.turn++;
                if ( targetEnt->building.turn >= targetEnt->building.timeToBuildTurns ) {
                    targetEnt->building.turn = 0;
                    targetEnt->building.isBuilding = false;
                }
            }
        }
    }

    void SimMap::SimAction_GiveCredits( PlayerNumber * playerNumberPtr, i32 * amountPtr ) {
        const PlayerNumber playerNumber = *playerNumberPtr;
        const i32 amount = *amountPtr;
        Assert( playerNumber.value != 0 );
        playerMonies[ playerNumber.value - 1 ].credits += amount;
    }

    void SimMap::SimAction_GiveEnergy( PlayerNumber * playerNumberPtr, i32 * amountPtr ) {
        const PlayerNumber playerNumber = *playerNumberPtr;
        const i32 amount = *amountPtr;
        Assert( playerNumber.value != 0 );
        playerMonies[ playerNumber.value - 1 ].energy += amount;
    }

    void SimMap::SimAction_GiveCompute( PlayerNumber * playerNumberPtr, i32 * amountPtr ) {
        const PlayerNumber playerNumber = *playerNumberPtr;
        const i32 amount = *amountPtr;
        Assert( playerNumber.value != 0 );
        playerMonies[ playerNumber.value - 1 ].compute += amount;
    }

    static void SimTick_SelfUpdate( const ConstEntList * entities, const EntPool * entityPool, i32 index, SimEntity * ent ) {
        const fp playerSpeed = Fp( 2500.0f / 100.0f );
        const fp maxForce = Fp( 20.0f );
        const fp invMass = Fp( 1.0f / 10.0f );

        ZeroStruct( ent->actions );

        switch( ent->type ) {
            case EntityType::UNIT_WORKER:
            case EntityType::UNIT_TEST:
            {
                Unit & unit = ent->unit;

                if( unit.currentHealth == 0 ) {
                    ent->actions.AddAction( MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                    break;
                }

                if ( unit.command.type == UnitCommandType::IDLE ) {
                }
                else if ( unit.command.type == UnitCommandType::MOVE ) {
                    ent->navigator.hasDest = true;
                    ent->navigator.dest = unit.command.targetPos;
                    ent->navigator.slowRad = Fp( 100 );
                }
                else if ( unit.command.type == UnitCommandType::ATTACK ) {
                }
                else if ( unit.command.type == UnitCommandType::FOLLOW ) {
                }
                else if ( unit.command.type == UnitCommandType::CONTRUCT_BUILDING ) {
                    if ( ent->type == EntityType::UNIT_WORKER ) {
                        const SimEntity * target = entityPool->Get( unit.command.targetEnt );
                        if ( target != nullptr ) {
                            fp dist2 = FpDistance2( target->pos, ent->pos );
                            if ( dist2 < unit.averageRange * unit.averageRange ) {
                                // Build
                                ent->navigator.hasDest = false;
                                ent->actions.AddAction( MapActionType::SIM_ENTITY_APPLY_CONSTRUCTION, target->handle );
                            } else {
                                // Move towards building
                                ent->navigator.hasDest = true;
                                ent->navigator.dest = target->pos;
                                ent->navigator.slowRad = Fp( 100 );
                            }
                        }
                    }
                }

                if( ent->navigator.hasDest == true ) {
                    fp2 targetPos = ent->navigator.dest;
                    fp2 desiredVel = ( targetPos - ent->pos );
                    fp dist = FpLength( desiredVel );

                    if( dist < ent->navigator.slowRad ) {
                        if( dist < Fp(5 )) {
                            desiredVel = Fp2( 0, 0 );
                        }
                        else {
                            desiredVel = FpNormalize( desiredVel ) * playerSpeed * ( dist / ent->navigator.slowRad );
                        }
                    }
                    else {
                        desiredVel = FpNormalize( desiredVel ) * playerSpeed;
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

                    fp2 steering = desiredVel - ent->vel;

                    // Avoidance
                    for ( i32 entityIndexB = 0; entityIndexB < entities->GetCount(); entityIndexB++ ) {
                        const SimEntity * otherEnt = *entities->Get( entityIndexB );
                        const bool correctType = IsUnitType( otherEnt->type ) == true || IsBuildingType( otherEnt->type );
                        if ( entityIndexB != index && correctType && otherEnt->teamNumber == ent->teamNumber ) {
                            fp dist = FpDistance( otherEnt->pos, ent->pos );
                            if( dist < Fp( 100 ) && dist > Fp( 1 ) ) {
                                fp2 dir = ( ent->pos - otherEnt->pos ) / dist;
                                //steering += dir * playerSpeed * 0.2f;
                                steering = steering + dir * playerSpeed * FpClamp( ( 1 - dist / Fp( 50 ) ), Fp( 0 ), Fp( 1 ) );
                            }
                        }
                    }

                    steering = FpTruncateLength( steering, maxForce );
                    steering = steering * invMass;

                    ent->vel = FpTruncateLength( ent->vel + steering, playerSpeed );
                } else {
                    ent->vel = ent->vel * Fp( 0.95f );
                    if( ent->vel != Fp2( 0, 0 ) && FpLength( ent->vel ) < Fp( 1 ) ) {
                        //ent->vel = glm::vec2( 0 );
                    }
                }

                // Update orientation
                if( FpLength2( ent->vel ) >= Fp( 1 ) ) {
                    fp2 nvel = FpNormalize( ent->vel );
                    ent->ori = FpATan2( nvel.x, nvel.y );
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

                const i32 turretCount = unit.turrets.GetCount();
                for( i32 turretIndex = 0; turretIndex < turretCount; turretIndex++ ) {
                    UnitTurret & turret = unit.turrets[ turretIndex ];
                    fp2 worldPos = ent->pos + FpRotate( turret.posOffset, -ent->ori );

                    turret.fireTimer -= SIM_DT;
                    if( turret.fireTimer < Fp(0) ) {
                        turret.fireTimer = Fp( 0 );
                    }

                    bool hasTarget = false;
                    for( i32 entityIndexB = 0; entityIndexB < entities->GetCount(); entityIndexB++ ) {
                        if( entityIndexB == index ) {
                            continue;
                        }

                        const SimEntity * otherEnt = *entities->Get( entityIndexB );
                        if( IsUnitType( otherEnt->type ) == false ) {
                            continue;
                        }

                        if( otherEnt->teamNumber == ent->teamNumber ) {
                            continue;
                        }

                        fp dist2 = FpDistance2( worldPos, otherEnt->pos );
                        if( dist2 <= turret.fireRange * turret.fireRange ) {
                            fp2 dir = FpNormalize( otherEnt->pos - worldPos );
                            turret.ori = FpATan2( dir.x, dir.y );
                            hasTarget = true;

                            if( turret.fireTimer == Fp( 0 ) ) {
                                turret.fireTimer = turret.fireRate;
                                EntityType btype = turret.size == WeaponSize::SMALL ? EntityType::Make( EntityType::BULLET_SMOL ) : EntityType::Make( EntityType::BULLET_MED );
                                fp2 spawnPos = turret.size == WeaponSize::SMALL ? worldPos : worldPos + dir * Fp( 10 );
                                ent->actions.AddAction( MapActionType::SIM_ENTITY_SPAWN, (i32)btype, ent->playerNumber, ent->teamNumber, spawnPos, turret.ori, dir * Fp( 250 ) );
                                break;
                            }
                        }
                    }

                    if( hasTarget == false ) {
                        turret.ori = ent->ori;
                    }
                }
            } break;
            case EntityType::BULLET_SMOL: // Fall 
            case EntityType::BULLET_MED:
            { 
                Bullet & bullet = ent->bullet;
                bullet.aliveTimer += SIM_DT;
                if( bullet.aliveTimer > bullet.aliveTime ) {
                    ent->actions.AddAction( MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                    break;
                }

                for( i32 entityIndexB = 0; entityIndexB < entities->GetCount(); entityIndexB++ ) {
                    if( entityIndexB == index ) {
                        continue;
                    }

                    const SimEntity * otherEnt = *entities->Get( entityIndexB );
                    if( IsUnitType( otherEnt->type ) == false ) {
                        continue;
                    }

                    if( otherEnt->teamNumber == ent->teamNumber ) {
                        continue;
                    }

                    fp dist2 = FpDistance2( ent->pos, otherEnt->pos );
                    fp r = ent->handle.idx * Fp( 10 );
                    if( dist2 <= Fp( 36 * 36 ) + r ) {
                        bool changed = ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteBank[1], false );
                        ent->vel = Fp2( 0, 0 );
                        if( changed == true ) {
                            ent->actions.AddAction( MapActionType::SIM_ENTITY_APPLY_DAMAGE, bullet.damage, otherEnt->handle );
                            ent->actions.AddAction( MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                        }
                        break;
                    }
                }
            } break;
            case EntityType::PLANET:
            {
                Planet & planet = ent->planet;
                planet.turn++;
                const i32 placementCount = planet.placements.GetCapcity();
                for ( i32 placementIndex = 0; placementIndex < placementCount; placementIndex++ ) {
                    const PlanetPlacementType & placementType = planet.placements[ placementIndex ];
                    switch ( placementType ) {
                        case PlanetPlacementType::INVALID: {} break;
                        case PlanetPlacementType::BLOCKED: {} break;
                        case PlanetPlacementType::OPEN: {} break;
                        case PlanetPlacementType::CREDIT_GENERATOR: {
                            if ( planet.turn % 30 == 0 ) {
                                ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_CREDITS, ent->playerNumber, 3 );
                            }
                        } break;
                        case PlanetPlacementType::ENERGY_GENERATOR: {
                            if ( planet.turn % 40 == 0) {
                                ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_ENERGY, ent->playerNumber, 3 );
                            }
                        } break;
                        case PlanetPlacementType::COMPUTE_GENERATOR: {
                            if ( planet.turn % 60 == 0) {
                                ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE, ent->playerNumber, 2 );
                            }
                        } break;
                    }
                }
            } break;
            case EntityType::BUILDING_STATION:
            case EntityType::BUILDING_SOLAR_ARRAY:
            case EntityType::BUILDING_COMPUTE:
            {
                Building & building = ent->building;

                if ( building.isBuilding == false ) {
                    building.turn++;
                    if ( ent->type == EntityType::BUILDING_SOLAR_ARRAY ) {
                        if ( building.turn == 60 ) {
                            building.turn = 0;
                            ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_ENERGY, ent->playerNumber, ent->building.giveEnergyAmount );
                        }
                    }
                    else if ( ent->type == EntityType::BUILDING_STATION ) {
                        if ( building.isTraining == true && building.turn == building.timeToTrainTurns ) {
                            fp2 p = ent->pos - Fp2( 0, 32 ); // @HACK
                            fp o = FP_PI; // @HACK
                            ent->actions.AddAction( MapActionType::SIM_ENTITY_SPAWN, ( i32 )building.trainingEnt, ent->playerNumber, ent->teamNumber, p, o, Fp2( 0, 0 ) );
                            building.turn = 0;
                            building.isTraining = false;
                            building.trainingEnt = EntityType::INVALID;
                            building.timeToTrainTurns = 0;
                        }
                    }
                }
            } break;
        }
        //ent->acc.x -= ent->vel.x * ent->resistance;
        //ent->acc.y -= ent->vel.y * ent->resistance;
        ent->vel = ent->vel + ent->acc * SIM_DT;
        ent->pos = ent->pos + ent->vel * SIM_DT;
    }
    
    struct SimMap_UpdateTaskContext {
        i32                                         startIndex = 0;
        i32                                         endIndex = 0;
        ConstEntList *                              entities = nullptr;
        EntCacheList *                              entityCache = nullptr;
        const EntPool *                             entityPool = nullptr;
    };

    class SimMap_UpdateTask : public enki::ITaskSet {
    public:
        SimMap_UpdateTaskContext ctx;
        SimMap_UpdateTask() {}
        
        SimMap_UpdateTask( const SimMap_UpdateTask & other ) {
            this->ctx = other.ctx;
        };
        
        SimMap_UpdateTask( SimMap_UpdateTaskContext c ) {
            this->ctx = c;
        }

        virtual void ExecuteRange( enki::TaskSetPartition range_, uint32_t threadnum_ ) override {
            for( i32 entityIndex = ctx.startIndex; entityIndex < ctx.endIndex; entityIndex++ ) {
                const SimEntity * ent = *ctx.entities->Get( entityIndex );
                SimEntity * cachedEnt = ctx.entityCache->Set_MemCpyPtr( entityIndex, ent );
                SimTick_SelfUpdate( ctx.entities, ctx.entityPool, entityIndex, cachedEnt );
            }
        }
    };

    void SimMap::SimTick( MapTurn * turn1, MapTurn * turn2 ) {
        //ScopedClock timer( "SimTick", core );

        if ( turn2 != nullptr && turn1->checkSum != turn2->checkSum ) {
            core->LogOutput( LogLevel::INFO, "We have a desync :(" );

            INVALID_CODE_PATH;
        }

        mapReplay.NextTurn( turnNumber );

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

    #if 1
        const i32 entityCount = entities->GetCount();
        const i32 threadCount = (i32)core->taskScheduler.GetNumTaskThreads();
        const i32 partitonCount = ( entityCount + threadCount - 1 ) / threadCount;

        std::vector<SimMap_UpdateTask> updateTasks; // @SPEED: This is a bit of a waste...
        updateTasks.reserve( threadCount );
        for( i32 threadIndex = 0; threadIndex < threadCount; threadIndex++ ) {
            i32 start = threadIndex * partitonCount;
            i32 end = glm::min( start + partitonCount, entityCount );

            SimMap_UpdateTaskContext context = {};
            context.startIndex = start;
            context.endIndex = end;
            context.entities = entities;
            context.entityCache = entityCache;
            context.entityPool = &entityPool;

            updateTasks.emplace_back( context );
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

        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * cachedEnt = entityCache->Get( entityIndex );
            SimEntity * readEnt = const_cast< SimEntity * > ( *entities->Get( entityIndex ) ); // @NOTE: Being very sneaky here by const casting away the const.
            *readEnt = *cachedEnt;
        }

        // Apply map actions
        for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = entityCache->Get( entityIndex );
            if( ent->actions.data.GetSize() != 0 ) {
                Sim_ApplyActions( &ent->actions );
            }
        }

        turnNumber++;
    }

    void SimMap::Sim_ApplyActions( MapActionBuffer * actionBuffer ) {
        char * turnData = actionBuffer->data.GetBuffer();
        const i32 turnSize = actionBuffer->data.GetSize();

        mapReplay.AddActionData( actionBuffer );

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

    i64 SimMap::Sim_CheckSum() {
        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        i64 checkSum = 0;

        const i32 entityCount = simAction_ActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
            #if 0
            checkSum += ent->handle.idx;
            checkSum -= ent->handle.gen;
            #endif
            checkSum += ToInt( ent->pos.x );
            checkSum -= ToInt( ent->pos.y );
        }

        const i32 playerCount = playerNumbers.GetCount();
        for ( i32 playerIndex = 0; playerIndex < playerCount; playerIndex++ ) {
            checkSum += playerMonies[playerIndex].credits;
            checkSum -= playerMonies[playerIndex].energy;
            checkSum -= playerMonies[playerIndex].compute;
        }

        return checkSum;
    }

    void SimMap::VisAction_PlayerSelect( PlayerNumber * playerNumberPtr, EntHandleList * selection, EntitySelectionChange change ) {
        PlayerNumber playerNumber = *playerNumberPtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        if( change == EntitySelectionChange::SET ) {
            const i32 entityCount = simAction_ActiveEntities.GetCount();
            for( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
                ent->visSelectedBy.RemoveValue( playerNumber );
            }
        }

        const i32 selectionCount = selection->GetCount();
        if( change == EntitySelectionChange::SET || change == EntitySelectionChange::ADD ) {
            for( i32 entityHandleIndex = 0; entityHandleIndex < selectionCount; entityHandleIndex++ ) {
                EntityHandle handle = *selection->Get( entityHandleIndex );
                SimEntity * ent = entityPool.Get( handle );
                if( ent != nullptr ) {
                    ent->visSelectedBy.Add( playerNumber );
                }
            }
        }

        if( change == EntitySelectionChange::REMOVE ) {
            for( i32 entityHandleIndex = 0; entityHandleIndex < selectionCount; entityHandleIndex++ ) {
                EntityHandle handle = *selection->Get( entityHandleIndex );
                SimEntity * ent = entityPool.Get( handle );
                if( ent != nullptr ) {
                    ent->visSelectedBy.RemoveValue( playerNumber );
                }
            }
        }
    }

    inline static Collider2D ColliderForSpace( const Collider2D & base, glm::vec2 p ) {
        Collider2D c = base;
        switch( base.type ) {
            case COLLIDER_TYPE_CIRCLE:
            {
                c.circle.pos += p;
            } break;
            case COLLIDER_TYPE_AXIS_BOX:
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
        return ColliderForSpace( collisionCollider, ToVec2(pos) );//@HACK: FPM
    }

    Collider2D SimEntity::GetWorldSelectionCollider() const {
        return ColliderForSpace( selectionCollider, ToVec2( pos )); //@HACK: FPM
    }

    void SimMapReplay::Prepare() {
        turns.reserve( 1024 );
        actionData.Reserve( Megabytes( 64 ) );
    }

    void SimMapReplay::NextTurn( i32 turn ) {
        TurnAction turnAction = {};
        turnAction.turnNumber = turn;
        turnAction.actionCount = 0;
        turns.push_back( turnAction );
    }

    void SimMapReplay::AddActionData( MapActionBuffer * actionBuffer ) {
        char * data = actionBuffer->data.GetBuffer();
        size_t size = actionBuffer->data.GetSize();
        if( size != 0 ) {
            actionData.Write( data, size );
            turns.back().actionCount++;
        }
    }

    void SimMapReplay::PrintActions( Core * core ) {
        core->LogOutput( LogLevel::INFO, "===========================================================" );
        
        size_t actionOffset = 0;
        const size_t turnCount = turns.size();
        for ( size_t turnIndex = 0; turnIndex < turnCount; turnIndex++ ) {
            const TurnAction &turn = turns[turnIndex];

            bool displayed = false;
            for ( size_t actionCounter = 0; actionCounter < turn.actionCount; actionCounter++ ) {
                MapActionType actionType = ( MapActionType )actionData.buffer.at( actionOffset );
                if( actionType == MapActionType::NONE ) {
                    core->LogOutput( LogLevel::ERR, "Can't apply a none turn" );
                    break;
                }

                if( displayed == false ) {
                    displayed = true;
                    core->LogOutput( LogLevel::INFO, "=============%d=============", turn.turnNumber );
                }
                
                actionOffset += sizeof( MapActionType );

                byte * data = &actionData.buffer.at( actionOffset );
                
                RpcHolder * holder = rpcTable[ (i32)actionType ];
                LargeString rpcString = holder->Log( (char *)data );
                i32 lastCallSize = holder->GetLastCallSize();
                actionOffset += lastCallSize;

                const char * actionName = MapActionTypeStrings[ (i32)actionType ];
                core->LogOutput( LogLevel::INFO, "%s :: ( %s )", actionName, rpcString.GetCStr() );
            }
        }
    }
}


