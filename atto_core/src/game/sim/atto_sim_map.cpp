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
        } else if ( type == EntityType::UNIT_NAIRAN_WORKER) {
            return cfgNairanWorkerTrainTimeTurns;
        } else if ( type == EntityType::UNIT_NAIRAN_SCOUT ) {
            return cfgNairanScoutTrainTimeTurns;
        } else if ( type == EntityType::UNIT_NAIRAN_FIGHTER ) {
            return cfgNairanFighterTrainTimeTurns;
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
        } else if ( type == EntityType::UNIT_NAIRAN_WORKER) {
            return cfgNairanWorkerCost;
        } else if ( type == EntityType::UNIT_NAIRAN_SCOUT ) {
            return cfgNairanScoutCost;
        } else if ( type == EntityType::UNIT_NAIRAN_FIGHTER ) {
            return cfgNairanFighterCost;
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
        } else if ( type == EntityType::UNIT_NAIRAN_WORKER ) {
            return 'Q';
        } else if ( type == EntityType::UNIT_NAIRAN_SCOUT ) {
            return 'E';
        } else if ( type == EntityType::UNIT_NAIRAN_FIGHTER ) {
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

    bool EntityListFilter::ContainsOnlyWorker() {
        const i32 entCount = result.GetCount();
        if( entCount == 0 ) {
            return false;
        }

        for ( i32 entityIndex = 0; entityIndex < entCount; entityIndex++ ) {
            SimEntity * ent = *result.Get( entityIndex );
            if ( IsWorkerType( ent->type ) == false ) {
                return false;
            }
        }

        return true;
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


        PlayerNumber p0 = PlayerNumber::Create( 0 );
        TeamNumber   t0 = TeamNumber::Create( 0 );
        PlayerNumber p1 = PlayerNumber::Create( 1 );
        PlayerNumber p2 = PlayerNumber::Create( 2 );
        PlayerNumber p3 = PlayerNumber::Create( 3 );

        TeamNumber t1 = TeamNumber::Create( 1 );
        TeamNumber t2 = TeamNumber::Create( 2 );

        SolarNumber s1 = SolarNumber::Create( 1 );



    }

    struct RollBackEnt {
        f32 pt;
        fp2 lp;
        fp2 p;
        fp2 dest;
    };

    struct GameState {
        int turnNumber;
        RollBackEnt e1;
        RollBackEnt e2;
    };

    void SimMap::Update( Core * core, f32 dt ) {
        //ScopedClock timer( "Update", core );
        static GameState gs = {};
        static SyncQueues sync = {};
        static bool inited = false;
        if ( inited == false ) {
            sync.Start( 200, SIM_DT_FLOAT );
            inited = true;
            gs.turnNumber = 1;
            gs.e1.p = Fp2( 200, 200 );
            gs.e1.dest = gs.e1.p;
            gs.e2.p = Fp2( 400, 200 );
            gs.e2.dest = gs.e2.p;
        }

        DrawContext * spriteDrawContext = core->RenderGetDrawContext( 0, true );
        spriteDrawContext->SetCameraDims( 640, 360 );

        const glm::vec2 mousePosPix = core->InputMousePosPixels();
        const glm::vec2 mousePosWorld = spriteDrawContext->ScreenPosToWorldPos( mousePosPix );

        static bool mousePressed = false;
        if ( core->InputMouseButtonJustPressed( MouseButton::MOUSE_BUTTON_2 ) ) {
            mousePressed = true;
        }

        if( core->NetworkIsConnected() == true ) {
            dtAccumulator += dt;
            if ( dtAccumulator > SIM_DT_FLOAT ) {
                dtAccumulator -= SIM_DT_FLOAT;

                NetworkMessage & msg = *core->MemoryAllocateTransient< NetworkMessage >();
                while( core->NetworkRecieve( msg ) ) {
                    switch( msg.type ) {
                        case NetworkMessageType::MAP_TURN:
                        {
                           
                        } break;
                    }
                }

                ZeroStruct( msg );
                msg.type = NetworkMessageType::MAP_TURN;
                NetworkMessagePush( msg, localMapTurn );

                core->NetworkSend( msg );

                ZeroStruct( localMapTurn );
                ZeroStruct( localActionBuffer );
            }
        }
        else {
            dtAccumulator += dt;
            if ( dtAccumulator >= SIM_DT_FLOAT ) {
                dtAccumulator = 0;

                sync.AddTurn(localPlayerNumber, localMapTurn);
                sync.AddTurn(PlayerNumber::Create(2), localMapTurn);
                sync.CanTurn();
                MapTurn *turn = sync.GetNextTurn( localPlayerNumber );
                if ( turn->madeMove == true ) {
                    gs.e1.dest = turn->moveLoc;
                    ATTOINFO( "t = %f ", (core->GetTheCurrentTime() - turn->moveTime) * 1000.0f) ;
                }

                sync.FinishTurn();
                ZeroStruct( localMapTurn );

                if ( FpDistance( gs.e1.p, gs.e1.dest ) > Fp( 5 ) ) {
                    gs.e1.pt = 0.0f;
                    gs.e1.lp = gs.e1.p;
                    gs.e1.p = gs.e1.p + FpNormalize( gs.e1.dest - gs.e1.p ) * Fp( 100 ) * SIM_DT;
                }
                if ( FpDistance( gs.e2.p, gs.e2.dest ) > Fp( 5 ) ) {
                    gs.e2.pt = 0.0f;
                    gs.e2.lp = gs.e2.p;
                    gs.e2.p = gs.e2.p + FpNormalize( gs.e2.dest - gs.e2.p ) * Fp( 100 ) * SIM_DT;
                }
            }
        }

        if ( mousePressed ) {
            mousePressed = false;
            if ( localPlayerNumber.value == 1 ) {
                localMapTurn.madeMove = true;
                localMapTurn.moveLoc = Fp2( mousePosWorld );
                localMapTurn.moveTime = (f32)core->GetTheCurrentTime();
            } else {
                localMapTurn.madeMove = true;
                localMapTurn.moveLoc = Fp2( mousePosWorld );
            }
        }

        //spriteDrawContext->DrawCircle( ToVec2( gs.e1.p ), 4 );
        //spriteDrawContext->DrawCircle( ToVec2( gs.e2.p ), 4 );

        //f32 visMove = ( 1 - glm::exp( -dt * 16 ) );
        //gs.e1.vp += ( ToVec2( gs.e1.p ) - gs.e1.vp ) * visMove;
        //gs.e2.vp += ( ToVec2( gs.e2.p ) - gs.e2.vp ) * visMove;
        gs.e1.pt += dt;
        gs.e2.pt += dt;

        f32 nt1 = glm::clamp( gs.e1.pt / SIM_DT_FLOAT, 0.0f, 1.0f );
        glm::vec2 vp1 = glm::mix( ToVec2( gs.e1.lp ), ToVec2( gs.e1.p ), nt1 );

        f32 nt2 = glm::clamp( gs.e2.pt / SIM_DT_FLOAT, 0.0f, 1.0f );
        glm::vec2 vp2 = glm::mix( ToVec2( gs.e2.lp ), ToVec2( gs.e2.p ), nt2 );

        spriteDrawContext->DrawCircle( vp1, 4 );
        spriteDrawContext->DrawCircle( vp2, 4 );

        //DrawContext * backgroundDrawContext = core->RenderGetDrawContext( 1, true );
        DrawContext * uiDrawContext = core->RenderGetDrawContext( 2, true );
        DrawContext * debugDrawContext = core->RenderGetDrawContext( 3, true );


        static FontHandle fontHandle = core->ResourceGetFont( "default" );
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

}

