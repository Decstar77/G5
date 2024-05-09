#include "atto_sim_actions.h"

namespace atto {
    void MapActionBuffer::Request_SelectEntities( PlayerNumber playerNumber, Span<EntityHandle> handle ) {
        AddAction( ( i32 ) MapAction::REQUEST_SELECTION_ENTITIES, playerNumber, handle );
    }

    void MapActionBuffer::Request_MoveUnit( PlayerNumber playerNumber, fp2 pos ) {
        AddAction( ( i32 ) MapAction::REQUEST_MOVE, playerNumber, pos );
    }

    void MapActionBuffer::Request_AttackUnit( PlayerNumber playerNumber, EntityHandle handle ) {
        AddAction( ( i32 ) MapAction::REQUEST_ATTACK, playerNumber, handle );
    }

    void MapActionBuffer::Request_PlaceStructure( PlayerNumber playerNumber, EntityType structureType, fp2 p ) {
        AddAction( ( i32 ) MapAction::REQUEST_PLACE_STRUCTURE, playerNumber, ( i32 )structureType, p );
    }
}

