#include "atto_sim_actions.h"

namespace atto {
    void MapActionBuffer::Request_SelectEntities( PlayerNumber playerNumber, Span<EntityHandle> handle ) {
        AddAction( ( i32 ) MapAction::REQUEST_SELECTION_ENTITIES, playerNumber, handle );
    }

    void MapActionBuffer::Request_MoveUnit( PlayerNumber playerNumber, glm::vec2 pos ) {
        AddAction( ( i32 ) MapAction::REQUEST_MOVE, playerNumber, pos );
    }

    void MapActionBuffer::Command_MoveUnit( EntityHandle entityHandle, glm::vec2 p1, glm::vec2 p2, f32 t1, f32 t2 ) {
        AddAction( ( i32 ) MapAction::COMMAND_MOVE, entityHandle, p1, p2, t1, t2 );
    }
    
}

