#include "atto_server_session.h"

#include "../../vendor/enet/include/enet.h"

namespace atto {
    void Session::Initialize() {
        simMap.SimInitialize();
    }

    void Session::Update( f32 dt ) {
        ZeroStruct( outAction );

        // Apply player actions
        simMap.ApplyActions( &inAction );
        outAction.Combine( simMap.simActionBuffer );
        ZeroStruct( inAction );

        // Step simulation
        simMap.SimUpdate( dt );
        outAction.Combine( simMap.simActionBuffer );

        if( outAction.data.GetSize() > 0 ) {
            ATTOINFO( "Sending actions" );
        }
    }
}


