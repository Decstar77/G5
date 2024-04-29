#include "atto_sim_actions.h"

namespace atto {
    void MapActionBuffer::Request_SelectEntities( PlayerNumber playerNumber, Span<EntityHandle> handle ) {
        AddAction( ( i32 ) MapAction::REQUEST_SELECTION_ENTITIES, playerNumber, handle );
    }

    void MapActionBuffer::Request_MoveUnit( PlayerNumber playerNumber, glm::vec2 pos ) {
        AddAction( ( i32 ) MapAction::REQUEST_MOVE, playerNumber, pos );
    }
}

