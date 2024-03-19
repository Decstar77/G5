#pragma once

#include "sim/atto_sim_map.h"
#include "../shared/atto_network.h"

namespace atto {

    class VisMap;

    class MapCommunicator {
    public:
        virtual void Request_Move( i32 playerNumber, glm::vec2 pos ) = 0;
        virtual void Request_Selection( i32 playerNumber, FixedList<EntityHandle, MAX_ENTITIES> & handles, ENTITY_SELECTION_CHANGE change ) = 0;
        virtual void Request_Attack( i32 playerNumber, EntityHandle target ) = 0;
        virtual void OnEntitySpawn( SimEntity * entity ) = 0;
        virtual void OnEntityDespawn( SimEntity * entity ) = 0;

        void SerializePacket_EntityPos( NetworkMessage * msg, i32 & playerNumber, glm::vec2 & pos );
        void SerializePacket_EntitySelection( NetworkMessage * msg, i32 & playerNumber, FixedList<EntityHandle, MAX_ENTITIES> & handles, ENTITY_SELECTION_CHANGE & change );
        void SerializePacket_EntityMove( NetworkMessage * msg, i32 & playerNumber, glm::vec2 & pos );
        void SerializePacket_EntityAttack( NetworkMessage * msg, i32 & playerNumber, EntityHandle & target );

        Core *          core = nullptr;
        VisMap *        visMap = nullptr;
        SimMap *        simMap = nullptr;
        bool            isWriting = false;
        NetworkMessage  msg = {};
    };

    class MapCommunicatorHost : public MapCommunicator {
    public:
        virtual void Request_Move( i32 playerNumber, glm::vec2 pos ) override;
        virtual void Request_Selection( i32 playerNumber, FixedList<EntityHandle, MAX_ENTITIES> & handles, ENTITY_SELECTION_CHANGE change ) override;
        virtual void Request_Attack( i32 playerNumber, EntityHandle target ) override;
        virtual void OnEntitySpawn( SimEntity * entity ) override;
        virtual void OnEntityDespawn( SimEntity * entity ) override;
    };

    class MapCommunicatorPeer : public MapCommunicator {
    public:
        virtual void Request_Move( i32 playerNumber, glm::vec2 pos ) override;
        virtual void Request_Selection( i32 playerNumber, FixedList<EntityHandle, MAX_ENTITIES> & handles, ENTITY_SELECTION_CHANGE change ) override;
        virtual void Request_Attack( i32 playerNumber, EntityHandle target ) override;
        virtual void OnEntitySpawn( SimEntity * entity ) override;
        virtual void OnEntityDespawn( SimEntity * entity ) override;
    };
}

