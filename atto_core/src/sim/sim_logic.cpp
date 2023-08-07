#include "../shared/atto_core.h"

namespace atto {

    void SimLogic::LoadResources() {
        tankFireSound = core->ResourceGetAndLoadAudio( "gun_pistol_shot_01.wav" );
    }

    void SimLogic::Start() {
        ZeroStruct( state );

        const i32 halfTileWidth = 70 / 2;
        const i32 halfTileHeight = 70 / 2;
        const i32 tileHCount = mapWidth / halfTileWidth;
        const i32 tileVCount = mapHeight / halfTileHeight;

        for( i32 x = 0; x < tileHCount; x++ ) {
            for( i32 y = 0; y < tileVCount; y++ ) {
                MapElement el = {};
                el.type = MAP_TILE_TYPE_METAL_WALL;
                el.index = 0;
                el.pos = CreateFPVec2( x * halfTileWidth + 12, y * halfTileHeight + 12 );

                if( x == 0 || y == 0 || x == tileHCount - 1 || y == tileVCount - 1 ) {
                    state.elements.Add( el );
                }
            }
        }

        state.playerTanks[ 0 ].pos = CreateFPVec2( mapWidth / 2.0f - 100, mapHeight / 2.0f );
        state.playerTanks[ 0 ].rot = CreateFP( 0 );
        state.playerTanks[ 0 ].turretRot = CreateFP( 0 );
        state.playerTanks[ 0 ].health = 3;

        state.playerTanks[ 1 ].pos = CreateFPVec2( mapWidth / 2.0f + 100, mapHeight / 2.0f );
        state.playerTanks[ 1 ].rot = CreateFP( 0 );
        state.playerTanks[ 1 ].turretRot = CreateFP( 0 );
        state.playerTanks[ 1 ].health = 3;
    }

    void SimLogic::TankFireBullet( SimTank & tank, bool isRollback ) {
        if( tank.fireCooldown == CreateFP( 0 ) ) {
            tank.fireCooldown = CreateFP( 30 );

            SimProjectile proj = {};
            proj.pos = tank.pos;
            proj.dir.x = FPSin( tank.turretRot );
            proj.dir.y = FPCos( tank.turretRot );
            proj.speed = CreateFP( 5.0f );
            proj.damage = 1;

            state.projectiles.Add( proj );

            if( isRollback == false ) {
                core->AudioPlay( tankFireSound );
            }

            core->LogOutput( LogLevel::DEBUG, "Fire!" );
        }
    }

