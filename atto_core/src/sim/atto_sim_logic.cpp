#include "atto_sim_logic.h"

namespace atto {
    
    void SimLogicAndState::Start(Core* core) {
    }

    void SimLogicAndState::Step(Core* core, i32 i1, i32 i2) {
        if (i1 == 1) {
            p1Pos.x -= 1;
        }
        if (i1 == 2) {
            p1Pos.x += 1;
        }
        if (i1 == 3) {
            p1Pos.y += 1;
        }
        if (i1 == 4) {
            p1Pos.y -= 1;
        }
        if (i2 == 1) {
            p2Pos.x -= 1;
        }
        if (i2 == 2) {
            p2Pos.x += 1;
        }
        if (i2 == 3) {
            p2Pos.y += 1;
        }
        if (i2 == 4) {
            p2Pos.y -= 1;
        }
    }
}