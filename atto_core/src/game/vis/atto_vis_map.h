#pragma once

#include "../sim/atto_sim_map.h"

namespace atto {

    class VisMap {
    public:
        SimMap * simMap = nullptr;
        MapCommunicator * communicator = nullptr;
        i32                                         localPlayerNumber = -1;
        i32                                         localPlayerTeamNumber = -1;
        glm::vec2                                   localCameraPos = glm::vec2( 0.0f );
        f32                                         localCameraZoom = 0.0f;

    public:
        void Initialize( Core * core, SimMap * simMap, MapCommunicator * communicator, i32 playerNumber, i32 teamNumber );
        void Render( Core * core, f32 dt );
        void OnSpawnEntity( Core * core, SimEntity * entity );
    };
}
