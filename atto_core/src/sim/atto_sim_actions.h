#pragma once

#include "../shared/atto_containers.h"
#include "../shared/atto_math.h"
#include "../shared/atto_rpc.h"

namespace atto {
    struct SimEntity;
    typedef ObjectHandle<SimEntity> EntityHandle;

    typedef ObjectHandle<SimEntity> EntityHandle;
    typedef TypeSafeNumber<i32, class PlayerNumberType>         PlayerNumber;
    typedef TypeSafeNumber<i32, class TeamNumberType>           TeamNumber;

    enum class MapAction : i32 {
        NONE = 0,
        REQUEST_SELECTION_ENTITIES,
        REQUEST_MOVE,
    };

    class MapActionBuffer : public RpcBuffer {
    public:
        void Request_SelectEntities( PlayerNumber playerNumber, Span<EntityHandle> handle );
        void Request_MoveUnit( PlayerNumber playerNumber, glm::vec2 pos );
    };
}
