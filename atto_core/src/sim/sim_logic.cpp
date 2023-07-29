#include "../shared/atto_core.h"


namespace atto {

    void SimLogic::Advance( i32 playerOneInput, i32 playerTwoInput, i32 dcFlags ) {
        if( skipNextSteps > 0 ) {
            skipNextSteps--;
            return;
        }
        
        if( playerOneInput == 1 ) {
            state.p1Pos.x -= 1.0f;
        }
        else if( playerOneInput == 2 ) {
            state.p1Pos.x += 1.0f;
        }
        else if( playerOneInput == 3 ) {
            state.p1Pos.y -= 1.0f;
        }
        else if( playerOneInput == 4 ) {
            state.p1Pos.y += 1.0f;
        }

        if( playerTwoInput == 1 ) {
            state.p2Pos.x -= 1.0f;
        }
        else if( playerTwoInput == 2 ) {
            state.p2Pos.x += 1.0f;
        }
        else if( playerTwoInput == 3 ) {
            state.p2Pos.y -= 1.0f;
        }
        else if( playerTwoInput == 4 ) {
            state.p2Pos.y += 1.0f;
        }
    }

    void SimLogic::LoadState( u8 * buffer, i32 len ) {
        core->LogOutput( LogLevel::INFO, "LOAD STATE" );
        Assert( len == sizeof( SimState ), "SimState size mismatch" );
        memcpy( &state, buffer, len );
    }

    void SimLogic::SaveState( u8 ** buffer, i32 * len, i32 * checksum, i32 frame ) {
        //core->LogOutput( LogLevel::INFO, "SAVE STATE" );
        *len = sizeof( SimState );
        *buffer = (u8 *)malloc( sizeof( SimState ) );
        memcpy( *buffer, &state, sizeof( SimState ) );
        
        f32 p = state.p1Pos.x + state.p2Pos.x;
        //*checksum = *(i32 *)(&p);
        *checksum = 1;
    }

    void SimLogic::FreeState( void * buffer ) {
        free( buffer );
    }

    void SimLogic::LogState( char * filename, u8 * buffer, i32 len ) {

    }

    void SimLogic::SkipNextUpdates( i32 count ) {
        skipNextSteps = count;
    }


}



