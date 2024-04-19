#include "atto_sim_map.h"

#include "../../shared/atto_rpc.h"
#include "../../shared/atto_colors.h"

namespace atto {
    static FixedList<glm::vec2, 5 * 3> ui_LeftPanelCenters;
    static FixedList<glm::vec2, 5 * 3> ui_RightPanelCenters;
    static FixedList<glm::vec2, 10> ui_CenterPanelCenters;

    static AudioResource * sndLaser1[23] = {};
    static AudioResource * sndLaser2[6] = {};

    static MoneySet costOfPlacementCredit = { 20, 50, 0 };
    static MoneySet costOfPlacementSolar = { 50, 0, 0 };
    static MoneySet costOfPlacementCompute = { 100, 50, 0 };

    static MoneySet costOfBuildingStation = { 150, 0, 0 };
    static MoneySet costOfBuildingTrade = { 50, 100, 0 };
    static MoneySet costOfBuildingSolar = { 100, 0, 0 };
    static MoneySet costOfBuildingCompute = { 100, 100, 0 };

    i32 GetUnitTrainTimeForEntityType( EntityType type ) {
        if ( type == EntityType::UNIT_KLAED_WORKER ) {
            return cfgKlaedWorkerTrainTimeTurns;
        } else if ( type == EntityType::UNIT_KLAED_SCOUT ) {
            return cfgKlaedScoutTrainTimeTurns;
        } else if ( type == EntityType::UNIT_KLAED_FIGHTER ) {
            return cfgKlaedFighterTrainTimeTurns;
        } else if ( type == EntityType::UNIT_NAUTOLAN_WORKER ) {
            return cfgNautolanWorkerTrainTimeTurns;
        } else if ( type == EntityType::UNIT_NAUTOLAN_SCOUT ) {
            return cfgNautolanScoutTrainTimeTurns;
        } else if ( type == EntityType::UNIT_NAUTOLAN_FIGHTER ) {
            return cfgNautolanFighterTrainTimeTurns;
        }

        INVALID_CODE_PATH;
        return -1;
    }

    MoneySet GetUnitCostForEntityType( EntityType type ) {
        if ( type == EntityType::UNIT_KLAED_WORKER ) {
            return cfgKlaedWorkerCost;
        } else if ( type == EntityType::UNIT_KLAED_SCOUT ) {
            return cfgKlaedScoutCost;
        } else if ( type == EntityType::UNIT_KLAED_FIGHTER ) {
            return cfgKlaedFighterCost;
        } else if ( type == EntityType::UNIT_NAUTOLAN_WORKER ) {
            return cfgNautolanWorkerCost;
        } else if ( type == EntityType::UNIT_NAUTOLAN_SCOUT ) {
            return cfgNautolanScoutCost;
        } else if ( type == EntityType::UNIT_NAUTOLAN_FIGHTER ) {
            return cfgNautolanFighterCost;
        }

        INVALID_CODE_PATH;
        return {};
    }

    char GetUnitSymbolForEntityType( EntityType type ) {
        if ( type == EntityType::UNIT_KLAED_WORKER ) {
            return 'Q';
        } else if ( type == EntityType::UNIT_KLAED_SCOUT ) {
            return 'E';
        } else if ( type == EntityType::UNIT_KLAED_FIGHTER ) {
            return 'F';
        }

        INVALID_CODE_PATH;
        return {};
    }

    SpriteResource * GetSpriteForBuildingType( EntityType type ) {
        if ( type == EntityType::BUILDING_STATION ) {
            return sprBuildingStationBlueOff;
        } else if ( type == EntityType::BUILDING_TRADE ) {
            return sprBuildingTradeBlue;
        } else if ( type == EntityType::BUILDING_SOLAR_ARRAY ) {
            return sprBuildingSolarBlue;
        } else if ( type == EntityType::BUILDING_COMPUTE ) {
            return sprBuildingComputeBlueOff;
        }
        INVALID_CODE_PATH;
        return nullptr;
    }

    MoneySet GetBuildingCostForEntityType( EntityType type ) {
        if ( type == EntityType::BUILDING_STATION ) {
            return costOfBuildingStation;
        } else if (type == EntityType::BUILDING_TRADE ) {
            return costOfBuildingTrade;
        } else if ( type == EntityType::BUILDING_SOLAR_ARRAY ) {
            return costOfBuildingSolar;
        } else  if ( type == EntityType::BUILDING_COMPUTE ) {
            return costOfBuildingCompute;
        }

        INVALID_CODE_PATH;
        return {};
    }

    FpCollider GetColliderForBuildingType( EntityType type ) {
        if ( type == EntityType::BUILDING_STATION ) {
            return colBuildingStation;
        } else if ( type == EntityType::BUILDING_TRADE ) {
            return colBuildingTrade;
        } else if ( type == EntityType::BUILDING_SOLAR_ARRAY ) {
            return colBuildingSolar;
        } else if ( type == EntityType::BUILDING_COMPUTE ) {
            return colBuildingCompute;
        }
        INVALID_CODE_PATH;
        return {};
    }

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