    void SimLogic::Advance( i32 playerOneInput, i32 playerTwoInput, i32 dcFlags, bool isRollback ) {
        fp tankSpeed = CreateFP( 1.0f );

        if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_LEFT ) ) {
            state.playerTanks[ 0 ].pos.x -= tankSpeed;
        }
        else if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_RIGHT ) ) {
            state.playerTanks[ 0 ].pos.x += tankSpeed;
        }
        if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_UP ) ) {
            state.playerTanks[ 0 ].pos.y += tankSpeed;
        }
        else if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_DOWN ) ) {
            state.playerTanks[ 0 ].pos.y -= tankSpeed;
        }
        if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_TURRET_LEFT ) ) {
            state.playerTanks[ 0 ].turretRot -= CreateFP( 0.1f );
        }
        else if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_TURRET_RIGHT ) ) {
            state.playerTanks[ 0 ].turretRot += CreateFP( 0.1f );
        }
        if( IsEnumFlagSet( playerOneInput, SIM_INPUT_TANK_TURRET_FIRE ) ) {
            TankFireBullet( state.playerTanks[ 0 ], isRollback );
        }

        if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_LEFT ) ) {
            state.playerTanks[ 1 ].pos.x -= tankSpeed;
        }
        else if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_RIGHT ) ) {
            state.playerTanks[ 1 ].pos.x += tankSpeed;
        }
        if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_UP ) ) {
            state.playerTanks[ 1 ].pos.y += tankSpeed;
        }
        else if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_DOWN ) ) {
            state.playerTanks[ 1 ].pos.y -= tankSpeed;
        }
        if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_TURRET_LEFT ) ) {
            state.playerTanks[ 1 ].turretRot -= CreateFP( 0.1f );
        }
        else if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_TURRET_RIGHT ) ) {
            state.playerTanks[ 1 ].turretRot += CreateFP( 0.1f );
        }
        if( IsEnumFlagSet( playerTwoInput, SIM_INPUT_TANK_TURRET_FIRE ) ) {
            TankFireBullet( state.playerTanks[ 1 ], isRollback );
        }

        for( i32 i = 0; i < 2; i++ ) {
            state.playerTanks[ i ].fireCooldown -= CreateFP( 1 );
            if( state.playerTanks[ i ].fireCooldown < CreateFP( 0 ) ) {
                state.playerTanks[ i ].fireCooldown = CreateFP( 0 );
            }
        }

        fp projSpeed = CreateFP( 10 );
        const i32 projCount = state.projectiles.GetCount();
        for( i32 i = 0; i < projCount; i++ ) {
            SimProjectile & p =  state.projectiles[ i ];
            p.pos += p.dir * projSpeed;
        }
    }

    void SimLogic::LoadState( u8 * buffer, i32 len ) {
        //core->LogOutput( LogLevel::INFO, "LOAD STATE" );
        AssertMsg( len == sizeof( SimState ), "SimState size mismatch" );
        memcpy( &state, buffer, len );
    }

    /*
    * Simple checksum function stolen from wikipedia
    *   http://en.wikipedia.org/wiki/Fletcher%27s_checksum
    */
    int Fletcher32Checksum( short * data, size_t len ) {
        int sum1 = 0xffff, sum2 = 0xffff;

        while( len ) {
            size_t tlen = len > 360 ? 360 : len;
            len -= tlen;
            do {
                sum1 += *data++;
                sum2 += sum1;
            } while( --tlen );
            sum1 = ( sum1 & 0xffff ) + ( sum1 >> 16 );
            sum2 = ( sum2 & 0xffff ) + ( sum2 >> 16 );
        }

        /* Second reduction step to reduce sums to 16 bits */
        sum1 = ( sum1 & 0xffff ) + ( sum1 >> 16 );
        sum2 = ( sum2 & 0xffff ) + ( sum2 >> 16 );
        return sum2 << 16 | sum1;
    }


    void SimLogic::SaveState( u8 ** buffer, i32 * len, i32 * checksum, i32 frame ) {
        //core->LogOutput( LogLevel::INFO, "SAVE STATE" );
        *len = sizeof( SimState );
        *buffer = (u8 *)malloc( sizeof( SimState ) );
        memcpy( *buffer, &state, sizeof( SimState ) );

        fpv2 check = {};
        check.x = state.playerTanks[ 0 ].pos.x;
        check.y = state.playerTanks[ 1 ].pos.y;
        *checksum = Fletcher32Checksum( (short *)( &check ), sizeof( fpv2 ) );
    }

    void SimLogic::FreeState( void * buffer ) {
        free( buffer );
    }

    void SimLogic::LogState( char * filename, u8 * buffer, i32 len ) {

    }

    void SimLogic::SkipNextUpdates( i32 count ) {
        skipNextSteps = count;
    }

    i32 SimLogic::GetNextInputs( i32 localPLayerNumber ) {
        i32 input = 0;

        if( core->InputKeyDown( KEY_CODE_A ) ) {
            SetEnumFlag( input, SIM_INPUT_TANK_LEFT );
        }
        if( core->InputKeyDown( KEY_CODE_D ) ) {
            SetEnumFlag( input, SIM_INPUT_TANK_RIGHT );
        }
        if( core->InputKeyDown( KEY_CODE_W ) ) {
            SetEnumFlag( input, SIM_INPUT_TANK_UP );
        }
        if( core->InputKeyDown( KEY_CODE_S ) ) {
            SetEnumFlag( input, SIM_INPUT_TANK_DOWN );
        }

        const f32 deadZone = glm::radians( 15.0f ); // Convert 15 degrees to radians
        const glm::vec2 mousePixels = core->InputMousePosPixels();
        const glm::vec2 mouseWorld = core->ScreenPosToWorldPos( mousePixels );

        glm::vec2 tankPos = FpVec2ToVec2( state.playerTanks[ localPLayerNumber ].pos );
        f32 tankTurretRot = FixedToFloat( state.playerTanks[ localPLayerNumber ].turretRot );

        glm::vec2 tankDir = glm::normalize( glm::vec2( sin( tankTurretRot ), cos( tankTurretRot ) ) );
        glm::vec2 tankToMouse = glm::normalize( mouseWorld - tankPos );

        f32 angle = glm::orientedAngle( tankDir, tankToMouse );

        if( angle < -deadZone ) {
            SetEnumFlag( input, SIM_INPUT_TANK_TURRET_RIGHT );
            core->LogOutput( LogLevel::INFO, "R" );
        }
        else if( angle > deadZone ) {
            SetEnumFlag( input, SIM_INPUT_TANK_TURRET_LEFT );
            core->LogOutput( LogLevel::INFO, "L" );
        }
        else {
            core->LogOutput( LogLevel::INFO, "In dead zone" );
        }

        if( core->InputMouseButtonDown( MOUSE_BUTTON_1 ) ) {
            SetEnumFlag( input, SIM_INPUT_TANK_TURRET_FIRE );
        }

        return input;
    }

    void SimLogic::StartSinglePlayerGame() {
        Start();
        isRunning = true;
        gameType = SIM_GAME_TYPE_SINGLE_PLAYER;
    }

    void SimLogic::StartMultiplayerGame() {
        isRunning = true;
        gameType = SIM_GAME_TYPE_MULTI_PLAYER;
    }


}



