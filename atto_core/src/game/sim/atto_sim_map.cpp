#include "atto_sim_map.h"

#include "../../shared/atto_rpc.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static FixedList<glm::vec2, 5 * 3> ui_LeftPanelCenters;
    static FixedList<glm::vec2, 5 * 3> ui_RightPanelCenters;

    static AudioResource * sndLaser1[23] = {};
    static AudioResource * sndLaser2[6] = {};

    static AudioResource * sndBuildBuilding = nullptr;
    static AudioResource * sndRogerRoger = nullptr;
    static AudioResource * sndNotEnoughResources = nullptr;

    static MoneySet costOfPlacementCredit = { 20, 50, 0 };
    static MoneySet costOfPlacementSolar = { 50, 0, 0 };
    static MoneySet costOfPlacementCompute = { 100, 50, 0 };

    static MoneySet costOfBuildingStation = { 150, 0, 0 };
    static MoneySet costOfBuildingSolar = { 100, 0, 0 };
    static MoneySet costOfBuildingCompute = { 100, 100, 0 };

    static MoneySet costOfWorker = { 50, 10, 50 };
    static MoneySet costOfFighter = { 100, 50, 100 };

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

    EntityListFilter * EntityListFilter::SimSelectedBy( PlayerNumber playerNumber ) {
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

    EntityListFilter * EntityListFilter::VisSelectedBy( PlayerNumber playerNumber ) {
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                SimEntity * ent = *activeEntities->Get( entityIndex );
                if ( ent->visSelectedBy.Contains( playerNumber ) == false ) {
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

        LoadAllAssets( core );

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
            sndLaser1[ i ]->maxInstances = 4;
            sndLaser1[ i ]->stealMode = AudioStealMode::OLDEST;
        }

        for ( i32 i = 0; i < 6; i++ ) {
            LargeString name = StringFormat::Large( "res/sounds/laser_2/sci-fi_weapon_plasma_pistol_0%d.wav", i + 1 );
            sndLaser2[ i ] = core->ResourceGetAndCreateAudio( name.GetCStr(), true, false, 4000.0f, 10000.0f );
            sndLaser2[ i ]->maxInstances = 4;
            sndLaser2[ i ]->stealMode = AudioStealMode::OLDEST;
        }

        sndBuildBuilding = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/protoss-electric.mp3", true, false, 0, 0 );
        sndBuildBuilding->maxInstances = 3;
        sndBuildBuilding->stealMode = AudioStealMode::OLDEST;

        sndRogerRoger = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starwars/rogerroger.mp3", true, false, 0, 0 );
        sndNotEnoughResources = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/vespene.mp3", true, false, 0, 0 );

        viewSolarNumber = SolarNumber::Create( 1 );

        playerNumbers.Add( PlayerNumber::Create( 1 ) );
        playerNumbers.Add( PlayerNumber::Create( 2 ) );

        for ( i32 i = 0; i < playerNumbers.GetCount(); i++ ) {
            //playerMonies[i].credits = 500;
            //playerMonies[i].energy = 100;
            //playerMonies[i].compute = 50;
            playerMonies[i].credits = 10000;
            playerMonies[i].energy = 10000;
            playerMonies[i].compute = 10000;
        }

        PlayerNumber p0 = PlayerNumber::Create( 0 );
        TeamNumber t0 = TeamNumber::Create( 0 );
        PlayerNumber p1 = PlayerNumber::Create( 1 );
        TeamNumber t1 = TeamNumber::Create( 1 );
        PlayerNumber p2 = PlayerNumber::Create( 2 );
        TeamNumber t2 = TeamNumber::Create( 2 );
        SolarNumber s1 = SolarNumber::Create( 1 );
        SolarNumber s2 = SolarNumber::Create( 2 );

        SimEntitySpawnInfo solarSystemSpawnInfo = {};
        solarSystemSpawnInfo.type = EntityType::Make( EntityType::SOLAR_SYSTEM );
        solarSystemSpawnInfo.playerNumber= p1;
        solarSystemSpawnInfo.teamNumber = t1;
        solarSystemSpawnInfo.solarNumber = s1;
        solarSystemSpawnInfo.pos = Fp2( 100.0f, 360.0f / 2.0f );
        solarSystemSpawnInfo.solarSystem.name = "TEXAS BBQ MEAL";

        SimEntity * solarSystem16CM = SpawnEntity( &solarSystemSpawnInfo );
        solarSystemSpawnInfo.solarSystem.name = "BIG G";
        solarSystemSpawnInfo.solarSystem.connections.AddUnique( solarSystem16CM->handle );
        solarSystemSpawnInfo.pos = Fp2( 500.0f, 360.0f / 2.0f );
        solarSystemSpawnInfo.solarNumber = s2;
        SpawnEntity( &solarSystemSpawnInfo );

        SpawnEntity( EntityType::Make( EntityType::PLANET ), p1, t1, s1, Fp2( 500.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_WORKER ), p1, t1, s1, Fp2( 700.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_SCOUT ), p1, t1, s1, Fp2( 800.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p1, t1, s1, Fp2( 900.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_FIGHTER ), p1, t1, s1, Fp2( 1000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_FIGHTER ), p1, t1, s1, Fp2( 900.0f, 800.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_FIGHTER ), p1, t1, s1, Fp2( 900.0f, 900.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_FIGHTER ), p1, t1, s1, Fp2( 900.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        
        SpawnEntity( EntityType::Make( EntityType::PLANET ), p2, t2, s1, Fp2( 2500.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_WORKER ), p2, t2, s1, Fp2( 2200.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        
        SpawnEntity( EntityType::Make( EntityType::STAR ), p0, t0, s1, Fp2( 1500.0f, 1200.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        SpawnEntity( EntityType::Make( EntityType::BUILDING_STATION ), p1, t1, s1, Fp2( 500.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) )->building.isBuilding = false;
        SpawnEntity( EntityType::Make( EntityType::BUILDING_COMPUTE ), p2, t2, s1, Fp2( 600.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) )->building.isBuilding = false;

        //SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, s1, Fp2( 800.2f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        //SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, s1, Fp2( 900.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        //SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, s1, Fp2( 1000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        //SpawnEntity( EntityType::Make( EntityType::UNIT_TEST ), p1, t1, s1, Fp2( 1100.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_SCOUT ), p2, t2, s1, Fp2( 2300.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p2, t2, s1, Fp2( 2200.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p2, t2, s1, Fp2( 2100.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p2, t2, s1, Fp2( 2000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        playSpawningSounds = true;
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
        static TextureResource * background = core->ResourceGetAndLoadTexture( "res/game/backgrounds/light_blue.png" );
        static TextureResource * sprTurretSmol = core->ResourceGetAndLoadTexture( "res/ents/test/turret_smol.png" );
        static TextureResource * sprTurretMed = core->ResourceGetAndLoadTexture( "res/ents/test/turret_med.png" );
        static TextureResource * sprUIMock = core->ResourceGetAndLoadTexture( "res/ents/test/ui_mock_01.png" );
        static TextureResource * sprMoveLocation = core->ResourceGetAndLoadTexture( "res/ents/test/move_location.png" );

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, true );
        //DrawContext * backgroundDrawContext = core->RenderGetDrawContext( 1, true );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 2, true );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 3, true );

        if ( viewMode == ViewMode::SOLAR ) {
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
                glm::vec2( 320, 180 ),
                glm::vec2( 640, 360 ),
                glm::vec2( 960, 540 ),
                glm::vec2( 1280, 720 ),
                glm::vec2( 1600, 900 ),
                glm::vec2( 1920, 1080 ),
            };

            if ( zoomDelta > 0 ) {
                localCameraZoomIndex--;
            }
            else if ( zoomDelta < 0 ) {
                localCameraZoomIndex++;
            }

            localCameraZoomIndex = glm::clamp( localCameraZoomIndex, 0, 4 );

            glm::vec2 wantedResolution = resolutions[ localCameraZoomIndex ];
            glm::vec2 oldCameraWidth = localCameraZoomLerp;
            localCameraZoomLerp = glm::mix( localCameraZoomLerp, wantedResolution, dt * 10.f );
            if ( glm::abs( localCameraZoomLerp.x - wantedResolution.x ) < 5 ) {
                localCameraZoomLerp = wantedResolution;
            }

            localCameraPos -= ( localCameraZoomLerp - oldCameraWidth ) * 0.5f;

            spriteDrawContext->SetCameraDims( localCameraZoomLerp.x, localCameraZoomLerp.y );
            uiDrawContext->SetCameraDims( 640, 360 );
            debugDrawContext->SetCameraDims( localCameraZoomLerp.x, localCameraZoomLerp.y );

            const glm::vec2 mapMin = glm::vec2( 0.0f );
            const glm::vec2 mapMax = glm::vec2( ( f32 )MAX_MAP_SIZE ) - glm::vec2( spriteDrawContext->GetCameraWidth(), spriteDrawContext->GetCameraHeight() );
            localCameraPos = glm::clamp( localCameraPos, mapMin, mapMax );

            spriteDrawContext->SetCameraPos( localCameraPos );
            debugDrawContext->SetCameraPos( localCameraPos );

            const glm::vec2 localCameraCenter = localCameraPos + glm::vec2( spriteDrawContext->GetCameraWidth(), spriteDrawContext->GetCameraHeight() ) / 2.0f;
            core->AudioSetListener( localCameraCenter );

            spriteDrawContext->DrawTextureBL( background, glm::vec2( 0, 0 ) );
        } else if ( viewMode == ViewMode::GALAXY )  {
            spriteDrawContext->SetCameraDims( 640, 360 );
            uiDrawContext->SetCameraDims( 640, 360 );
            debugDrawContext->SetCameraDims( 640, 360 );
            spriteDrawContext->SetCameraPos( glm::vec2( 0 ) );
            debugDrawContext->SetCameraPos( glm::vec2( 0 ) );
        }

        if ( core->InputKeyJustPressed( KEY_CODE_F1 ) ) {
            viewMode = ViewMode::GALAXY;
        }
        if ( core->InputKeyJustPressed( KEY_CODE_F2 ) ) {
            viewMode = ViewMode::SOLAR;
        }

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
            VisSelectedBy( localPlayerNumber )->
            End()->
            ContainsOnlyOneOfType( EntityType::PLANET );

        const bool onlyWorkerSelected = entityFilter->Begin( &entities )->
            OwnedBy( localPlayerNumber )->
            VisSelectedBy( localPlayerNumber )->
            End()->
            ContainsOnlyType( EntityType::UNIT_KLAED_WORKER );


        const bool onlyBuildingSelected = entityFilter->Begin( &entities )->
            OwnedBy( localPlayerNumber )->
            VisSelectedBy( localPlayerNumber )->
            End()->
            ContainsOnlyType( EntityType::BUILDING_STATION );

        SimEntity * onlyBuildingSelectedEnt = nullptr;  
        if ( onlyBuildingSelected == true ) {
            onlyBuildingSelectedEnt = *entityFilter->result.Get( 0 );
        }

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
            } else { // @NOTE: Submenu
                const glm::vec2 s = glm::vec2( 15 );
                if ( gameUI.ButtonPix( 145, "", ui_LeftPanelCenters[0], s, Colors::GREEN ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfPlacementCredit ) == true ) {
                        localActionBuffer.AddAction( MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                    localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::CREDIT_GENERATOR );
                        planetPlacementSubMenu = false;
                        planetPlacementSubMenuIndex = -1;
                    }
                }
                if ( gameUI.ButtonPix( 146, "", ui_LeftPanelCenters[1], s, Colors::GOLD ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfPlacementSolar ) == true ) {
                        localActionBuffer.AddAction( MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                    localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::ENERGY_GENERATOR );
                        planetPlacementSubMenu = false;
                        planetPlacementSubMenuIndex = -1;
                    }
                }
                if ( gameUI.ButtonPix( 147, "", ui_LeftPanelCenters[2], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfPlacementCompute ) == true ) {
                        localActionBuffer.AddAction( MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                    localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::COMPUTE_GENERATOR );
                        planetPlacementSubMenu = false;
                        planetPlacementSubMenuIndex = -1;
                    }
                }
            }
        } else {
            planetPlacementSubMenu = false;
            planetPlacementSubMenuIndex = -1;
        }

        if ( onlyWorkerSelected == true ) {
            SimEntity * ent = entityFilter->result[ 0 ];
            Unit & unit = ent->unit;

            if ( isPlacingBuilding == false ) { // @HACK: NO SIM CHECK, should be more concrete here. I don't think this works in all cases.
                const glm::vec2 s = glm::vec2( 15 );
                MoneySet & localPlayerMoney = playerMonies[ localPlayerNumber.value - 1 ];

                if( core->InputKeyJustPressed( KEY_CODE_Q ) || gameUI.ButtonPix( 232, "Q", ui_RightPanelCenters[ 0 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfBuildingStation ) == true ) {
                        isPlacingBuilding = true;
                        placingBuildingType = EntityType::BUILDING_STATION;
                    }
                }
                if ( core->InputKeyJustPressed( KEY_CODE_E ) || gameUI.ButtonPix( 233, "E", ui_RightPanelCenters[ 1 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfBuildingSolar ) == true ) {
                        isPlacingBuilding = true;
                        placingBuildingType = EntityType::BUILDING_SOLAR_ARRAY;
                    }
                }
                if ( core->InputKeyJustPressed( KEY_CODE_C ) || gameUI.ButtonPix( 234, "C", ui_RightPanelCenters[ 2 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfBuildingCompute ) == true ) {
                        isPlacingBuilding = true;
                        placingBuildingType = EntityType::BUILDING_COMPUTE;
                    }
                }
            }
        }

        if ( onlyBuildingSelected == true ) {
            if ( onlyBuildingSelectedEnt->building.isBuilding == false ) {
                const glm::vec2 s = glm::vec2( 15 );
                if ( core->InputKeyJustPressed( KEY_CODE_Q ) || gameUI.ButtonPix( 242, "Q", ui_RightPanelCenters[ 0 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfWorker ) == true ) {
                        localActionBuffer.AddAction( MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, ( i32 )EntityType::UNIT_KLAED_WORKER );
                        core->AudioPlay( sndRogerRoger );
                    }
                }
                if ( core->InputKeyJustPressed( KEY_CODE_E ) || gameUI.ButtonPix( 243, "E", ui_RightPanelCenters[ 1 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfFighter ) == true ) {
                        localActionBuffer.AddAction( MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, ( i32 )EntityType::UNIT_KLAED_SCOUT );
                        core->AudioPlay( sndRogerRoger );
                    }
                }
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
                    core->AudioPlay( sndBuildBuilding );
                    localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING, localPlayerNumber, ( i32 )placingBuildingType, mousePosWorldFp );
                }
            } else {
                if ( core->InputMouseButtonJustPressed( MOUSE_BUTTON_1 ) == true ) {
                    viewIsDragging = true;
                    viewStartDrag = mousePosWorld;
                    viewEndDrag = mousePosWorld;
                    viewDragSelection.Clear();
                }
            }
        }

        if ( viewIsDragging == true ) {
            viewEndDrag = mousePosWorld;
            if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                viewIsDragging = false;

                bool hasLocalPlayerUnits = false;
                bool hasPlanetType = false;
                bool hasBuildingType = false;
                bool hasUnitType = false;

                // @NOTE: Selection Priority
                i32 selectionCount = viewDragSelection.GetCount();
                for ( i32 selectionIndex = 0; selectionIndex < selectionCount; selectionIndex++ ) {
                    EntityHandle handle = viewDragSelection[ selectionIndex ];
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

                for ( i32 selectionIndex = 0; selectionIndex < viewDragSelection.GetCount(); selectionIndex++ ) {
                    EntityHandle handle = viewDragSelection[ selectionIndex ];
                    SimEntity * selectionEnt = entityPool.Get( handle );
                    if ( selectionEnt != nullptr ) {
                        if ( hasLocalPlayerUnits == true && selectionEnt->playerNumber != localPlayerNumber ) {
                            viewDragSelection.RemoveIndex( selectionIndex );
                            selectionIndex--;
                        }
                        else if ( hasUnitType == true ) {
                            if ( IsBuildingType( selectionEnt->type ) == true || selectionEnt->type == EntityType::PLANET ) {
                                viewDragSelection.RemoveIndex( selectionIndex );
                                selectionIndex--;
                            }
                        } else if ( hasBuildingType == true ) {
                            if ( selectionEnt->type == EntityType::PLANET ) {
                                viewDragSelection.RemoveIndex( selectionIndex );
                                selectionIndex--;
                            }
                        }
                    }
                }

                if ( viewDragSelection.GetCount() >= 1 ) {
                    SimEntity * soundEnt = entityPool.Get( viewDragSelection[0] );
                    if ( soundEnt->sndHello != nullptr && soundEnt->playerNumber == localPlayerNumber ) {
                        core->AudioPlay( soundEnt->sndHello );
                    }
                }

                VisAction_PlayerSelect( &localPlayerNumber, &viewDragSelection,  EntitySelectionChange::SET );
                localActionBuffer.AddAction( MapActionType::PLAYER_SELECTION, localPlayerNumber, viewDragSelection, EntitySelectionChange::SET );
            }
        }

        BoxBounds2D selectionBounds = {};
        selectionBounds.min = glm::min( viewStartDrag, viewEndDrag );
        selectionBounds.max = glm::max( viewStartDrag, viewEndDrag );

        i32 particlesSeenAliveCount = 0;
        for ( i32 particleIndex = 0; particleIndex < viewParticles.GetCapcity(); particleIndex++ ) {
            Particle & particle = viewParticles[particleIndex];
            if ( particle.alive == true ) {
                particlesSeenAliveCount++;

                particle.spriteAnimator.Update( core, dt );
                spriteDrawContext->DrawSprite( particle.spriteAnimator.sprite, particle.spriteAnimator.frameIndex, particle.pos, particle.ori, glm::vec2( 1 ), particle.spriteAnimator.color );

                if ( particle.spriteAnimator.loopCount >= 1 ) {
                    particle.alive = false;
                    viewParticleAliveCount--;
                    particlesSeenAliveCount--;
                }
            }
            if ( particlesSeenAliveCount == viewParticleAliveCount ) {
                break;
            }
        }

        const i32 entityCount = entities.GetCount();
        for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
            SimEntity * ent = entities[ entityIndexA ];

            f32 visMove = ( 1 - glm::exp( -dt * 16 ) );
            ent->visPos += ( ToVec2( ent->pos ) - ent->visPos ) * visMove;
            ent->visOri = LerpAngle( ent->visOri, ToFloat( ent->ori ), dt * 7 );
            //ent->visOri += ( ToFloat( ent->ori ) - ent->visOri ) * visMove;

            if ( viewIsDragging == true && ent->isSelectable == true ) {
                Collider2D collider = ent->GetWorldSelectionCollider();
                if ( collider.Intersects( selectionBounds ) == true ) {
                    viewDragSelection.AddUnique( ent->handle );
                }
            }

            // @LOCATION: DRAW
            if ( viewMode == ViewMode::SOLAR && ent->solarNumber == viewSolarNumber ) {
                glm::vec2 drawPos = ent->visPos;
                f32 drawOri = ent->visOri;

                if ( ent->visSelectedBy.Contains( localPlayerNumber ) && ent->playerNumber == localPlayerNumber && ent->navigator.hasDest == true ) {
                    glm::vec2 dest = ToVec2( ent->navigator.dest );
                    spriteDrawContext->DrawTexture( sprMoveLocation, dest, 0.0f );
                }

                if ( ent->visSelectedBy.Contains( localPlayerNumber ) && ent->selectionAnimator.sprite != nullptr ) {
                    //spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, ent->pos, ent->ori );
                    spriteDrawContext->DrawSprite( ent->selectionAnimator.sprite, ent->selectionAnimator.frameIndex, drawPos, drawOri );
                }

                ent->spriteAnimator.Update( core, dt );
                if ( ent->spriteAnimator.sprite != nullptr ) {
                    spriteDrawContext->DrawSprite( ent->spriteAnimator.sprite, ent->spriteAnimator.frameIndex, drawPos, drawOri, glm::vec2( 1 ), ent->spriteAnimator.color );
                }

                ent->engineAnimator.Update( core, dt );
                if ( ent->engineAnimator.sprite != nullptr ) {
                    spriteDrawContext->DrawSprite( ent->engineAnimator.sprite, ent->engineAnimator.frameIndex, drawPos, drawOri, glm::vec2( 1 ), ent->engineAnimator.color );
                }

                if ( IsUnitType( ent->type ) ) {
                    const Unit & unit = ent->unit;
                    if ( glm::length2( ToVec2( ent->vel ) ) > 2 ) {
                        ent->engineAnimator.SetSpriteIfDifferent( ent->spriteUnit.engine, true );
                    } else {
                        ent->engineAnimator.SetSpriteIfDifferent ( nullptr, false );
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
                    } else if ( building.trainingQueue.IsEmpty() == false ) {
                        EntityType trainingEnt = *building.trainingQueue.Peek();
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteBank[ 1 ], false );
                        const i32 timeToTrainTurns = GetTrainTimeForEntityType( trainingEnt );
                        const f32 f = ( f32 ) building.turn / timeToTrainTurns;
                        glm::vec2 bl = ent->visPos + glm::vec2( -50, 20 );
                        glm::vec2 tr = ent->visPos + glm::vec2( 50, 30 );
                        spriteDrawContext->DrawRect( bl, tr, glm::vec4( 0.7f ) );
                        tr.x = glm::mix( bl.x, tr.x, f );
                        spriteDrawContext->DrawRect( bl, tr, glm::vec4( 0.9f ) );
                    } else {
                        ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteBank[ 0 ], false );
                    }
                }

  
                if ( isPlacingBuilding == true ) {
                    spriteDrawContext->DrawTexture( sprMoveLocation, mousePosWorld );
                }

                // DEBUG
                #if ATTO_DEBUG
                if ( false ) {
                    if ( ent->visSelectionCollider.type == ColliderType::COLLIDER_TYPE_CIRCLE ) {
                        debugDrawContext->DrawCircle( ent->visPos, ent->visSelectionCollider.circle.rad, glm::vec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
                    } else if ( ent->visSelectionCollider.type == ColliderType::COLLIDER_TYPE_AXIS_BOX ) {
                        Collider2D c = ent->GetWorldSelectionCollider();
                        debugDrawContext->DrawRect( c.box.min, c.box.max, glm::vec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
                    }
                }
                if ( false ) {
                    FpCollider col = ent->GetWorldCollisionCollider();
                    if ( col.type == ColliderType::COLLIDER_TYPE_CIRCLE ) {
                        debugDrawContext->DrawCircle( ToVec2( col.circle.pos ), ToFloat( col.circle.rad ), glm::vec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
                    } else if ( col.type == ColliderType::COLLIDER_TYPE_AXIS_BOX ) {
                        debugDrawContext->DrawRect( ToVec2( col.box.min ), ToVec2( col.box.max ), glm::vec4( 1.0f, 1.0f, 1.0f, 0.5f ) );
                    }
                }
                #endif
            }
        }

        if ( viewMode == ViewMode::GALAXY ) {

            // @SPEED: We could cache this but I'm going to rebuild it every frame for now.
            viewSolarSystemConnections.Clear();

            for ( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                SimEntity * ent = entities[ entityIndexA ];

                if ( ent->type == EntityType::SOLAR_SYSTEM ) {
                    const f32 r = 8;
                    const f32 fontSize = 24;
                    const glm::vec2 textPos = spriteDrawContext->WorldPosToScreenPos( ent->visPos + glm::vec2( 0, r + fontSize / 2.0f ) );
                    spriteDrawContext->DrawTextScreen( fontHandle, textPos, fontSize, ent->solarSystem.name.GetCStr(), TextAlignment_H::FONS_ALIGN_CENTER, TextAlignment_V::FONS_ALIGN_MIDDLE );
                    if ( ent->visSelectedBy.Contains( localPlayerNumber ) == true ) {
                        spriteDrawContext->DrawCircle( ent->visPos, r + 1, Colors::GREEN );
                    }
                    spriteDrawContext->DrawCircle( ent->visPos, r, glm::vec4( 0.6, 0.6, 0.6, 1.0f ) );

                    const i32 connectionCount = ent->solarSystem.connections.GetCount();
                    for ( i32 connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++ ) {
                        EntityHandle conHandle = *ent->solarSystem.connections.Get( connectionIndex );

                        SolarSystemConnectionPair pair = {};
                        pair.a = ent->handle;
                        pair.b = conHandle;

                        viewSolarSystemConnections.AddUnique( pair );
                    }
                }
            }

            const i32 viewSolarSystemConnectionsCount = viewSolarSystemConnections.GetCount();
            for ( i32 connectionIndex = 0; connectionIndex < viewSolarSystemConnectionsCount ; connectionIndex++ ) {
                SolarSystemConnectionPair pair = *viewSolarSystemConnections.Get( connectionIndex );
                SimEntity * entA = entityPool.Get( pair.a );
                SimEntity * entB = entityPool.Get( pair.b );
                if ( entA != nullptr && entB != nullptr ) {
                    glm::vec2 p1 = entA->visPos;
                    glm::vec2 p2 = entB->visPos;
                    spriteDrawContext->DrawLine( p1, p2, 1 );
                }
            }

        }

        if ( isMouseOverUI == false && core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
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
                        } else if ( IsBuildingType( ent->type ) == true ) {
                            if ( ent->teamNumber != localPlayerTeamNumber ) {
                                localActionBuffer.AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK, localPlayerNumber, ent->handle );
                            } else if ( ent->building.isBuilding == true ) {
                                for ( i32 entityIndexB = 0; entityIndexB < entities.GetCount(); entityIndexB++ ) {
                                    const SimEntity * otherEnt = *entities.Get( entityIndexB );
                                    if ( otherEnt->playerNumber == localPlayerNumber && otherEnt->type == EntityType::UNIT_KLAED_WORKER ) {
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

                
                // @HACK: We should use a featured unit
                for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                    const SimEntity * sndEnt = entities[ entityIndex ];
                    if ( sndEnt->visSelectedBy.Contains( localPlayerNumber ) == true ) {
                        if ( sndEnt->sndMove != nullptr ) {
                            core->AudioPlay( sndEnt->sndMove );
                            break;
                        }
                    }
                }
            }
        }

        if ( viewIsDragging == true ) {
            glm::vec2 minBox = glm::min( viewStartDrag, viewEndDrag );
            glm::vec2 maxBox = glm::max( viewStartDrag, viewEndDrag );
            spriteDrawContext->DrawRect( minBox, maxBox, Colors::BOX_SELECTION_COLOR );
        }

        if( core->InputKeyJustPressed( KEY_CODE_F11 ) == true ) {
            mapReplay.PrintActions( core );
        }

        SmallString s = StringFormat::Small( "ping=%d", (i32)core->NetworkGetPing() );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 0, 128 ), 32, s.GetCStr() );
        s = StringFormat::Small( "dt=%f", dt );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 0, 160 ), 32, s.GetCStr() );
        s = StringFormat::Small( "fps=%f", 1.0f / dt );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 0, 200 ), 32, s.GetCStr() );
        s = StringFormat::Small( "turn=%d", turnNumber );
        spriteDrawContext->DrawTextScreen( fontHandle, glm::vec2( 0, 230 ), 32, s.GetCStr() );
        //ScopedClock renderingClock( "Rendering clock", core );

        core->RenderSubmit( spriteDrawContext, true );
        //core->RenderSubmit( backgroundDrawContext, false );
        core->RenderSubmit( uiDrawContext, false );
        core->RenderSubmit( debugDrawContext, false );
    }

    i32 GetTrainTimeForEntityType( EntityType type ) {
        if ( type == EntityType::UNIT_KLAED_WORKER ) {
            return cfgKlaedWorkerTrainTimeTurns;
        } else if ( type == EntityType::UNIT_KLAED_SCOUT ) {
            return cfgKlaedScoutTrainTimeTurns;
        } else if ( type == EntityType::UNIT_KLAED_FIGHTER ) {
            return cfgKlaedFighterTrainTimeTurns;
        }

        INVALID_CODE_PATH;
        return 0;
    }

    SimEntity * SimMap::SpawnEntity( SimEntitySpawnInfo * spawnInfo ) {
        EntityHandle handle = {};
        SimEntity * entity = entityPool.Add( handle );
        AssertMsg( entity != nullptr, "Spawn Entity is nullptr" );
        if( entity != nullptr ) {
            ZeroStructPtr( entity );
            entity->handle = handle;

            entity->active = true;
            entity->type = spawnInfo->type;
            entity->resistance = ToFP( 14.0f );
            entity->pos = spawnInfo->pos;
            entity->visPos = ToVec2( spawnInfo->pos );
            entity->ori = spawnInfo->ori;
            entity->visOri = ToFloat( spawnInfo->ori );
            entity->vel = spawnInfo->vel;
            entity->teamNumber = spawnInfo->teamNumber;
            entity->playerNumber = spawnInfo->playerNumber;
            entity->solarNumber = spawnInfo->solarNumber;

            switch( entity->type ) {
                case EntityType::UNIT_KLAED_WORKER: {
                    

                    entity->spriteUnit.base = sprKlaedWorkerBase;
                    entity->spriteUnit.engine = sprKlaedWorkerEngine;
                    entity->spriteUnit.destruction = sprKlaedWorkerDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->sndHello = sndKlaedWorkerHello;
                    entity->sndMove = sndKlaedWorkerMove;
                    entity->sndDestructions.Add( sndKlaedWorkerDestruction1 );
                    entity->sndDestructions.Add( sndKlaedWorkerDestruction2 );
                    entity->sndDestructions.Add( sndKlaedWorkerDestruction3 );
                    entity->sndDestructions.Add( sndKlaedWorkerDestruction4 );

                    if ( playSpawningSounds == true ) {
                        core->AudioPlay( sndKlaedWorkerBuilt );
                    }

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.pos = glm::vec2( 0, 0 );
                    entity->visSelectionCollider.circle.rad = 16.0f;
                    
                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 50;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.range = ToFP( 40 );
                    entity->unit.speed = Fp( 25 );

                } break;
                case EntityType::UNIT_KLAED_SCOUT:
                {
                    static AudioResource * sndHello = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/shoot.mp3", true, false, 0, 0 );
                    static AudioResource * sndBuilt = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/five-by-five-hele.mp3", true, false, 0, 0 );
                    static AudioResource * sndMove = core->ResourceGetAndCreateAudio( "res/sounds/not_legal/starcraft/proceeding-seige.mp3", true, false, 0, 0 );

                    entity->spriteUnit.base = sprKlaedScoutBase;
                    entity->spriteUnit.engine = sprKlaedScoutEngine;
                    entity->spriteUnit.shield = sprKlaedScoutShield;
                    entity->spriteUnit.weapons = sprKlaedScoutWeapon;
                    entity->spriteUnit.destruction = sprKlaedScoutDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->sndHello = sndHello;
                    entity->sndMove = sndMove;
                    if ( playSpawningSounds == true ) {
                        core->AudioPlay( sndBuilt );
                    }

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 36 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.speed = Fp( 75 );
                    entity->unit.range = ToFP( 400 );
                    entity->unit.fireRateTurns = SecondsToTurns( 0, 5 );
                    entity->unit.weapons.SetCount( 1 );
                    entity->unit.weaponType = EntityType::BULLET_KLARD_BULLET;
                    entity->unit.weapons[ 0 ].posOffset = Fp2( 0, 12 );
                    entity->unit.weapons[ 0 ].fireRateDelayTurns = SecondsToTurns( 0 );
                } break;
                case EntityType::UNIT_KLAED_FIGHTER: {
                    entity->spriteUnit.base = sprKlaedFighterBase;
                    entity->spriteUnit.engine = sprKlaedFighterEngine;
                    entity->spriteUnit.shield = sprKlaedFighterShield;
                    entity->spriteUnit.weapons = sprKlaedFighterWeapon;
                    entity->spriteUnit.destruction = sprKlaedFighterDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 26 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.speed = Fp( 35 );
                    entity->unit.range = ToFP( 400 );
                    entity->unit.fireRateTurns = SecondsToTurns( 0, 5 );
                    entity->unit.weapons.SetCount( 2 );
                    entity->unit.weaponType = EntityType::BULLET_KLARD_BULLET;
                    entity->unit.weapons[ 0 ].posOffset = Fp2( 6, 4 );
                    entity->unit.weapons[ 0 ].fireRateDelayTurns = SecondsToTurns( 0 );
                    entity->unit.weapons[ 1 ].posOffset = Fp2( -6, 4 );
                    entity->unit.weapons[ 1 ].fireRateDelayTurns = SecondsToTurns( 0, 2 );
                } break;
                case EntityType::UNIT_NAIRAN_WORKER: {
                    entity->spriteUnit.base = sprNairanWorkerBase;
                    entity->spriteUnit.engine = sprNairanWorkerEngine;
                    entity->spriteUnit.destruction = sprNairanWorkerDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.pos = glm::vec2( 0, 0 );
                    entity->visSelectionCollider.circle.rad = 16.0f;
                    
                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 50;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.range = ToFP( 40 );
                    entity->unit.speed = Fp( 25 );
                } break;
                case EntityType::UNIT_NAIRAN_SCOUT: {
                    entity->spriteUnit.base = sprNairanScoutBase;
                    entity->spriteUnit.engine = sprNairanScoutEngine;
                    entity->spriteUnit.shield = sprNairanScoutShield;
                    entity->spriteUnit.weapons = sprNairanScoutWeapon;
                    entity->spriteUnit.destruction = sprNairanScoutDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 36 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;
                    entity->unit.speed = Fp( 25 );
                    entity->unit.range = ToFP( 400 );
                    entity->unit.fireRateTurns = SecondsToTurns( 0, 5 );
                    entity->unit.weapons.SetCount( 2 );
                    entity->unit.weaponType = EntityType::PROJECTILE_NAIRAN_BOLT;
                    entity->unit.weapons[ 0 ].posOffset = Fp2( 9, 3 );
                    entity->unit.weapons[ 0 ].fireRateDelayTurns = SecondsToTurns( 0 );
                    entity->unit.weapons[ 1 ].posOffset = Fp2( -9, 3 );
                    entity->unit.weapons[ 1 ].fireRateDelayTurns = SecondsToTurns( 0, 2 );
                } break;
                case EntityType::UNIT_NAIRAN_FIGHTER: {
                    entity->spriteUnit.base = sprNairanFighterBase;
                    entity->spriteUnit.engine = sprNairanFighterEngine;
                    entity->spriteUnit.shield = sprNairanFighterShield;
                    entity->spriteUnit.weapons = sprNairanFighterWeapon;
                    entity->spriteUnit.destruction = sprNairanFighterDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 26 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;

                    const i32 baseAnimationLengthTurns = SecondsToTurns( Fp( sprNairanFighterWeapon->frameCount ) / Fp( sprNairanFighterWeapon->frameRate ) );

                    entity->unit.speed = Fp( 25 );
                    entity->unit.range = ToFP( 400 );
                    entity->unit.fireRateTurns = baseAnimationLengthTurns - SecondsToTurns( 2, 3 );
                    entity->unit.weapons.SetCount( 6 );
                    entity->unit.weaponType = EntityType::PROJECTILE_NAIRAN_ROCKET;
                    entity->unit.weapons[ 0 ].posOffset = Fp2( 8, 7 );
                    entity->unit.weapons[ 0 ].fireRateDelayTurns = SecondsToTurns( 0, 7 );
                    entity->unit.weapons[ 1 ].posOffset = Fp2( -8, 5 );
                    entity->unit.weapons[ 1 ].fireRateDelayTurns = SecondsToTurns( 1 );
                    entity->unit.weapons[ 2 ].posOffset = Fp2( 10, 5 );
                    entity->unit.weapons[ 2 ].fireRateDelayTurns = SecondsToTurns( 1, 3 );
                    entity->unit.weapons[ 3 ].posOffset = Fp2( -10, 5 );
                    entity->unit.weapons[ 3 ].fireRateDelayTurns = SecondsToTurns( 1, 6 );
                    entity->unit.weapons[ 4 ].posOffset = Fp2( 12, 5 );
                    entity->unit.weapons[ 4 ].fireRateDelayTurns = SecondsToTurns( 2, 1 );
                    entity->unit.weapons[ 5 ].posOffset = Fp2( -12, 5 );
                    entity->unit.weapons[ 5 ].fireRateDelayTurns = SecondsToTurns( 2, 3 );
                } break;
                case EntityType::BULLET_KLARD_BULLET:
                {
                    static AudioResource * sndDestruction1 = core->ResourceGetAndCreateAudio( "res/sounds/bullet_hit_smol_01.wav", true, false, 0, 0 );
                    sndDestruction1->maxInstances = 2;
                    sndDestruction1->stealMode = AudioStealMode::OLDEST;
                    sndDestruction1->volumeMultiplier = 1.75f;

                    static AudioResource * sndDestruction2 = core->ResourceGetAndCreateAudio( "res/sounds/bullet_hit_smol_02.wav", true, false, 0, 0 );
                    sndDestruction2->maxInstances = 2;
                    sndDestruction2->stealMode = AudioStealMode::OLDEST;
                    sndDestruction2->volumeMultiplier = 1.75f;

                    entity->sprBullet.base = sprKlaedProjectileBullet;
                    entity->sprBullet.destruction = sprKlaedProjectileBulletHit;
                    entity->spriteAnimator.SetSpriteIfDifferent( entity->sprBullet.base, true );

                    entity->sndDestructions.Add( sndDestruction1 );
                    entity->sndDestructions.Add( sndDestruction2 );
                    core->AudioPlay( sndLaser2[ Random::Int( 6 ) ] );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 4 );

                    entity->bullet.aliveTime = Fp( 1.87f );
                    entity->bullet.damage = 5;
                } break;
                case EntityType::PROJECTILE_NAIRAN_BOLT: {
                    entity->sprBullet.base = sprNairanProjectileBolt;
                    entity->sprBullet.destruction = sprKlaedProjectileBulletHit;
                    entity->spriteAnimator.SetSpriteIfDifferent( entity->sprBullet.base, true );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 4 );

                    entity->bullet.aliveTime = Fp( 1.87f );
                    entity->bullet.damage = 5;
                } break;
                case EntityType::PROJECTILE_NAIRAN_ROCKET: {
                    entity->sprBullet.base = sprNairanProjectileRocket;
                    entity->sprBullet.destruction = sprKlaedProjectileBulletHit;
                    entity->spriteAnimator.SetSpriteIfDifferent( entity->sprBullet.base, true );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 4 );

                    entity->bullet.aliveTime = Fp( 1.87f );
                    entity->bullet.damage = 5;
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
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.pos = glm::vec2( 0, 0 );
                    entity->visSelectionCollider.circle.rad = 125.0f;

                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                } break;
                case EntityType::SOLAR_SYSTEM:
                {
                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.pos = glm::vec2( 0, 0 );
                    entity->visSelectionCollider.circle.rad = 8;
                    memcpy( &entity->solarSystem, &spawnInfo->solarSystem, sizeof( SolarSystem ) );

                    // @NOTE: CREATE NEW CONNECTIONS
                    const i32 connectionCount = entity->solarSystem.connections.GetCount();
                    for ( i32 connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++ ) {
                        EntityHandle conHandle = *entity->solarSystem.connections.Get( connectionIndex );
                        SimEntity * connectionEnt = entityPool.Get( conHandle );
                        if ( connectionEnt != nullptr ) {
                            Assert( connectionEnt->type == EntityType::SOLAR_SYSTEM );
                            if ( connectionEnt->type == EntityType::SOLAR_SYSTEM ) {
                                connectionEnt->solarSystem.connections.AddUnique( entity->handle );
                            }
                        }
                    }
                } break;
                case EntityType::BUILDING_STATION:
                {
                    static SpriteResource * blueSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_blue_off.png", 1, 64, 64, 0 );
                    static SpriteResource * blueSpriteOn =  core->ResourceGetAndCreateSprite( "res/ents/test/building_station_blue_on.png", 1, 64, 64, 0 );
                    static SpriteResource * redSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_red_off.png", 1, 64, 64, 0 );
                    static SpriteResource * redSpriteOn  = core->ResourceGetAndCreateSprite( "res/ents/test/building_station_red_on.png", 1, 64, 64, 0 );
                    
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    if ( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( blueSpriteOff, false );
                        entity->spriteBank[ 0 ] =  blueSpriteOff;
                        entity->spriteBank[ 1 ] = blueSpriteOn;
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( redSpriteOff, false );
                        entity->spriteBank[ 0 ] = redSpriteOff;
                        entity->spriteBank[ 1 ] = redSpriteOn;
                    }

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 64 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->collisionCollider.box = FpAxisBoxCreateFromCenterSize( Fp2( 0, 0 ), Fp2( 48, 48 ) );

                    entity->maxHealth = 200;
                    entity->currentHealth = entity->maxHealth;

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 60 );

                } break;
                case EntityType::BUILDING_SOLAR_ARRAY:
                {
                    static SpriteResource * blueSprite = core->ResourceGetAndCreateSprite( "res/ents/test/building_solar_array_blue.png", 1, 64, 32, 0 );
                    static SpriteResource * redSprite = core->ResourceGetAndCreateSprite( "res/ents/test/building_solar_array_red.png", 1, 64, 32, 0 );

                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    if ( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( blueSprite, false );
                        entity->spriteBank[ 0 ] = blueSprite;
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( redSprite, false );
                        entity->spriteBank[ 1 ] = redSprite;
                    }

                    SpriteResource * mainSprite = spawnInfo->teamNumber.value == 1 ? blueSprite : redSprite;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 32 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->collisionCollider.box = FpAxisBoxCreateFromCenterSize( Fp2( 0, 0 ), Fp2( 48, 24 ) );

                    entity->maxHealth = 100;
                    entity->currentHealth = entity->maxHealth;

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 30 );

                    const i32 energyPerMinute = 45;
                    const i32 energyTickRate = 8;

                    entity->building.timeToGiveEnergyTurns = SecondsToTurns( 60 ) / energyTickRate;
                    entity->building.amountToGiveEnergy = energyPerMinute / energyTickRate;

                    // @SPEED:
                    //simAction_ActiveEntities.Clear( false );
                    //entityPool.GatherActiveObjs( simAction_ActiveEntities );
                    //for( i32 starIndex = 0; starIndex < simAction_ActiveEntities.GetCount(); starIndex++ ) {
                    //    if( simAction_ActiveEntities[ starIndex ]->type == EntityType::STAR ) {
                    //        fp2 dir = simAction_ActiveEntities[ starIndex ]->pos - spawnInfo->pos;
                    //        entity->ori = FpATan2( dir.x, dir.y );
                    //
                    //        // @SPEED:
                    //        const fp dist = FpLength( dir );
                    //        const fp minDist = Fp( 600 ); // @NOTE: Based of start 500
                    //        const fp maxDist = Fp( 1250 );
                    //        const fp t = FpClamp( dist, minDist, maxDist );
                    //        // Make 0 - 1
                    //        const fp t01 = ( t - minDist ) / ( maxDist - minDist );
                    //        entity->building.giveEnergyAmount = (i32)( Fp( 1 ) + Fp( 4 ) * FpCos( t01 ) );
                    //        entity->building.isBuilding = true;
                    //        entity->building.timeToBuildTurns = SecondsToTurns( 30 );
                    //        break;
                    //    }
                    //}
                } break;
                case EntityType::BUILDING_COMPUTE:
                {
                    static SpriteResource * blueSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_blue_off.png", 1, 48, 48, 0 );
                    static SpriteResource * blueSpriteOn =  core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_blue_on.png", 1, 48, 48, 0 );
                    static SpriteResource * redSpriteOff = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_red_off.png", 1, 48, 48, 0 );
                    static SpriteResource * redSpriteOn  = core->ResourceGetAndCreateSprite( "res/ents/test/building_cpu_red_on.png", 1, 48, 48, 0 );

                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    if ( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( blueSpriteOff, false );
                        entity->spriteBank[ 0 ] =  blueSpriteOff;
                        entity->spriteBank[ 1 ] =  blueSpriteOn;
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( redSpriteOff, false );
                        entity->spriteBank[ 0 ] = redSpriteOff;
                        entity->spriteBank[ 1 ] =  redSpriteOn;
                    }

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.rad = 22;

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 22 );

                    entity->maxHealth = 75;
                    entity->currentHealth = entity->maxHealth;

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 40 );

                    const i32 computePerMinute = 30;
                    const i32 computeTickRate = 5;
                    entity->building.timeToGiveComputeTurns = SecondsToTurns( 60 ) / computeTickRate;
                    entity->building.amountToGiveCompute = computePerMinute / computeTickRate;
                }
            }
        }

        return entity;
    }

    SimEntity * SimMap::SpawnEntity( EntityType type, PlayerNumber playerNumber, TeamNumber teamNumber, SolarNumber solarNumber, fp2 pos, fp ori, fp2 vel ) {
        SimEntitySpawnInfo spawnInfo = {};
        spawnInfo.type = type;
        spawnInfo.playerNumber = playerNumber;
        spawnInfo.teamNumber = teamNumber;
        spawnInfo.solarNumber = solarNumber;
        spawnInfo.pos = pos;
        spawnInfo.vel = vel;
        spawnInfo.ori = ori;
        return SpawnEntity( &spawnInfo );
    }

    void SimMap::DestroyEntity( SimEntity * entity ) {
        if( entity != nullptr ) {
            if ( IsUnitType( entity->type ) == true ) {
                Particle p = {};
                p.pos = entity->visPos;
                p.ori = entity->visOri;
                p.alive = true;
                p.spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.destruction, false );

                viewParticleAliveCount++;
                viewParticles.Add( p );

                if ( entity->sndDestructions.GetCount() > 0 ) {
                    core->AudioPlayRandom( entity->sndDestructions );
                }
            }

            if ( entity->type == EntityType::BULLET_KLARD_BULLET ) {
                if ( entity->bullet.aliveTimer <= entity->bullet.aliveTime ) {
                    Particle p = {};
                    p.pos = entity->visPos;
                    p.ori = entity->visOri;
                    p.alive = true;
                    p.spriteAnimator.SetSpriteIfDifferent( entity->sprBullet.destruction, false );

                    viewParticleAliveCount++;
                    viewParticles.Add( p );

                    if ( entity->sndDestructions.GetCount() > 0 ) {
                        core->AudioPlayRandom( entity->sndDestructions );
                    }
                }
            }

            entityPool.Remove( entity->handle );
        }
    }

    void SimMap::SimAction_SpawnEntity( i32 * typePtr, PlayerNumber * playerNumberPtr, TeamNumber * teamNumberPtr, SolarNumber * solarNumberPtr, fp2 * posPtr, fp * oriPtr, fp2 * velPtr ) {
        EntityType type = EntityType::Make( (EntityType::_enumerated)( * typePtr) );
        PlayerNumber playerNumber = *playerNumberPtr;
        TeamNumber teamNumber = *teamNumberPtr;
        SolarNumber solarNumber = *solarNumberPtr;
        fp ori = *oriPtr;
        fp2 pos = *posPtr;
        fp2 vel = *velPtr;
        SpawnEntity( type, playerNumber, teamNumber, solarNumber, pos, ori, vel );
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

        //VisAction_PlayerSelect( playerNumberPtr, selection, change );
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
                ent->unit.command.type = UnitCommandType::ATTACK;
                ent->unit.command.targetEnt = target;
            }
        }
    }

    void SimMap::SimAction_ContructBuilding( PlayerNumber * playerNumberPtr, i32 * typePtr, fp2 * posPtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        EntityType type = EntityType::Make( (EntityType::_enumerated)( * typePtr) );
        fp2 pos = *posPtr;

        // @HACK: Monies
        if ( type == EntityType::BUILDING_STATION ) {
            SimUtil_Pay( playerNumber, costOfBuildingStation );
        }
        if ( type == EntityType::BUILDING_SOLAR_ARRAY ) {
            SimUtil_Pay( playerNumber, costOfBuildingSolar );
        }
        if ( type == EntityType::BUILDING_COMPUTE ) {
            SimUtil_Pay( playerNumber, costOfBuildingCompute );
        } 

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        const i32 entityCount = simAction_ActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && ent->type == EntityType::UNIT_KLAED_WORKER && ent->selectedBy.Contains( playerNumber ) ) {
                SolarNumber s1 = SolarNumber::Create( 1 ); // @HACK SOLAR NUMBER
                SimEntity * structure = SpawnEntity( type, playerNumber, ent->teamNumber, s1, pos, Fp( 0 ), Fp2( 0, 0 ) );
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
                    if ( ent->playerNumber == playerNumber && ent->type == EntityType::UNIT_KLAED_WORKER && ent->selectedBy.Contains( playerNumber ) ) {
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

        // @HACK: Monies
        if ( placementType == PlanetPlacementType::CREDIT_GENERATOR ) {
            SimUtil_Pay( playerNumber, costOfPlacementCredit );
        }
        if ( placementType == PlanetPlacementType::ENERGY_GENERATOR ) {
            SimUtil_Pay( playerNumber, costOfPlacementSolar );
        }
        if ( placementType == PlanetPlacementType::COMPUTE_GENERATOR ) {
            SimUtil_Pay( playerNumber, costOfPlacementCompute );
        }

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        simAction_EntityFilter.Begin( &simAction_ActiveEntities )->
            SimSelectedBy( playerNumber )->
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
            SimSelectedBy( playerNumber )->
            OwnedBy( playerNumber )->
            Type( EntityType::BUILDING_STATION )->
            End(); 

        const i32 count = simAction_EntityFilter.result.GetCount();
        for ( i32 entityIndex = 0; entityIndex < count; entityIndex++ ) {
            SimEntity * ent = *simAction_EntityFilter.result.Get( entityIndex );
            Assert( ent->type == EntityType::BUILDING_STATION );

            Building & building = ent->building;

            // @HACK: Monies
            if ( type == EntityType::UNIT_KLAED_WORKER ) {
                SimUtil_Pay( playerNumber, costOfWorker );
            }
            if ( type == EntityType::UNIT_KLAED_SCOUT ) {
                SimUtil_Pay( playerNumber, costOfFighter );
            }

            if ( building.trainingQueue.IsEmpty() == true ) {
                building.turn = 0;
            }
            if ( building.trainingQueue.IsFull() == false ) {
                building.trainingQueue.Enqueue( type );
            }
        }
    }

    void SimMap::SimAction_ApplyDamage( i32 * damagePtr, EntityHandle * targetPtr ) {
        const i32 damage = *damagePtr;
        const EntityHandle target = *targetPtr;
        SimEntity * targetEnt = entityPool.Get( target );
        if( targetEnt != nullptr ) {
            targetEnt->currentHealth -= damage;
            if( targetEnt->currentHealth <= 0 ) {
                targetEnt->currentHealth = 0;
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
        const fp maxForce = Fp( 20.0f );
        const fp invMass = Fp( 1.0f / 10.0f );

        ZeroStruct( ent->actions );

        switch( ent->type ) {
            case EntityType::UNIT_KLAED_WORKER:
            case EntityType::UNIT_KLAED_SCOUT:
            case EntityType::UNIT_KLAED_FIGHTER:
            case EntityType::UNIT_NAIRAN_WORKER:
            case EntityType::UNIT_NAIRAN_SCOUT:
            case EntityType::UNIT_NAIRAN_FIGHTER:
            {
                Unit & unit = ent->unit;

                if( ent->currentHealth == 0 ) { // HACK: I think this should go into the apply damange call??? Maybe something to do with death animations ?
                    ent->actions.AddAction( MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                    break;
                }

                {
                    unit.fireTimerTurns++;
                    if ( unit.fireTimerTurns > unit.fireRateTurns ) {
                        unit.fireTimerTurns = unit.fireRateTurns;
                    }

                    bool allFired = true;
                    bool hasSomeFired = false;
                    bool hasNoneFired = false;
                    const i32 weaponCount = unit.weapons.GetCount();
                    for ( i32 weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++ ) {
                        UnitWeapon & weapon = unit.weapons[weaponIndex];
                        if ( weapon.hasFired == false ) {
                            hasNoneFired = true;
                        } else {
                            hasSomeFired = true;
                        }

                        allFired = allFired && weapon.hasFired;
                    }
                    
                    // @NOTE: I'm not too sure about this, will have to keep an eye out for it.
                    if ( unit.command.type == UnitCommandType::ATTACK ) {
                        if ( allFired == true ) {
                            unit.fireTimerTurns = 0;
                            for ( i32 weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++ ) {
                                UnitWeapon & weapon = unit.weapons[weaponIndex];
                                weapon.fireTimerDelayTurns = 0;
                                weapon.hasFired = false;
                            }
                        }
                    } else {
                        if ( hasNoneFired == true && hasSomeFired == true ) {
                            unit.fireTimerTurns = 0;
                            for ( i32 weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++ ) {
                                UnitWeapon & weapon = unit.weapons[weaponIndex];
                                weapon.fireTimerDelayTurns = 0;
                                weapon.hasFired = false;
                            }
                        }
                    }
                }

                if ( unit.command.type == UnitCommandType::IDLE ) {
                    ent->navigator = {};
                    ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteUnit.base, false );
                }
                else if ( unit.command.type == UnitCommandType::MOVE ) {
                    ent->navigator.hasDest = true;
                    ent->navigator.dest = unit.command.targetPos;
                    ent->navigator.slowRad = Fp( 100 );
                    ent->navigator.facingMode = NavigatorFacingMode::VEL;
                    ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteUnit.base, true );
                }
                else if ( unit.command.type == UnitCommandType::ATTACK ) {
                    const SimEntity * targetEnt = entityPool->Get( ent->unit.command.targetEnt );
                    if ( targetEnt != nullptr ) {
                        fp2 dir = ( ent->pos - targetEnt->pos );
                        fp dist = FpLength( dir );
                        dir = dir / dist;
                        if ( dist > ent->unit.range ) {
                            ent->navigator.hasDest = true;
                            ent->navigator.dest = targetEnt->pos + dir * ( ent->unit.range - Fp( 16 ) ); // @HACK: Range
                            ent->navigator.slowRad = Fp( 100 );
                            ent->navigator.facingMode = NavigatorFacingMode::VEL;
                            ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteUnit.base, true );
                        }
                        else {
                            ent->navigator.hasDest = false;
                            ent->navigator.facingMode = NavigatorFacingMode::TARGET;
                            ent->spriteAnimator.SetSpriteIfDifferent( ent->spriteUnit.weapons, true );

                            const i32 weaponCount = unit.weapons.GetCount();
                            for ( i32 weaponIndex = 0; weaponIndex < weaponCount; weaponIndex++ ) {
                                UnitWeapon & weapon = unit.weapons[weaponIndex];
                                if ( unit.fireTimerTurns == unit.fireRateTurns ) { // If we are at the base fire rate
                                    weapon.fireTimerDelayTurns++;
                                    if ( weapon.fireTimerDelayTurns >= weapon.fireRateDelayTurns && weapon.hasFired == false ) { // If we are past the delay
                                        weapon.hasFired = true;
                                        fp spawnOri = FpATan2( -dir.x, -dir.y );
                                        fp2 spawnPos = ent->pos + FpRotate( weapon.posOffset, -spawnOri );
                                        fp2 spawnVel = - dir * Fp( 250 );
                                        Assert( unit.weaponType != EntityType::INVALID );
                                        ent->actions.AddAction( MapActionType::SIM_ENTITY_SPAWN, ( i32 )unit.weaponType, ent->playerNumber, ent->teamNumber, ent->solarNumber, spawnPos, spawnOri, spawnVel );
                                    }
                                }
                            }
                        }
                    }
                    else {
                        unit.command.type = UnitCommandType::IDLE;
                        unit.command.targetPos = {};
                        unit.command.targetEnt = {};
                    }
                }
                else if ( unit.command.type == UnitCommandType::FOLLOW ) {
                }
                else if ( unit.command.type == UnitCommandType::CONTRUCT_BUILDING ) {
                    if ( ent->type == EntityType::UNIT_KLAED_WORKER ) {
                        const SimEntity * target = entityPool->Get( unit.command.targetEnt );
                        if ( target != nullptr ) {
                            fp dist2 = FpDistance2( target->pos, ent->pos );
                            if ( dist2 < unit.range * unit.range ) {
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
                            desiredVel = FpNormalize( desiredVel ) * unit.speed * ( dist / ent->navigator.slowRad );
                        }
                    }
                    else {
                        desiredVel = FpNormalize( desiredVel ) * unit.speed;
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
                                steering = steering + dir * unit.speed * FpClamp( ( 1 - dist / Fp( 50 ) ), Fp( 0 ), Fp( 1 ) );
                            }
                        }
                    }

                    steering = FpTruncateLength( steering, maxForce );
                    steering = steering * invMass;

                    ent->vel = FpTruncateLength( ent->vel + steering, unit.speed );
                } else {
                    ent->vel = ent->vel * Fp( 0.95f );
                    if( ent->vel != Fp2( 0, 0 ) && FpLength( ent->vel ) < Fp( 1 ) ) {
                        //ent->vel = glm::vec2( 0 );
                    }
                }

                // Update orientation
                if ( ent->navigator.facingMode == NavigatorFacingMode::VEL ) {
                    if ( FpLength2( ent->vel ) >= Fp( 1 ) ) {
                        fp2 nvel = FpNormalize( ent->vel );
                        ent->ori = FpATan2( nvel.x, nvel.y );
                    }
                } else if ( ent->navigator.facingMode == NavigatorFacingMode::TARGET ) {
                    const SimEntity * targetEnt = entityPool->Get( ent->unit.command.targetEnt );
                    if ( targetEnt != nullptr ) {
                        fp2 dir = ( targetEnt->pos - ent->pos );
                        ent->ori = FpATan2( dir.x, dir.y );
                    }
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
            } break;
            case EntityType::BULLET_KLARD_BULLET:
            case EntityType::PROJECTILE_NAIRAN_BOLT:
            case EntityType::PROJECTILE_NAIRAN_ROCKET:
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
                    if( IsUnitType( otherEnt->type ) == false  && IsBuildingType( otherEnt->type ) == false ) {
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
                const i32 placementCount = planet.placements.GetCapcity();
                for ( i32 placementIndex = 0; placementIndex < placementCount; placementIndex++ ) {
                    const PlanetPlacementType & placementType = planet.placements[ placementIndex ];
                    switch ( placementType ) {
                        case PlanetPlacementType::INVALID: {} break;
                        case PlanetPlacementType::BLOCKED: {} break;
                        case PlanetPlacementType::OPEN: {} break;
                        case PlanetPlacementType::CREDIT_GENERATOR: {
                            const i32 ticker = 20;
                            const i32 pm = SecondsToTurns( 60 ) / ticker;
                            const i32 amountToGive = 100 / ticker;
                            const i32 turn = planet.placementsTurns[placementIndex]++;

                            if ( turn >= pm ) {
                                ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_CREDITS, ent->playerNumber, amountToGive );
                                planet.placementsTurns[placementIndex] = 0;
                            }
                        } break;
                        case PlanetPlacementType::ENERGY_GENERATOR: {
                            const i32 ticker = 5;
                            const i32 pm = SecondsToTurns( 60 ) / ticker;
                            const i32 amountToGive = 50 / ticker;
                            const i32 turn = planet.placementsTurns[placementIndex]++;

                            if ( turn >= pm ) {
                                ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_ENERGY, ent->playerNumber, amountToGive );
                                planet.placementsTurns[placementIndex] = 0;
                            }
                        } break;
                        case PlanetPlacementType::COMPUTE_GENERATOR: {
                            const i32 ticker = 5;
                            const i32 pm = SecondsToTurns( 60 ) / ticker;
                            const i32 amountToGive = 25 / ticker;
                            const i32 turn = planet.placementsTurns[placementIndex]++;

                            if ( turn >= pm ) {
                                ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE, ent->playerNumber, amountToGive );
                                planet.placementsTurns[placementIndex] = 0;
                            }
                        } break;
                    }
                }
            } break;
            case EntityType::BUILDING_STATION:
            case EntityType::BUILDING_SOLAR_ARRAY:
            case EntityType::BUILDING_COMPUTE:
            {
                if( ent->currentHealth == 0 ) { // HACK: I think this should go into the apply damange call??? Maybe something to do with death animations ?
                    ent->actions.AddAction( MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                    break;
                }

                Building & building = ent->building;

                if ( building.isBuilding == false ) {
                    building.turn++;
                    if ( ent->type == EntityType::BUILDING_SOLAR_ARRAY ) {
                        if ( building.turn >= building.timeToGiveEnergyTurns ) {
                            building.turn = 0;
                            ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_ENERGY, ent->playerNumber, ent->building.amountToGiveEnergy );
                        }
                    } else if ( ent->type == EntityType::BUILDING_COMPUTE ) {
                        if ( building.turn >= building.timeToGiveComputeTurns ) {
                            building.turn = 0;
                            ent->actions.AddAction( MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE, ent->playerNumber, ent->building.amountToGiveCompute );
                        }
                    }
                    else if ( ent->type == EntityType::BUILDING_STATION ) {
                        if ( building.trainingQueue.IsEmpty() == false ) {
                            EntityType trainingEnt = *building.trainingQueue.Peek();
                            const i32 timeToTrainTurns = GetTrainTimeForEntityType( trainingEnt );
                            if ( building.turn == timeToTrainTurns ) {
                                fp2 spawnLocation = ent->pos - Fp2( 0, 32 ); // @HACK
                                fp spawnOri = FP_PI; // @HACK
                                ent->actions.AddAction( MapActionType::SIM_ENTITY_SPAWN, ( i32 )trainingEnt, ent->playerNumber, ent->teamNumber, ent->solarNumber, spawnLocation, spawnOri, Fp2( 0, 0 ) );
                                building.turn = 0;
                                building.trainingQueue.Dequeue();
                            }
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

    bool SimMap::SimUtil_CanAfford( PlayerNumber playerNumber, MoneySet costSet ) {
        MoneySet mon = playerMonies[playerNumber.value - 1];
        if ( mon.credits >= costSet.credits && 
            mon.energy >= costSet.energy && 
            mon.compute >= costSet.compute ) {
            return true;
        }
        return false;
    }
  
    void SimMap::SimUtil_Pay( PlayerNumber playerNumber, MoneySet costSet ) {
        Assert( SimUtil_CanAfford( playerNumber, costSet ) );
        MoneySet & mon = playerMonies[ playerNumber.value - 1 ];
        mon.credits -= costSet.credits;
        mon.energy -= costSet.energy;
        mon.compute -= costSet.compute;
    }

    bool SimMap::Vis_CanAfford( PlayerNumber playerNumber, MoneySet costSet ) {
        MoneySet mon = playerMonies[playerNumber.value - 1];
        if ( mon.credits >= costSet.credits && 
            mon.energy >= costSet.energy && 
            mon.compute >= costSet.compute ) {
            return true;
        } else {
            core->AudioPlay( sndNotEnoughResources );
        }
        return false;
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

    inline static FpCollider FpColliderForSpace( const FpCollider & base, fp2 p ) {
        FpCollider result = base;
        if ( base.type == COLLIDER_TYPE_CIRCLE ) {
            result.circle.pos = result.circle.pos + p;
        } else if ( base.type == COLLIDER_TYPE_AXIS_BOX ) {
            result.box.min = result.box.min + p;
            result.box.max = result.box.max + p;
        }
        return result;
    }

    inline static Collider2D ColliderForSpace( const Collider2D & base, glm::vec2 p ) {
        Collider2D result = base;
        if ( result.type == COLLIDER_TYPE_CIRCLE ) {
            result.circle.pos += p;
        } else if ( result.type == COLLIDER_TYPE_AXIS_BOX ) {
            result.box.Translate( p );
        } else {
            INVALID_CODE_PATH;
        }
        return result;
    }

    FpCollider SimEntity::GetWorldCollisionCollider() const {
        return FpColliderForSpace( collisionCollider, pos );
    }

    Collider2D SimEntity::GetWorldSelectionCollider() const {
        return ColliderForSpace( visSelectionCollider, ToVec2( pos ) );
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