    EntityListFilter * EntityListFilter::Types( EntityType * types, i32 count ) {
        const i32 entCount = activeEntities->GetCount();
        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            if ( marks[ entityIndex ] == true ) {
                SimEntity * ent = *activeEntities->Get( entityIndex );
                bool found = false;
                for ( i32 typeIndex = 0; typeIndex < count; typeIndex++ ) {
                    EntityType type = types[typeIndex];
                    if ( ent->type == type ) {
                        found = true;
                        break;
                    }
                }
                if ( found == false ) {
                    marks[ entityIndex ] = false;
                }
            }
        }
        return this;
    }

    EntityListFilter * EntityListFilter::TypeRange( EntityType::_enumerated start, EntityType::_enumerated end ) {
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

    static GrowableList<i32> SomethingTewst() {
        GrowableList<i32> ee = {};
        ee.Add( 2 );
        return ee;
    }

    void SimMap::Initialize( Core * core ) {
        // @TODO: This won't work if we have more than one map instance !!
        if( GlobalRpcTable[ 1 ] == nullptr ) {
            GlobalRpcTable[ ( i32 )MapActionType::PLAYER_SELECTION ]                          = new RpcMemberFunction( this, &SimMap::SimAction_PlayerSelect );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_MOVE ]              = new RpcMemberFunction( this, &SimMap::SimAction_Move );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK ]            = new RpcMemberFunction( this, &SimMap::SimAction_Attack );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING ] = new RpcMemberFunction( this, &SimMap::SimAction_ContructBuilding );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING ] = new RpcMemberFunction( this, &SimMap::SimAction_ContructExistingBuilding );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT]  = new RpcMemberFunction( this, &SimMap::SimAction_PlanetPlacePlacement );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT ]    = new RpcMemberFunction( this, &SimMap::SimAction_BuildingTrainUnit );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_BUILDING_COMMAND_CANCEL_UNIT ]    = new RpcMemberFunction( this, &SimMap::SimAction_BuildingCancelUnit );

            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_SPAWN ]                          = new RpcMemberFunction( this, &SimMap::SimAction_SpawnEntity );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_DESTROY ]                        = new RpcMemberFunction( this, &SimMap::SimAction_DestroyEntity );

            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_APPLY_DAMAGE ]                   = new RpcMemberFunction( this, &SimMap::SimAction_ApplyDamage );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_ENTITY_APPLY_CONSTRUCTION ]             = new RpcMemberFunction( this, &SimMap::SimAction_ApplyContruction );

            GlobalRpcTable[ ( i32 )MapActionType::SIM_MAP_MONIES_GIVE_CREDITS ]               = new RpcMemberFunction( this, &SimMap::SimAction_GiveCredits );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_MAP_MONIES_GIVE_ENERGY ]                = new RpcMemberFunction( this, &SimMap::SimAction_GiveEnergy );
            GlobalRpcTable[ ( i32 )MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE ]               = new RpcMemberFunction( this, &SimMap::SimAction_GiveCompute );
            GlobalRpcTable[ ( i32 )MapActionType::TEST ]                                      = new RpcMemberFunction( this, &SimMap::SimAction_Test );
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
        
        ui_CenterPanelCenters[0] = glm::vec2(320, 38) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[1] = glm::vec2( 282 + 0.0f * ( panelDim + 2.0f ), 30 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[2] = glm::vec2( 282 + 1.0f * ( panelDim + 2.0f ), 30 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[3] = glm::vec2( 282 + 3.0f * ( panelDim + 2.0f ), 30 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[4] = glm::vec2( 282 + 4.0f * ( panelDim + 2.0f ), 30 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[5] = glm::vec2( 282 + 0.0f * panelDim, 11 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[6] = glm::vec2( 282 + 1.0f * ( panelDim + 2.0f ), 11 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[7] = glm::vec2( 282 + 2.0f * ( panelDim + 2.0f ), 11 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[8] = glm::vec2( 282 + 3.0f * ( panelDim + 2.0f ), 11 ) + glm::vec2( 0.5f, -0.5f );
        ui_CenterPanelCenters[9] = glm::vec2( 282 + 4.0f * ( panelDim + 2.0f ), 11 ) + glm::vec2( 0.5f, -0.5f );

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

        viewSolarNumber = SolarNumber::Create( 1 );

        playerNumbers.Add( PlayerNumber::Create( 1 ) );
        playerNumbers.Add( PlayerNumber::Create( 2 ) );
        playerNumbers.Add( PlayerNumber::Create( 3 ) );

        for ( i32 i = 0; i < playerNumbers.GetCount(); i++ ) {
            playerMonies[i].credits = 500;
            playerMonies[i].energy = 100;
            playerMonies[i].compute = 50;
            //playerMonies[i].credits = 10000;
            //playerMonies[i].energy = 10000;
            //playerMonies[i].compute = 10000;
        }

        playerMonies[2].credits *= 2;
        playerMonies[2].energy *= 2;
        playerMonies[2].compute *= 2;

        //playerMonies[2].credits = 15000;
        //playerMonies[2].energy = 25000;
        //playerMonies[2].compute = 10000;


        PlayerNumber p0 = PlayerNumber::Create( 0 );
        TeamNumber   t0 = TeamNumber::Create( 0 );
        PlayerNumber p1 = PlayerNumber::Create( 1 );
        PlayerNumber p2 = PlayerNumber::Create( 2 );
        PlayerNumber p3 = PlayerNumber::Create( 3 );

        TeamNumber t1 = TeamNumber::Create( 1 );
        TeamNumber t2 = TeamNumber::Create( 2 );

        SolarNumber s1 = SolarNumber::Create( 1 );

        SpawnEntity( EntityType::Make( EntityType::STAR ), p0, t0, s1, Fp2( 1500.0f, 1200.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        SpawnEntity( EntityType::Make( EntityType::PLANET ), p1, t1, s1, Fp2( 500.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_WORKER ), p1, t1, s1, Fp2( 700.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        SpawnEntity( EntityType::Make( EntityType::PLANET ), p2, t1, s1, Fp2( 2500.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_WORKER ), p2, t1, s1, Fp2( 2200.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        SpawnEntity( EntityType::Make( EntityType::PLANET ), p3, t2, s1,  Fp2( 1500.0f, 2000.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAUTOLAN_WORKER ), p3, t2, s1, Fp2( 1500.0f + 200, 1700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAUTOLAN_WORKER ), p3, t2, s1, Fp2( 1500.0f - 200, 1700.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        aiThinker.clientHasAuth = localPlayerNumber == p1;
        aiThinker.aiNumber = p3;
        aiThinker.map = this;
        aiThinker.core = core;

        //GrowableList<i32> aa = {};
        //aa.Add( 37 );
        //localActionBuffer.AddAction( (i32)MapActionType::TEST, aa );


        #if 0
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_SCOUT ), p1, t1, s1, Fp2( 800.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p1, t1, s1, Fp2( 900.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_TORPEDO ), p1, t1, s1, Fp2( 1000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_FRIGATE ), p1, t1, s1, Fp2( 900.0f, 800.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_BATTLE_CRUISER ), p1, t1, s1, Fp2( 900.0f, 900.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_KLAED_DREADNOUGHT ), p1, t1, s1, Fp2( 900.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) );

        SpawnEntity( EntityType::Make( EntityType::BUILDING_STATION ), p1, t1, s1, Fp2( 500.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) )->building.isBuilding = false;
        SpawnEntity( EntityType::Make( EntityType::BUILDING_STATION ), p1, t1, s1, Fp2( 500.0f, 1100.0f ), Fp( 0 ), Fp2( 0, 0 ) )->building.isBuilding = false;
        SpawnEntity( EntityType::Make( EntityType::BUILDING_STATION ), p2, t2, s1, Fp2( 600.0f, 1000.0f ), Fp( 0 ), Fp2( 0, 0 ) )->building.isBuilding = false;

        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_SCOUT ), p2, t2, s1, Fp2( 2300.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p2, t2, s1, Fp2( 2100.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        SpawnEntity( EntityType::Make( EntityType::UNIT_NAIRAN_FIGHTER ), p2, t2, s1, Fp2( 2000.0f, 700.0f ), Fp( 0 ), Fp2( 0, 0 ) );
        #endif

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
                std::memcpy( &localMapTurn.playerActions, &localActionBuffer, sizeof( MapActionBuffer ) );
                std::memcpy( &localMapTurn.aiActions, &aiThinker.actionBuffer, sizeof( MapActionBuffer ) );

                syncQueues.AddTurn( localPlayerNumber, localMapTurn );

                ZeroStruct( msg );
                msg.type = NetworkMessageType::MAP_TURN;
                NetworkMessagePush( msg, localMapTurn );
                core->NetworkSend( msg );
                //core->LogOutput( LogLevel::INFO, "Sending turn %d", localMapTurn.turnNumber );

                ZeroStruct( localMapTurn );
                ZeroStruct( localActionBuffer );
                ZeroStruct( aiThinker.actionBuffer );
            }
        }
        else {
            dtAccumulator += dt;
            if ( dtAccumulator > SIM_DT_FLOAT ) {
                dtAccumulator -= SIM_DT_FLOAT;

                localMapTurn.playerNumber = localPlayerNumber;
                localMapTurn.turnNumber = turnNumber;
                localMapTurn.checkSum = 0;
                std::memcpy( &localMapTurn.playerActions, &localActionBuffer, sizeof( MapActionBuffer ) );
                std::memcpy( &localMapTurn.aiActions, &aiThinker.actionBuffer, sizeof( MapActionBuffer ) );

                SimTick( &localMapTurn, nullptr );

                ZeroStruct( localMapTurn );
                ZeroStruct( localActionBuffer );
                ZeroStruct( aiThinker.actionBuffer );
            }
        }

        static FontHandle fontHandle = core->ResourceGetFont( "default" );
        static TextureResource * background = core->ResourceGetAndLoadTexture( "res/game/backgrounds/light_blue.png" );
        static TextureResource * sprTurretSmol = core->ResourceGetAndLoadTexture( "res/ents/test/turret_smol.png" );
        static TextureResource * sprTurretMed = core->ResourceGetAndLoadTexture( "res/ents/test/turret_med.png" );
        static TextureResource * sprUIMock = core->ResourceGetAndLoadTexture( "res/ents/test/ui_mock_01.png" );
        static TextureResource * sprMoveLocation = core->ResourceGetAndLoadTexture( "res/ents/test/move_location.png" );
        static TextureResource * ui_InfoBackground = core->ResourceGetAndCreateTexture( "res/ents/test/ui_mock_right.png", false , false );

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, true );
        //DrawContext * backgroundDrawContext = core->RenderGetDrawContext( 1, true );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 2, true );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 3, true );

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

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );
        const glm::vec2 mousePosUISpace = uiDrawContext->ScreenPosToWorldPos( mousePosPix );
        const fp2 mousePosWorldFp = Fp2( mousePosWorld );

        BoxBounds2D uiBounds = {};
        uiBounds.min = glm::vec2( 114, 0 );
        uiBounds.max = glm::vec2( 524, 60 );
        bool isMouseOverUI = uiBounds.Contains( mousePosUISpace );

        if ( isMouseOverUI == false && core->WindowIsFullscreen() == true ) {
            if ( mousePosPix.x <= 10 ) {
                localCameraPos.x -= cameraSpeed;
            }
            if (mousePosPix.x >= core->RenderGetMainSurfaceWidth() - 10 ) {
                localCameraPos.x += cameraSpeed;
            }
            if ( mousePosPix.y <= 10 ) {
                localCameraPos.y -= cameraSpeed;
            }
            if ( mousePosPix.y >= core->RenderGetMainSurfaceHeight() - 10 ) {
                localCameraPos.y += cameraSpeed;
            }
        }

        EntList & entities = *core->MemoryAllocateTransient<EntList>();
        entityPool.GatherActiveObjs( entities );

        EntityListFilter * entityFilter = core->MemoryAllocateTransient<EntityListFilter>();

        //PlayerNumber playerNumberMonies = localPlayerNumber;
        PlayerNumber playerNumberMonies = PlayerNumber::Create( 3 );
        SmallString creditsStr = StringFormat::Small( "Credits: %d",    playerMonies[ playerNumberMonies.value - 1 ].credits );
        SmallString energyStr = StringFormat::Small( "Energy: %d",      playerMonies[ playerNumberMonies.value - 1 ].energy );
        SmallString computeStr = StringFormat::Small( "Compute: %d",    playerMonies[ playerNumberMonies.value - 1 ].compute );

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
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                    localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::CREDIT_GENERATOR );
                        planetPlacementSubMenu = false;
                        planetPlacementSubMenuIndex = -1;
                    }
                }
                if ( gameUI.ButtonPix( 146, "", ui_LeftPanelCenters[1], s, Colors::GOLD ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfPlacementSolar ) == true ) {
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
                                                    localPlayerNumber, planetPlacementSubMenuIndex, PlanetPlacementType::ENERGY_GENERATOR );
                        planetPlacementSubMenu = false;
                        planetPlacementSubMenuIndex = -1;
                    }
                }
                if ( gameUI.ButtonPix( 147, "", ui_LeftPanelCenters[2], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfPlacementCompute ) == true ) {
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_PLANET_COMMAND_PLACE_PLACEMENT,
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
                if ( core->InputKeyJustPressed( KEY_CODE_T ) || gameUI.ButtonPix( 234, "T", ui_RightPanelCenters[ 1 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfBuildingTrade ) == true ) {
                        isPlacingBuilding = true;
                        placingBuildingType = EntityType::BUILDING_TRADE;
                    }
                }
                if ( core->InputKeyJustPressed( KEY_CODE_E ) || gameUI.ButtonPix( 233, "E", ui_RightPanelCenters[ 2 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfBuildingSolar ) == true ) {
                        isPlacingBuilding = true;
                        placingBuildingType = EntityType::BUILDING_SOLAR_ARRAY;
                    }
                }
                if ( core->InputKeyJustPressed( KEY_CODE_C ) || gameUI.ButtonPix( 236, "C", ui_RightPanelCenters[ 3 ], s, Colors::SKY_BLUE ) ) {
                    if ( Vis_CanAfford( localPlayerNumber, costOfBuildingCompute ) == true ) {
                        isPlacingBuilding = true;
                        placingBuildingType = EntityType::BUILDING_COMPUTE;
                    }
                }

                if ( gameUI.hoverId == 232 || gameUI.hoverId == 234 || gameUI.hoverId == 233 || gameUI.hoverId == 236 ) {
                    uiDrawContext->DrawTextureBL( ui_InfoBackground, glm::vec2( 537, 0 ) ); // @HACK: Bit of cheat here drawing the background.
                    UIWidgetPos pos = {};
                    pos.type = UI_PosType::PIXELS;
                    pos.value = glm::vec2( 539, 64 );
                    UIWidgetSize sizeX = {};
                    sizeX.type = UI_SizeType::PIXELS;
                    sizeX.value = 105;
                    UIWidgetSize sizeY = {};
                    sizeY.type = UI_SizeType::CHILDRENSUM;
                    gameUI.BeginVBox( 23425, pos, sizeX, sizeY, 0 );

                    if ( gameUI.hoverId == 232 ) {
                        gameUI.Lable( "Construction", true );
                        //gameUI.Seperator( 23426 );
                        gameUI.Lable( StringFormat::Small( "Cost Creds: %d", costOfBuildingStation.credits ) );
                        gameUI.Lable( StringFormat::Small( "Cost Enrgy: %d", costOfBuildingStation.energy ) );
                        gameUI.Lable( StringFormat::Small( "Cost Compt: %d", costOfBuildingStation.compute ) );
                    }
                    else if ( gameUI.hoverId == 234 ) {
                        gameUI.Lable( "Trade", true );
                        gameUI.Seperator( 23426 );
                        gameUI.Lable( StringFormat::Small( "Cost Creds: %d", costOfBuildingTrade.credits ) );
                        gameUI.Lable( StringFormat::Small( "Cost Enrgy: %d", costOfBuildingTrade.energy ) );
                        gameUI.Lable( StringFormat::Small( "Cost Compt: %d", costOfBuildingTrade.compute ) );
                    }
                    else if ( gameUI.hoverId == 233 ) {
                        gameUI.Lable( "Solar", true );
                        gameUI.Seperator( 23426 );
                        gameUI.Lable( StringFormat::Small( "Cost Creds: %d", costOfBuildingSolar.credits ) );
                        gameUI.Lable( StringFormat::Small( "Cost Enrgy: %d", costOfBuildingSolar.energy ) );
                        gameUI.Lable( StringFormat::Small( "Cost Compt: %d", costOfBuildingSolar.compute ) );
                    }
                    else if ( gameUI.hoverId == 236 ) {
                        gameUI.Lable( "Compute", true );
                        gameUI.Seperator( 23426 );
                        gameUI.Lable( StringFormat::Small( "Cost Creds: %d", costOfBuildingCompute.credits ) );
                        gameUI.Lable( StringFormat::Small( "Cost Enrgy: %d", costOfBuildingCompute.energy ) );
                        gameUI.Lable( StringFormat::Small( "Cost Compt: %d", costOfBuildingCompute.compute ) );
                    }

                    gameUI.EndVBox();
                }
            }
        }

        if ( onlyBuildingSelected == true ) {
            if ( onlyBuildingSelectedEnt->building.isBuilding == false ) {
                const glm::vec2 s = glm::vec2( 15 );
                if ( core->InputKeyJustPressed( KEY_CODE_Q ) || gameUI.ButtonPix( 242, "Q", ui_RightPanelCenters[ 0 ], s, Colors::SKY_BLUE ) ) {
                    EntityType type = EntityType::Make( EntityType::UNIT_KLAED_WORKER );
                    if ( Vis_CanAfford( localPlayerNumber, GetUnitCostForEntityType( type ) ) == true ) {
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, ( i32 )type );
                        core->AudioPlay( sndKlaedBuidlingQueue );
                    }
                }

                if ( core->InputKeyJustPressed( KEY_CODE_E ) || gameUI.ButtonPix( 243, "E", ui_RightPanelCenters[ 1 ], s, Colors::SKY_BLUE ) ) {
                    EntityType type =  EntityType::Make( EntityType::UNIT_KLAED_SCOUT );
                    if ( Vis_CanAfford( localPlayerNumber, GetUnitCostForEntityType( type ) ) == true ) {
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, ( i32 )type );
                        core->AudioPlay( sndKlaedBuidlingQueue );
                    }
                }

                if ( core->InputKeyJustPressed( KEY_CODE_F ) || gameUI.ButtonPix( 244, "F", ui_RightPanelCenters[ 2 ], s, Colors::SKY_BLUE ) ) {
                    EntityType type =  EntityType::Make( EntityType::UNIT_KLAED_FIGHTER );
                    if ( Vis_CanAfford( localPlayerNumber, GetUnitCostForEntityType( type ) ) == true ) {
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, localPlayerNumber, ( i32 )type );
                        core->AudioPlay( sndKlaedBuidlingQueue );
                    }
                }

                i32 cancelIndex = -1;
                const i32 trainingCount = glm::min( onlyBuildingSelectedEnt->building.trainingQueue.GetCount(), ui_CenterPanelCenters.GetCapcity() );
                for ( i32 trainingIndex = 0; trainingIndex < trainingCount; trainingIndex++ ) {
                    EntityType entType = *onlyBuildingSelectedEnt->building.trainingQueue.Get( trainingIndex );
                    char symbol = GetUnitSymbolForEntityType( entType );
                    char text[2] = {};
                    text[0] = symbol;
                    if ( gameUI.ButtonPix( 245 + trainingIndex, text, ui_CenterPanelCenters[ trainingIndex ], s, Colors::SKY_BLUE ) ) {
                        cancelIndex = trainingIndex;
                    }
                }

                if ( cancelIndex != -1 ) {
                    localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_BUILDING_COMMAND_CANCEL_UNIT, localPlayerNumber, cancelIndex );
                }
            }
        }

        gameUI.UpdateAndRender( core, uiDrawContext );
        gameUI.End();

        isMouseOverUI |= gameUI.mouseOverAnyElements;

        if( isMouseOverUI == false ) {
            if ( isPlacingBuilding  == true ) {
                FpCollider collider = GetColliderForBuildingType ( placingBuildingType );
                FpColliderSetPos( &collider, mousePosWorldFp );

                EntityListFilter * entityFilter = core->MemoryAllocateTransient<EntityListFilter>();
                bool intersectsOtherEnt = !( SimUtil_CanPlaceBuilding( entityFilter, entities, collider ) );

                glm::vec4 ghostCol = glm::vec4( 0.7f, 0.7f, 0.7f, 0.5f );
                if ( intersectsOtherEnt == true ) {
                    ghostCol = glm::vec4( 0.7f, 0.2f, 0.2f, 0.7f );
                }

                SpriteResource * ghostSprite = GetSpriteForBuildingType( placingBuildingType );
                spriteDrawContext->DrawSprite( ghostSprite, 0, mousePosWorld, 0.0f, glm::vec2( 1.0f ), ghostCol );

                if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_2 ) == true ) {
                    isPlacingBuilding = false;
                }

                if ( core->InputMouseButtonJustReleased( MOUSE_BUTTON_1 ) == true ) {
                    if ( intersectsOtherEnt == false ) {
                        isPlacingBuilding = false;
                        core->AudioPlay( sndKlaedBuildBuilding );
                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING, localPlayerNumber, ( i32 )placingBuildingType, mousePosWorldFp );
                    } else {
                        core->AudioPlay( sndCantPlaceBuilding );
                    }
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
                localActionBuffer.AddAction( (i32)MapActionType::PLAYER_SELECTION, localPlayerNumber, viewDragSelection, EntitySelectionChange::SET );
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
            if ( ent->solarNumber == viewSolarNumber ) {
                glm::vec2 drawPos = ent->visPos;
                f32 drawOri = ent->visOri;

                if ( ent->visSelectedBy.Contains( localPlayerNumber ) && ent->playerNumber == localPlayerNumber && ent->visHasDest == true ) {
                    spriteDrawContext->DrawTexture( sprMoveLocation, ent->visDestPos, 0.0f );
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
                        const i32 timeToTrainTurns = GetUnitTrainTimeForEntityType( trainingEnt );
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

        if ( isMouseOverUI == false && core->InputMouseButtonJustPressed( MOUSE_BUTTON_2 ) == true ) {
            bool inputMade = false;
            for( i32 entityIndexA = 0; entityIndexA < entityCount; entityIndexA++ ) {
                const SimEntity * ent = entities[ entityIndexA ];
                if( ent->isSelectable == true ) {
                    const Collider2D selectionCollider = ent->GetWorldSelectionCollider();
                    if( selectionCollider.Contains( mousePosWorld ) ) {
                        if ( IsUnitType( ent->type ) ) {
                            if ( ent->teamNumber != localPlayerTeamNumber ) {
                                localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK, localPlayerNumber, ent->handle );
                            }
                            else {
                                // @TODO: Follow
                                break;
                            }
                        } else if ( IsBuildingType( ent->type ) == true ) {
                            if ( ent->teamNumber != localPlayerTeamNumber ) {
                                localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_UNIT_COMMAND_ATTACK, localPlayerNumber, ent->handle );
                            } else if ( ent->building.isBuilding == true ) {
                                for ( i32 entityIndexB = 0; entityIndexB < entities.GetCount(); entityIndexB++ ) {
                                    const SimEntity * otherEnt = *entities.Get( entityIndexB );
                                    if ( otherEnt->playerNumber == localPlayerNumber && IsWorkerType( otherEnt->type ) == true ) {
                                        localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_EXISTING_BUILDING, localPlayerNumber, ent->handle );
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
                localActionBuffer.AddAction( (i32)MapActionType::SIM_ENTITY_UNIT_COMMAND_MOVE, localPlayerNumber, mousePosWorldFp );
                for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
                    SimEntity * visEnt = entities[ entityIndex ];
                    if ( visEnt->visSelectedBy.Contains( localPlayerNumber ) == true && visEnt->playerNumber == localPlayerNumber ) {
                        visEnt->visHasDest = true;
                        visEnt->visDestPos = mousePosWorld;

                        // @HACK: We should use a featured unit
                        if ( visEnt->sndMove != nullptr ) {
                            core->AudioPlay( visEnt->sndMove );
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

        if ( aiThinker.clientHasAuth == true ) {
            aiThinker.Think( entities );
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
                case EntityType::UNIT_KLAED_WORKER:
                {
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

                    entity->unit.range = ToFP( 60 );
                    entity->unit.speed = Fp( 25 );

                } break;
                case EntityType::UNIT_KLAED_SCOUT:
                {
                    entity->spriteUnit.base = sprKlaedScoutBase;
                    entity->spriteUnit.engine = sprKlaedScoutEngine;
                    entity->spriteUnit.shield = sprKlaedScoutShield;
                    entity->spriteUnit.weapons = sprKlaedScoutWeapon;
                    entity->spriteUnit.destruction = sprKlaedScoutDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->sndHello = sndKlaedScoutHello;
                    entity->sndMove = sndKlaedScoutMove;
                    if ( playSpawningSounds == true ) {
                        core->AudioPlay( sndKlaedScoutBuilt );
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
                case EntityType::UNIT_KLAED_FIGHTER:
                {
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
                case EntityType::UNIT_KLAED_TORPEDO: {
                    entity->spriteUnit.base = sprKlaedTorpedoBase;
                    entity->spriteUnit.engine = sprKlaedTorpedoEngine;
                    entity->spriteUnit.shield = sprKlaedTorpedoShield;
                    entity->spriteUnit.weapons = sprKlaedTorpedoWeapon;
                    entity->spriteUnit.destruction = sprKlaedTorpedoDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 56, 16 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    // @TODO: STATS + WEAPONS
                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;
                    entity->unit.speed = Fp( 35 );
                    entity->unit.range = ToFP( 400 );
                } break;
                case EntityType::UNIT_KLAED_FRIGATE: {
                    entity->spriteUnit.base = sprKlaedFrigateBase;
                    entity->spriteUnit.engine = sprKlaedFrigateEngine;
                    entity->spriteUnit.shield = sprKlaedFrigateShield;
                    entity->spriteUnit.weapons = sprKlaedFrigateWeapon;
                    entity->spriteUnit.destruction = sprKlaedFrigateDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.rad = 25.0f;

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 22.0f );

                    // @TODO: STATS + WEAPONS
                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;
                    entity->unit.speed = Fp( 35 );
                    entity->unit.range = ToFP( 400 );
                } break;
                case EntityType::UNIT_KLAED_BATTLE_CRUISER: {
                    entity->spriteUnit.base = sprKlaedBattleCruiserBase;
                    entity->spriteUnit.engine = sprKlaedBattleCruiserEngine;
                    entity->spriteUnit.shield = sprKlaedBattleCruiserShield;
                    entity->spriteUnit.weapons = sprKlaedBattleCruiserWeapon;
                    entity->spriteUnit.destruction = sprKlaedBattleCruiserDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.rad = 50.0f;

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 45.0f );

                    // @TODO: STATS + WEAPONS
                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;
                    entity->unit.speed = Fp( 35 );
                    entity->unit.range = ToFP( 400 );
                } break;
                case EntityType::UNIT_KLAED_DREADNOUGHT: {
                    entity->spriteUnit.base = sprKlaedDreadnoughtBase;
                    entity->spriteUnit.engine = sprKlaedDreadnoughtEngine;
                    entity->spriteUnit.shield = sprKlaedDreadnoughtShield;
                    entity->spriteUnit.weapons = sprKlaedDreadnoughtWeapon;
                    entity->spriteUnit.destruction = sprKlaedDreadnoughtDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.rad = 50.0f;

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 45.0f );

                    // @TODO: STATS + WEAPONS
                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;
                    entity->unit.speed = Fp( 35 );
                    entity->unit.range = ToFP( 400 );
                } break;
                case EntityType::UNIT_NAIRAN_WORKER:
                {
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
                case EntityType::UNIT_NAIRAN_SCOUT:
                {
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
                case EntityType::UNIT_NAIRAN_FIGHTER:
                {
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
                case EntityType::UNIT_NAUTOLAN_WORKER: {
                    entity->spriteUnit.base = sprNautolanWorkerBase;
                    entity->spriteUnit.engine = sprNautolanWorkerEngine;
                    entity->spriteUnit.destruction = sprNautolanWorkerDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 26 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.speed = Fp( 25 );
                    entity->unit.range = ToFP( 50 );
                } break;
                case EntityType::UNIT_NAUTOLAN_SCOUT: {
                    entity->spriteUnit.base = sprNautolanScoutBase;
                    entity->spriteUnit.engine = sprNautolanScoutEngine;
                    entity->spriteUnit.shield = sprNautolanScoutShield;
                    entity->spriteUnit.weapons = sprNautolanScoutWeapon;
                    entity->spriteUnit.destruction = sprNautolanScoutDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 26 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.speed = Fp( 75 );
                    entity->unit.range = ToFP( 400 );
                } break;
                case EntityType::UNIT_NAUTOLAN_FIGHTER: {
                    entity->spriteUnit.base = sprNautolanFighterBase;
                    entity->spriteUnit.engine = sprNautolanFighterEngine;
                    entity->spriteUnit.shield = sprNautolanFighterShield;
                    entity->spriteUnit.weapons = sprNautolanFighterWeapon;
                    entity->spriteUnit.destruction = sprNautolanFighterDestruction;

                    entity->spriteAnimator.SetSpriteIfDifferent( entity->spriteUnit.base, false );
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 26, 26 ) );

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.rad = Fp( 13 );

                    entity->maxHealth = 150;
                    entity->currentHealth = entity->maxHealth;

                    entity->unit.speed = Fp( 75 );
                    entity->unit.range = ToFP( 400 );
                } break;
                case EntityType::BULLET_KLARD_BULLET: 
                {
                    entity->sprBullet.base = sprKlaedProjectileBullet;
                    entity->sprBullet.destruction = sprKlaedProjectileBulletHit;
                    entity->spriteAnimator.SetSpriteIfDifferent( entity->sprBullet.base, true );

                    entity->sndDestructions.Add( sndKlaedProjectileBulletDestruction1 );
                    entity->sndDestructions.Add( sndKlaedProjectileBulletDestruction2 );
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

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.pos = Fp2( 0, 0 );
                    entity->collisionCollider.circle.rad = Fp( 270 );

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

                    entity->collisionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->collisionCollider.circle.pos = Fp2( 0, 0 );
                    entity->collisionCollider.circle.rad = Fp( 125 );

                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                    entity->planet.placements.Add( PlanetPlacementType::OPEN );
                } break;
                case EntityType::BUILDING_STATION:
                {
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );
                    if ( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingStationBlueOff, false );
                        entity->spriteBank[ 0 ] =  sprBuildingStationBlueOff;
                        entity->spriteBank[ 1 ] = sprBuildingStationBlueOn;
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingStationRedOff, false );
                        entity->spriteBank[ 0 ] = sprBuildingStationRedOff;
                        entity->spriteBank[ 1 ] = sprBuildingStationRedOn;
                    }   

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 64 ) );

                    entity->collisionCollider = colBuildingStation;

                    entity->maxHealth = 200;
                    entity->currentHealth = entity->maxHealth;

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 60 );

                } break;
                case EntityType::BUILDING_TRADE:
                {
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );
                    if ( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingTradeBlue, false );
                        entity->spriteBank[ 0 ] = sprBuildingTradeBlue;
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingTradeRed, false );
                        entity->spriteBank[ 0 ] = sprBuildingTradeRed;
                    }

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 64 ) );

                    entity->collisionCollider = colBuildingTrade;

                    entity->maxHealth = 200;
                    entity->currentHealth = entity->maxHealth;

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 60 );

                    const i32 credsPerMinute = 60;
                    const i32 credTickRate = 20;
                    entity->building.timeToGiveCreditsTurns = SecondsToTurns( 60 ) / credTickRate;
                    entity->building.amountToGiveCredits = credsPerMinute / credTickRate;

                } break;
                case EntityType::BUILDING_SOLAR_ARRAY:
                {
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    if ( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingSolarBlue, false );
                        entity->spriteBank[ 0 ] = sprBuildingSolarBlue;
                    } else {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingSolarRed, false );
                        entity->spriteBank[ 0 ] = sprBuildingSolarRed;
                    }

                    SpriteResource * mainSprite = spawnInfo->teamNumber.value == 1 ? sprBuildingSolarBlue : sprBuildingSolarRed;
                    entity->spriteAnimator.SetSpriteIfDifferent( mainSprite, false );

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_AXIS_BOX;
                    entity->visSelectionCollider.box.CreateFromCenterSize( glm::vec2( 0 ), glm::vec2( 64, 32 ) );

                    entity->collisionCollider = colBuildingSolar;

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
                    entity->selectionAnimator.SetSpriteIfDifferent( sprSmolSelection, false );

                    if( spawnInfo->teamNumber.value == 1 ) {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingComputeBlueOff, false );
                        entity->spriteBank[ 0 ] = sprBuildingComputeBlueOff;
                        entity->spriteBank[ 1 ] = sprBuildingComputeBlueOn;
                    }
                    else {
                        entity->spriteAnimator.SetSpriteIfDifferent( sprBuildingComputeRedOff, false );
                        entity->spriteBank[ 0 ] = sprBuildingComputeRedOff;
                        entity->spriteBank[ 1 ] = sprBuildingComputeRedOn;
                    }

                    entity->isSelectable = true;
                    entity->visSelectionCollider.type = COLLIDER_TYPE_CIRCLE;
                    entity->visSelectionCollider.circle.rad = 22;

                    entity->collisionCollider = colBuildingCompute;

                    entity->maxHealth = 75;
                    entity->currentHealth = entity->maxHealth;

                    entity->building.isBuilding = true;
                    entity->building.timeToBuildTurns = SecondsToTurns( 40 );

                    const i32 computePerMinute = 30;
                    const i32 computeTickRate = 5;
                    entity->building.timeToGiveComputeTurns = SecondsToTurns( 60 ) / computeTickRate;
                    entity->building.amountToGiveCompute = computePerMinute / computeTickRate;
                } break;
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
        MoneySet buildingCost = GetBuildingCostForEntityType( type );
        SimAction_Pay( playerNumber, buildingCost );

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        const i32 entityCount = simAction_ActiveEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < entityCount; entityIndex++ ) {
            SimEntity * ent = simAction_ActiveEntities[ entityIndex ];
            if ( ent->playerNumber == playerNumber && IsWorkerType( ent->type ) && ent->selectedBy.Contains( playerNumber ) ) {
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
                    if ( ent->playerNumber == playerNumber && IsWorkerType( ent->type ) == true && ent->selectedBy.Contains( playerNumber ) ) {
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
            SimAction_Pay( playerNumber, costOfPlacementCredit );
        }
        if ( placementType == PlanetPlacementType::ENERGY_GENERATOR ) {
            SimAction_Pay( playerNumber, costOfPlacementSolar );
        }
        if ( placementType == PlanetPlacementType::COMPUTE_GENERATOR ) {
            SimAction_Pay( playerNumber, costOfPlacementCompute );
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

            MoneySet monies = GetUnitCostForEntityType( type );
            SimAction_Pay( playerNumber, monies );

            if ( building.trainingQueue.IsEmpty() == true ) {
                building.turn = 0;
            }
            if ( building.trainingQueue.IsFull() == false ) {
                building.trainingQueue.Enqueue( type );
            }
        }
    }

    void SimMap::SimAction_BuildingCancelUnit( PlayerNumber * playerNumberPtr , i32 * indexPtr ) {
        PlayerNumber playerNumber = *playerNumberPtr;
        i32 index = *indexPtr;

        // @SPEED:
        simAction_ActiveEntities.Clear( false );
        entityPool.GatherActiveObjs( simAction_ActiveEntities );

        simAction_EntityFilter.Begin( &simAction_ActiveEntities )->
            SimSelectedBy( playerNumber )->
            OwnedBy( playerNumber )->
            Type( EntityType::BUILDING_STATION )->
            End(); 


        SimEntity * stationEnt = simAction_EntityFilter.result[0];
        Assert( stationEnt->type == EntityType::BUILDING_STATION );
        Assert( index < stationEnt->building.trainingQueue.GetCount() );

        stationEnt->building.trainingQueue.RemoveIndex( index );
        if ( index == 0 ) {
            stationEnt->building.turn = 0;
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

                // TODO: CVar:
                #if ATTO_DEBUG
                if ( true ) {
                    targetEnt->building.turn = 0;
                    targetEnt->building.isBuilding = false;
                    return;
                }
                #endif

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

    void SimMap::SimAction_Test( GrowableList<i32> *test ) {
        int a = 2;
    }

    static void SimTick_SelfUpdate( const ConstEntList * entities, const EntPool * entityPool, i32 index, SimEntity * ent ) {
        const fp maxForce = Fp( 20.0f );
        const fp invMass = Fp( 1.0f / 10.0f );

        ZeroStruct( ent->actions );

        switch( ent->type ) {
            case EntityType::UNIT_KLAED_WORKER:
            case EntityType::UNIT_KLAED_SCOUT:
            case EntityType::UNIT_KLAED_FIGHTER:
            case EntityType::UNIT_KLAED_TORPEDO:
            case EntityType::UNIT_KLAED_FRIGATE:
            case EntityType::UNIT_NAIRAN_WORKER:
            case EntityType::UNIT_KLAED_BATTLE_CRUISER:
            case EntityType::UNIT_KLAED_DREADNOUGHT:
            case EntityType::UNIT_NAIRAN_SCOUT:
            case EntityType::UNIT_NAIRAN_FIGHTER:
            case EntityType::UNIT_NAUTOLAN_WORKER:
            case EntityType::UNIT_NAUTOLAN_SCOUT:
            case EntityType::UNIT_NAUTOLAN_FIGHTER:
            {
                Unit & unit = ent->unit;

                if( ent->currentHealth == 0 ) { // HACK: I think this should go into the apply damange call??? Maybe something to do with death animations ?
                    ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                    break;
                }

                FpCollider entCollider = ent->GetWorldCollisionCollider();

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
                                        ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_SPAWN, ( i32 )unit.weaponType, ent->playerNumber, ent->teamNumber, ent->solarNumber, spawnPos, spawnOri, spawnVel );
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
                    if ( IsWorkerType( ent->type ) == true ) {
                        const SimEntity * target = entityPool->Get( unit.command.targetEnt );
                        if ( target != nullptr && target->building.isBuilding == true ) {
                            fp dist2 = FpDistance2( target->pos, ent->pos );
                            if ( dist2 <= unit.range * unit.range ) {
                                // Build
                                ent->vel = Fp2( 0, 0 );
                                ent->navigator.hasDest = false;
                                ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_APPLY_CONSTRUCTION, target->handle );
                            } else {
                                // Move towards building
                                ent->navigator.hasDest = true;
                                ent->navigator.dest = target->pos;
                                ent->navigator.slowRad = Fp( 100 );
                            }
                        } else {
                            unit.command.type = UnitCommandType::IDLE;
                            unit.command.targetPos = {};
                            unit.command.targetEnt = {};
                        }
                    }
                }

                fp2 steering = Fp2( 0, 0 );

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

                    if ( dist < Fp( 50 ) ) {
                        ent->navigator.hasDest = false;
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

                    steering = steering + desiredVel - ent->vel;
                }

                // Avoidance
                for ( i32 entityIndexB = 0; entityIndexB < entities->GetCount(); entityIndexB++ ) {
                    const SimEntity * otherEnt = *entities->Get( entityIndexB );
                    const bool correctType = IsUnitType( otherEnt->type ) == true || IsBuildingType( otherEnt->type );
                    if ( entityIndexB != index && correctType && otherEnt->teamNumber == ent->teamNumber ) {
                        FpCollider otherEntCollider = otherEnt->GetWorldCollisionCollider();
                        fp dist = FpColliderSurfaceDistance( entCollider, otherEntCollider );
                        fp dd = Fp( 10 );
                        //fp dist = FpDistance( otherEnt->pos, ent->pos );
                        if( dist < dd ) {
                            fp2 dir = FpNormalize( ent->pos - otherEnt->pos );
                            steering = steering + dir * unit.speed * FpClamp( ( 1 - dist / ( dd / Fp( 2 ) ) ), Fp( 0 ), Fp( 1 ) );
                        }
                    }
                }

                steering = FpTruncateLength( steering, maxForce );
                steering = steering * invMass;

                ent->vel = FpTruncateLength( ent->vel + steering, unit.speed );
                
                ent->vel = ent->vel * Fp( 0.95f );

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
                    ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_DESTROY, ent->handle );
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
                            ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_APPLY_DAMAGE, bullet.damage, otherEnt->handle );
                            ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_DESTROY, ent->handle );
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
                                ent->actions.AddAction( (i32)MapActionType::SIM_MAP_MONIES_GIVE_CREDITS, ent->playerNumber, amountToGive );
                                planet.placementsTurns[placementIndex] = 0;
                            }
                        } break;
                        case PlanetPlacementType::ENERGY_GENERATOR: {
                            const i32 ticker = 5;
                            const i32 pm = SecondsToTurns( 60 ) / ticker;
                            const i32 amountToGive = 50 / ticker;
                            const i32 turn = planet.placementsTurns[placementIndex]++;

                            if ( turn >= pm ) {
                                ent->actions.AddAction( (i32)MapActionType::SIM_MAP_MONIES_GIVE_ENERGY, ent->playerNumber, amountToGive );
                                planet.placementsTurns[placementIndex] = 0;
                            }
                        } break;
                        case PlanetPlacementType::COMPUTE_GENERATOR: {
                            const i32 ticker = 5;
                            const i32 pm = SecondsToTurns( 60 ) / ticker;
                            const i32 amountToGive = 25 / ticker;
                            const i32 turn = planet.placementsTurns[placementIndex]++;

                            if ( turn >= pm ) {
                                ent->actions.AddAction( (i32)MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE, ent->playerNumber, amountToGive );
                                planet.placementsTurns[placementIndex] = 0;
                            }
                        } break;
                    }
                }
            } break;
            case EntityType::BUILDING_STATION:
            case EntityType::BUILDING_TRADE:
            case EntityType::BUILDING_SOLAR_ARRAY:
            case EntityType::BUILDING_COMPUTE:
            {
                if( ent->currentHealth == 0 ) { // HACK: I think this should go into the apply damange call??? Maybe something to do with death animations ?
                    ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_DESTROY, ent->handle );
                    break;
                }

                Building & building = ent->building;

                if ( building.isBuilding == false ) {
                    building.turn++;
                    if ( ent->type == EntityType::BUILDING_SOLAR_ARRAY ) {
                        if ( building.turn >= building.timeToGiveEnergyTurns ) {
                            building.turn = 0;
                            ent->actions.AddAction( (i32)MapActionType::SIM_MAP_MONIES_GIVE_ENERGY, ent->playerNumber, ent->building.amountToGiveEnergy );
                        }
                    } else if ( ent->type == EntityType::BUILDING_COMPUTE ) {
                        if ( building.turn >= building.timeToGiveComputeTurns ) {
                            building.turn = 0;
                            ent->actions.AddAction( (i32)MapActionType::SIM_MAP_MONIES_GIVE_COMPUTE, ent->playerNumber, ent->building.amountToGiveCompute );
                        }
                    }
                    else if ( ent->type == EntityType::BUILDING_STATION ) {
                        if ( building.trainingQueue.IsEmpty() == false ) {
                            EntityType trainingEnt = *building.trainingQueue.Peek();
                            const i32 timeToTrainTurns = GetUnitTrainTimeForEntityType( trainingEnt );
                            if ( building.turn == timeToTrainTurns ) {
                                fp2 spawnLocation = ent->pos - Fp2( 0, 32 ); // @HACK
                                fp spawnOri = FP_PI; // @HACK
                                ent->actions.AddAction( (i32)MapActionType::SIM_ENTITY_SPAWN, ( i32 )trainingEnt, ent->playerNumber, ent->teamNumber, ent->solarNumber, spawnLocation, spawnOri, Fp2( 0, 0 ) );
                                building.turn = 0;
                                building.trainingQueue.Dequeue();
                            }
                        }
                    } else if ( ent->type == EntityType::BUILDING_TRADE ) {
                        if ( building.turn >= building.timeToGiveCreditsTurns ) {
                            building.turn = 0;
                            ent->actions.AddAction( (i32)MapActionType::SIM_MAP_MONIES_GIVE_CREDITS, ent->playerNumber, ent->building.amountToGiveCredits );
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
            Sim_ApplyActions( &turn1->playerActions );
            Sim_ApplyActions( &turn1->aiActions );
        }
        
        if( turn2 != nullptr ) {
            Sim_ApplyActions( &turn2->playerActions );
            Sim_ApplyActions( &turn2->aiActions );
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

            RpcHolder * holder = GlobalRpcTable[ (i32)actionType ];
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

    bool SimMap::SimUtil_CanPlaceBuilding( EntityListFilter * entityFilter, EntList & entities, FpCollider collider ) {
        EntityType types[] = { EntityType::BUILDING_STATION, EntityType::BUILDING_TRADE, EntityType::BUILDING_SOLAR_ARRAY, EntityType::BUILDING_COMPUTE, EntityType::PLANET, EntityType::STAR };
        entityFilter->Begin( &entities )->Types( types, ArrayCount( types ) )->End();
        const i32 otherEntCount = entityFilter->result.GetCount();
        for ( i32 otherEntIndex = 0; otherEntIndex < otherEntCount; otherEntIndex++ ) {
            SimEntity * otherEnt = entityFilter->result[otherEntIndex];
            FpCollider otherCollider = otherEnt->GetWorldCollisionCollider();
            if ( FpColliderIntersects( collider, otherCollider ) == true ) {
                return false;
            }
        }
        return true;
    }

    void SimMap::SimAction_Pay( PlayerNumber playerNumber, MoneySet costSet ) {
        if( SimUtil_CanAfford( playerNumber, costSet ) == false ) {
            return;
        }

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
            core->AudioPlay( sndKlaedNotEnoughResources );
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
                
                RpcHolder * holder = GlobalRpcTable[ (i32)actionType ];
                LargeString rpcString = holder->Log( (char *)data );
                i32 lastCallSize = holder->GetLastCallSize();
                actionOffset += lastCallSize;

                const char * actionName = MapActionTypeStrings[ (i32)actionType ];
                core->LogOutput( LogLevel::INFO, "%s :: ( %s )", actionName, rpcString.GetCStr() );
            }
        }
    }

    bool AIThinker::FindBuildingLocation( EntList & activeEntities, EntityType buildingType, fp2 basePos, fp2 & pos ) {
        FpCollider collider = GetColliderForBuildingType( buildingType );

        EntityListFilter * entityFilter = core->MemoryAllocateTransient<EntityListFilter>();
        for ( i32 it = 0; it < 10; it++ ) {
            // HACK: Scary random.
            f32 px = Random::Float( -1, 1 );
            f32 py = Random::Float( -1, 1 );
            glm::vec2 d = glm::normalize( glm::vec2( px, py ) ) * 200.0f;
            pos = basePos + Fp2( d );
            FpColliderSetPos( &collider, pos );

            if ( map->SimUtil_CanPlaceBuilding( entityFilter, activeEntities, collider ) == true ) {
                return true;
            }
        }

        return false;
    }

    void AIThinker::Think( EntList & activeEntities ) {
        if ( nextThinkTurn != map->turnNumber ) {
            return;
        }

        nextThinkTurn = map->turnNumber + map->syncQueues.GetSlidingWindowWidth() + 1;

        SimEntity * idleWorker = nullptr;
        std::vector<SimEntity *> workers;
        std::vector<SimEntity *> scouts;
        std::vector<SimEntity *> fighters;
        std::vector<SimEntity *> bldStations;
        std::vector<SimEntity *> bldCreds;
        std::vector<SimEntity *> bldEngys;
        std::vector<SimEntity *> bldComps;

        const i32 activeEntityCount = activeEntities.GetCount();
        for ( i32 entityIndex = 0; entityIndex < activeEntityCount; entityIndex++ ) {
            SimEntity * ent = activeEntities[entityIndex];
            if ( ent->playerNumber == aiNumber ) {
                if ( IsWorkerType( ent->type ) == true ) {
                    workers.push_back( ent );
                    if ( idleWorker == nullptr && ent->unit.command.type == UnitCommandType::IDLE ) {
                        idleWorker = ent;
                    }
                } else if ( ent->type == EntityType::UNIT_NAUTOLAN_SCOUT ) {
                    scouts.push_back( ent );
                }
                else if ( ent->type == EntityType::UNIT_NAUTOLAN_FIGHTER ) {
                    fighters.push_back( ent );
                }
                else if ( ent->type == EntityType::BUILDING_STATION ) {
                    bldStations.push_back( ent );
                }
                else if ( ent->type == EntityType::BUILDING_TRADE ) {
                    bldCreds.push_back( ent );
                }
                else if ( ent->type == EntityType::BUILDING_SOLAR_ARRAY ) {
                    bldEngys.push_back( ent );
                }
                else if ( ent->type == EntityType::BUILDING_COMPUTE ) {
                    bldComps.push_back( ent );
                }
            }
        }

        const i32 workerCount = ( i32 )workers.size();
        const i32 scoutCount = ( i32 )scouts.size();
        const i32 fighterCount = ( i32 )fighters.size();
        const i32 bldStationCount = ( i32 )bldStations.size();
        const i32 bldCredCount = ( i32 )bldCreds.size();
        const i32 bldEngyCount = ( i32 )bldEngys.size();
        const i32 bldCompCount = ( i32 )bldComps.size();

        i32 stationRatio = ( ( bldCredCount + bldEngyCount + bldCompCount ) / 5 ) + 2;

        f32 credRatio = (f32)bldCredCount;
        f32 engyRatio = (f32)bldEngyCount;
        if ( bldCompCount != 0 ) {
            credRatio = (f32)bldCredCount / (f32)bldCompCount;
            engyRatio = (f32)bldEngyCount / (f32)bldCompCount;
        }

        bool builtBuilding = false;
        if ( idleWorker != nullptr ) {
            EntityType stationBuildingType = EntityType::Make( EntityType::BUILDING_STATION );
            EntityType tradeBuildingType = EntityType::Make( EntityType::BUILDING_TRADE );
            EntityType solarBuildingType = EntityType::Make( EntityType::BUILDING_SOLAR_ARRAY );
            EntityType computeBuildingType = EntityType::Make( EntityType::BUILDING_COMPUTE );
            if ( stationRatio > bldStationCount && map->SimUtil_CanAfford( aiNumber, GetBuildingCostForEntityType( stationBuildingType ) ) == true ) {
                fp2 buildingPos = {};
                if ( FindBuildingLocation( activeEntities, stationBuildingType, idleWorker->pos, buildingPos ) == true ) {
                    selection.Clear();
                    selection.Add( idleWorker->handle );
                    actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                    actionBuffer.ConstructBuilding( aiNumber, stationBuildingType, buildingPos );
                    lastBuildingTurn = map->turnNumber;
                    builtBuilding = true;
                }
            } else if ( credRatio < 2.0f && map->SimUtil_CanAfford( aiNumber, GetBuildingCostForEntityType( tradeBuildingType ) ) == true ) {
                fp2 buildingPos = {};
                if ( FindBuildingLocation( activeEntities, tradeBuildingType, idleWorker->pos, buildingPos ) == true ) {
                    selection.Clear();
                    selection.Add( idleWorker->handle );
                    actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                    actionBuffer.ConstructBuilding( aiNumber, tradeBuildingType, buildingPos );
                    lastBuildingTurn = map->turnNumber;
                    builtBuilding = true;
                }
            } else if ( engyRatio < 3.0f && map->SimUtil_CanAfford( aiNumber, GetBuildingCostForEntityType( solarBuildingType ) ) == true ) {
                fp2 buildingPos = {};
                if ( FindBuildingLocation( activeEntities, solarBuildingType, idleWorker->pos, buildingPos ) == true ) {
                    selection.Clear();
                    selection.Add( idleWorker->handle );
                    actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                    actionBuffer.ConstructBuilding( aiNumber, solarBuildingType, buildingPos );
                    lastBuildingTurn = map->turnNumber;
                    builtBuilding = true;
                }
            } else if ( map->SimUtil_CanAfford( aiNumber, GetBuildingCostForEntityType( computeBuildingType ) ) == true ) {
                fp2 buildingPos = {};
                if ( FindBuildingLocation( activeEntities, computeBuildingType, idleWorker->pos, buildingPos ) == true ) {
                    selection.Clear();
                    selection.Add( idleWorker->handle );
                    actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                    actionBuffer.ConstructBuilding( aiNumber, computeBuildingType, buildingPos );
                    lastBuildingTurn = map->turnNumber;
                    builtBuilding = true;
                }
            }
        }

        EntityType workerType = EntityType::Make( EntityType::UNIT_NAUTOLAN_WORKER );
        EntityType scoutType = EntityType::Make( EntityType::UNIT_NAUTOLAN_SCOUT );
        EntityType fighterType = EntityType::Make( EntityType::UNIT_NAUTOLAN_FIGHTER );

        if ( builtBuilding == false && map->turnNumber - lastBuildingTurn < 400 ) {
            for ( i32 stationIndex = 0; stationIndex < bldStationCount; stationIndex++ ) {
                SimEntity * station = bldStations[stationIndex];
                if ( station->building.trainingQueue.IsEmpty() == true ) {
                    if ( workerCount < 4 && map->SimUtil_CanAfford( aiNumber, GetUnitCostForEntityType( workerType ) ) == true ) {
                        selection.Clear();
                        selection.Add( station->handle );
                        actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                        actionBuffer.TrainUnit( aiNumber, EntityType::UNIT_NAUTOLAN_WORKER );
                    } else if ( scoutCount < 10 && map->SimUtil_CanAfford( aiNumber, GetUnitCostForEntityType( scoutType ) ) == true ) {
                        selection.Clear();
                        selection.Add( station->handle );
                        actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                        actionBuffer.TrainUnit( aiNumber, EntityType::UNIT_NAUTOLAN_SCOUT );
                    } else if ( fighterCount < 20 && map->SimUtil_CanAfford( aiNumber, GetUnitCostForEntityType( fighterType ) ) == true ) {
                        selection.Clear();
                        selection.Add( station->handle );
                        actionBuffer.PlayerSelect( aiNumber, selection, EntitySelectionChange::SET );
                        actionBuffer.TrainUnit( aiNumber, EntityType::UNIT_NAUTOLAN_FIGHTER );
                    }
                }
            }
        }

    }

}

