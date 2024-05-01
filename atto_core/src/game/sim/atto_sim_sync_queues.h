#pragma once

#include "atto_sim_base.h"

namespace atto {
    class SyncQueues {
    public:
        void      Start( i32 ping, f32 tickTime );
        bool      CanTurn();
        void      AddTurn( PlayerNumber playerNumber, const MapTurn & turn );
        MapTurn * GetNextTurn( PlayerNumber playerNumber );
        void      FinishTurn();
        i32       GetSlidingWindowWidth() { return slidingWindowWidth; }

    private:
        i32                      slidingWindowWidth = -1;
        FixedQueue<MapTurn, 100> player1Turns = {};
        FixedQueue<MapTurn, 100> player2Turns = {};
    };
}


