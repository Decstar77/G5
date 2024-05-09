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

    enum class EntityType : i32 {
        INVALID = 0,
        UNIT_SCOUT,

        STRUCTURE_CITY_CENTER,
        STRUCTURE_SMOL_REACTOR,
    };

    enum class MapAction : i32 {
        NONE = 0,
        REQUEST_SELECTION_ENTITIES,
        REQUEST_MOVE,
        REQUEST_ATTACK,
        REQUEST_PLACE_STRUCTURE,
    };

    class MapActionBuffer : public RpcBuffer {
    public:
        void Request_SelectEntities( PlayerNumber playerNumber, Span<EntityHandle> handle );
        void Request_MoveUnit( PlayerNumber playerNumber, fp2 pos );
        void Request_AttackUnit( PlayerNumber playerNumber, EntityHandle handle );
        void Request_PlaceStructure( PlayerNumber playerNumber, EntityType structureType, fp2 p );
    };
}
