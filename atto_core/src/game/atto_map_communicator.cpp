#include "atto_map_communicator.h"
#include "vis/atto_vis_map.h"
#include "../shared/atto_core.h"

namespace atto {
    void MapCommunicatorHost::Request_Move( i32 playerNumber, glm::vec2 pos ) {
        simMap->Action_Move( playerNumber, pos );
    }

    void MapCommunicatorHost::Request_Selection( i32 playerNumber, FixedList<EntityHandle, MAX_ENTITIES> & handles, ENTITY_SELECTION_CHANGE change ) {
        simMap->Action_Select( playerNumber, handles, change );
    }

    void MapCommunicatorHost::Request_Attack( i32 playerNumber, EntityHandle target ) {
        simMap->Action_Attack( playerNumber, target );
    }

    void MapCommunicatorHost::OnEntitySpawn( SimEntity * entity ) {
        visMap->OnSpawnEntity( core, entity );
    }

    void MapCommunicatorHost::OnEntityDespawn( SimEntity * entity ) {
    }

    /*
        =====================================================================
    */

    void MapCommunicatorPeer::Request_Move( i32 playerNumber, glm::vec2 pos ) {
        ZeroStruct( msg );
        msg.type = NetworkMessageType::ENTITY_REQUEST_MOVE;
        NetworkMessagePush( msg, playerNumber );
        NetworkMessagePush( msg, pos );
        core->NetworkSend( msg );
    }

    void MapCommunicatorPeer::Request_Selection( i32 playerNumber, FixedList<EntityHandle, MAX_ENTITIES> & handles, ENTITY_SELECTION_CHANGE change ) {
        ZeroStruct( msg );
        msg.type = NetworkMessageType::ENTITY_REQUEST_SELECTION;
        NetworkMessagePush( msg, playerNumber );
        const i32 count = handles.GetCount();
        NetworkMessagePush( msg, count );
        for( i32 i = 0; i < count; i++ ) {
            NetworkMessagePush( msg, handles[ i ] );
        }
        NetworkMessagePush( msg, change );
        core->NetworkSend( msg );

        // We pre-emptively apply the selection change, we'll get a response from the host will the final selection. Which in 99% of cases will be the same as the pre-emptive selection.
        simMap->Action_Select( playerNumber, handles, change );
    }

    void MapCommunicatorPeer::Request_Attack( i32 playerNumber, EntityHandle target ) {
        ZeroStruct( msg );
        msg.type = NetworkMessageType::ENTITY_REQUEST_ATTACK;
        NetworkMessagePush( msg, playerNumber );
        NetworkMessagePush( msg, target );
        core->NetworkSend( msg );
    }

    void MapCommunicatorPeer::OnEntitySpawn( SimEntity * entity ) {
        visMap->OnSpawnEntity( core, entity );
    }

    void MapCommunicatorPeer::OnEntityDespawn( SimEntity * entity ) {

    }

    /*
        =====================================================================
    */
    void MapCommunicator::SerializePacket_EntityPos( NetworkMessage * msg, i32 & playerNumber, glm::vec2 & pos ) {
        if( isWriting == true ) {
            msg->type = NetworkMessageType::ENTITY_STATE_UPDATE;
        }
        else {
        }
    }

}
