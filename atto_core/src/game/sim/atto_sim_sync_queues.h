#pragma once

#include "atto_sim_base.h"

namespace atto {
    class SyncQueues {
    public:
        void      Start();
        bool      CanTurn();
        void      AddTurn( PlayerNumber playerNumber, const MapTurn & turn );
        MapTurn * GetNextTurn( PlayerNumber playerNumber );
        void      FinishTurn();
        i32       GetSlidingWindowWidth() { return slidingWindowWidth; }

    private:
        i32                     slidingWindowWidth = 4;
        FixedQueue<MapTurn, 10> player1Turns = {};
        FixedQueue<MapTurn, 10> player2Turns = {};
    };
}


