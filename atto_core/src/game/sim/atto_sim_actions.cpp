
#include "atto_sim_base.h"
#include "atto_sim_map.h"
#include "../../shared/atto_rpc.h"

namespace atto {
    void MapActionBuffer::PlayerSelect( PlayerNumber playerNumber, const EntHandleList & entList, EntitySelectionChange change ) {
        AddAction( MapActionType::PLAYER_SELECTION, playerNumber, entList, change );
    }

    void MapActionBuffer::ConstructBuilding( PlayerNumber playerNumber, EntityType::_enumerated buildingType, fp2 pos ) {
        AddAction( MapActionType::SIM_ENTITY_UNIT_COMMAND_CONSTRUCT_BUILDING, playerNumber, ( i32 )buildingType, pos );
    }

    void MapActionBuffer::TrainUnit( PlayerNumber playerNumber, EntityType::_enumerated unitType ) {
        AddAction( MapActionType::SIM_ENTITY_BUILDING_COMMAND_TRAIN_UNIT, playerNumber, ( i32 )unitType );
    }
}

