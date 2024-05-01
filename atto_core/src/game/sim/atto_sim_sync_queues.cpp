#include "atto_sim_sync_queues.h"

namespace atto {

    void SyncQueues::Start( i32 ping, f32 tickTime ) {
        slidingWindowWidth = ( i32 )( ( f32 )ping / ( 1000.0f * tickTime )  );

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
            return true;
        }

        return false;
    }

    void SyncQueues::AddTurn( PlayerNumber playerNumber, const MapTurn & turn ) {
        if( playerNumber.value == 1 ) {
            player1Turns.Enqueue( turn );
        } else {
            player2Turns.Enqueue( turn );
        }
    }

    MapTurn * SyncQueues::GetNextTurn( PlayerNumber playerNumber ) {
        if( playerNumber.value == 1 ) {
            return player1Turns.Peek();
        } else {
            return player2Turns.Peek();
        }
    }

    void SyncQueues::FinishTurn() {
        player1Turns.Dequeue();
        player2Turns.Dequeue();
    }
}

