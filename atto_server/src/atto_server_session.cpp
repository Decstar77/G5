#include "atto_server_session.h"

#include "../../vendor/enet/include/enet.h"

namespace atto {
    void Session::Initialize() {
        simMap.SimInitialize();
    }

    void Session::Update( f32 dt ) {
        const i32 tickRate = 30;
        const f32 tickTime = 1.0f / tickRate;

        timeAccumulator += dt;
        if( timeAccumulator >= tickTime ) {
            timeAccumulator -= tickTime;

            ZeroStruct( outAction );

            // Apply player actions
            simMap.ApplyActions( &inAction );
            outAction.Combine( simMap.simActionBuffer );
            ZeroStruct( inAction );
            
            // Step simulation
            simMap.SimUpdate( tickTime );
            outAction.Combine( simMap.simActionBuffer );


            if( outAction.data.GetSize() > 0 ) {
                ATTOINFO( "Sending actions" );
            }
        }
    
    }
}


